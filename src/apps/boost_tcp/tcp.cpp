#include <cppalls/core/server.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/connection.hpp>
#include <cppalls/core/stack_request.hpp>
#include <cppalls/core/stack_response.hpp>
#include <cppalls/core/export.hpp>
#include <cppalls/core/detail/boost_asio_fwd.hpp>
#include <cppalls/core/detail/stack_pimpl.hpp>
#include <cppalls/core/detail/shared_allocator_friend.hpp>
#include <cppalls/api/io_service.hpp>
#include <cppalls/api/logger.hpp>
#include <cppalls/api/connection_processor.hpp>
#include <memory>
#include <functional>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>

#include <yaml-cpp/yaml.h>


namespace {

class tcp;

typedef cppalls::connection::callback_t     callback_t;
typedef cppalls::api::io_service_provider   io_service_t;
typedef cppalls::api::connection_processor  connection_processor_t;
typedef cppalls::api::logger                logger_t;


// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_allocator {
public:
    inline handler_allocator() noexcept {
        in_use_[0] = false;
        in_use_[1] = false;
    }

    handler_allocator(const handler_allocator&) = delete;
    handler_allocator& operator=(const handler_allocator&) = delete;

    inline void* allocate(std::size_t size) {
        if (!in_use_[0] && size < sizeof(storage_t)) {
            in_use_[0] = true;
            return &storages_;
        } else if (!in_use_[1] && size < sizeof(storage_t)) {
            in_use_[1] = true;
            return &storages_[1];
        } else {
            return ::operator new(size);
        }
    }

    inline void deallocate(void* pointer) noexcept {
        if (pointer == &storages_[0]) {
            in_use_[0] = false;
        } else if (pointer == &storages_[1]) {
            in_use_[1] = false;
        } else {
            ::operator delete(pointer);
        }
    }

private:
    static const std::size_t storages_min_size_ = 512u;
    static const std::size_t storages_count_ = 2u;

    // Storage space used for handler-based custom memory allocation.
    typedef typename std::aligned_storage<storages_min_size_>::type storage_t;
    storage_t storages_[storages_count_];

    // Whether the handler-based custom allocation storage has been used.
    bool in_use_[storages_count_];
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler {
public:
    template <class H>
    inline custom_alloc_handler(handler_allocator& a, H&& h)
        : allocator_(a)
        , handler_(std::forward<H>(h))
    {}

    template <typename... Args>
    inline void operator()(Args&&... args) {
        handler_(std::forward<Args>(args)...);
    }

    inline friend void* asio_handler_allocate(std::size_t size,
        custom_alloc_handler<Handler>* this_handler)
    {
        return this_handler->allocator_.allocate(size);
    }

    inline friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/,
        custom_alloc_handler<Handler>* this_handler) noexcept
    {
        this_handler->allocator_.deallocate(pointer);
    }

private:
    handler_allocator& allocator_;
    Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<typename std::remove_reference<Handler>::type>
    make_custom_alloc_handler(handler_allocator& a, Handler&& h)
{
    return custom_alloc_handler<typename std::remove_reference<Handler>::type>(a, std::forward<Handler>(h));
}

inline std::error_code to_std_error_code(const boost::system::error_code& error) noexcept {
    return std::make_error_code( static_cast<std::errc>(error.value()) );
}

struct callback_read_wrapped {
    callback_t                              callback_;
    std::shared_ptr<cppalls::connection>    connection_;

    callback_read_wrapped(callback_t&& callback, std::shared_ptr<cppalls::connection>&& connection) noexcept
        : callback_(std::move(callback))
        , connection_(std::move(connection))
    {}

    void operator()(const boost::system::error_code& error, std::size_t /*bytes_received*/) {
        if (callback_) {
            callback_(*connection_, to_std_error_code(error));
        }
    }
};

template <class Protocol>
class async_connection_fast final: public cppalls::connection, public std::enable_shared_from_this<async_connection_fast<Protocol> > {
    // We do keep parent_ alive to make sure, that DLL won't be unloaded while tcp_connection_fasts are running.
    std::shared_ptr<cppalls::api::application> parent_;

    std::shared_ptr<logger_t>               log_;
    std::shared_ptr<connection_processor_t> processor_;
    std::shared_ptr<io_service_t>           io_service_;
    cppalls::detail::stack_pimpl<Protocol > s_;

    cppalls::stack_request                  request_;
    cppalls::stack_response                 response_;
    handler_allocator                       allocator_;

    typedef async_connection_fast<Protocol> this_t;

    struct callback_write_wrapped {
        callback_t                              callback_;
        std::shared_ptr<this_t>    connection_;

        callback_write_wrapped(callback_t&& callback, std::shared_ptr<this_t>&& connection) noexcept
            : callback_(std::move(callback))
            , connection_(std::move(connection))
        {}

        void operator()(const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            connection_->response_.clear();
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };

    explicit async_connection_fast(std::shared_ptr<cppalls::api::application>&& parent,std::shared_ptr<connection_processor_t>&& processor, std::shared_ptr<io_service_t>&& io_service)
        : parent_(std::move(parent))
        , processor_(std::move(processor))
        , io_service_(std::move(io_service))
        , s_(io_service_->io_service())
    {}

    template <class> friend class cppalls::detail::shared_allocator_friend;

public:
    void async_write(callback_t&& cb) override {
        socket().async_send(
            boost::asio::buffer(response_.begin(), response_.end() - response_.begin()),
            make_custom_alloc_handler(allocator_, callback_write_wrapped(std::move(cb), this_t::shared_from_this()))
        );
    }

    void async_read(callback_t&& cb, std::size_t size) override {
        request_.clear();
        request_.resize(size);
        socket().async_receive(
            boost::asio::buffer(request_.begin(), size),
            make_custom_alloc_handler(allocator_, callback_read_wrapped(std::move(cb), this_t::shared_from_this()))
        );
    }

    cppalls::request& request() noexcept override {
        return request_;
    }

    cppalls::response& response() noexcept override {
        return response_;
    }

    void close() override {
        s_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        s_->close();
    }

    Protocol& socket() noexcept {
        return *s_;
    }

    static std::shared_ptr<async_connection_fast<Protocol> > create(std::shared_ptr<cppalls::api::application> parent, std::shared_ptr<connection_processor_t> processor, std::shared_ptr<io_service_t> io_serv) {
        return std::allocate_shared<this_t>(
            cppalls::detail::shared_allocator_friend<this_t>(),
            std::move(parent),
            std::move(processor),
            std::move(io_serv)
        );
    }

    void start() {
        auto& processor = *processor_;
        processor(*this);
    }

    ~async_connection_fast() override {
        boost::system::error_code ignore;
        s_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
        s_->close(ignore);
    }
};

class tcp : public cppalls::api::application {
    std::shared_ptr<logger_t>                       log_;
    std::shared_ptr<connection_processor_t>         processor_;
    std::shared_ptr<io_service_t>                   io_service_;

    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    bool                                            nodelay_;

    std::shared_ptr<tcp> shared_from_this() {
        return std::static_pointer_cast<tcp>(cppalls::api::application::shared_from_this());
    }

    void accept() {
        auto c = async_connection_fast<cppalls::detail::tcp_socket>::create(cppalls::api::application::shared_from_this(), processor_, io_service_);
        auto& socket = c->socket();
        acceptor_->async_accept(
            socket,
            std::bind(&tcp::on_accpet, shared_from_this(), std::move(c), std::placeholders::_1)
        );
    }

    void on_accpet(std::shared_ptr<async_connection_fast<cppalls::detail::tcp_socket> > c, const boost::system::error_code& error) {
        accept();

        if (!error) {
            c->socket().set_option(boost::asio::ip::tcp::no_delay(nodelay_));
            processor_->operator ()(*c);
        } else {
            LWARN(log_) << "Error during accept: " << error;
        }
    }

public:
    void start(const YAML::Node& conf) override {
        log_ = cppalls::server::get<logger_t>(
            conf["logger"].as<std::string>()
        );

        processor_ = cppalls::server::get<connection_processor_t>(
            conf["processor"].as<std::string>()
        );

        io_service_ = cppalls::server::get<io_service_t>(
            conf["io_service"].as<std::string>()
        );


        boost::asio::ip::tcp::endpoint ep(
            boost::asio::ip::address::from_string(conf["listen-address"].as<std::string>("0.0.0.0")),
            conf["listen-port"].as<unsigned short>()
        );

        acceptor_ = std::make_shared<boost::asio::ip::tcp::acceptor>(
            io_service_->io_service(),
            std::move(ep)
        );

        if (conf["reuse-address"].as<bool>(true)) {
            acceptor_->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
        }

        nodelay_ = conf["nodelay"].as<bool>(true);

        accept();
    }

    void stop() {
        if (acceptor_) {
            acceptor_->close();
            acceptor_.reset();
        }

        log_.reset();
        processor_.reset();
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new tcp());
    }

    ~tcp() override {
        stop();
    }
};



class udp : public cppalls::api::application {
    std::shared_ptr<logger_t>                       log_;
    std::shared_ptr<connection_processor_t>         processor_;
    std::shared_ptr<io_service_t>                   io_service_;

    boost::asio::ip::udp::endpoint                  listen_endpoint_;
    boost::asio::ip::udp::endpoint                  endpoint_;
    bool                                            nodelay_;
    char                                            tmp_buf_[1];

    std::shared_ptr<udp> shared_from_this() {
        return std::static_pointer_cast<udp>(cppalls::api::application::shared_from_this());
    }

    void accept() {
        auto c = async_connection_fast<cppalls::detail::udp_socket>::create(cppalls::api::application::shared_from_this(), processor_, io_service_);
        cppalls::detail::udp_socket& socket = c->socket();
        socket.bind(listen_endpoint_);
        socket.async_receive_from(
            boost::asio::buffer(tmp_buf_),
            endpoint_,
            cppalls::detail::udp_socket::message_peek,
            std::bind(&udp::on_accpet, shared_from_this(), std::move(c), std::placeholders::_1, std::placeholders::_2)
        );
    }

    void on_accpet(std::shared_ptr<async_connection_fast<cppalls::detail::udp_socket> > c, const boost::system::error_code& error, std::size_t /*bytes_received*/) {
        boost::system::error_code error2;
        if (!error) {
            c->socket().connect(endpoint_, error2);
        }

        accept();

        if (!error2 && !error) {
            processor_->operator ()(*c);
        } else if (error) {
            LWARN(log_) << "Error during UDP accept: " << error;
        } else if (error2) {
            LWARN(log_) << "Error during UDP connect: " << error2;
        }
    }

public:
    void start(const YAML::Node& conf) override {
        log_ = cppalls::server::get<logger_t>(
            conf["logger"].as<std::string>()
        );

        processor_ = cppalls::server::get<connection_processor_t>(
            conf["processor"].as<std::string>()
        );

        io_service_ = cppalls::server::get<io_service_t>(
            conf["io_service"].as<std::string>()
        );


        listen_endpoint_ = boost::asio::ip::udp::endpoint(
            boost::asio::ip::address::from_string(conf["listen-address"].as<std::string>("0.0.0.0")),
            conf["listen-port"].as<unsigned short>()
        );

        //if (conf["reuse-address"].as<bool>(true)) {
        //    acceptor_->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
        //}

        nodelay_ = conf["nodelay"].as<bool>(true);

        accept();
    }

    void stop() {
        listen_endpoint_ = boost::asio::ip::udp::endpoint();
        log_.reset();
        processor_.reset();
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new tcp());
    }

    ~udp() override {
        stop();
    }
};

} // anonymous namespace

CPPALLS_APPLICATION(tcp::create, tcp_acceptor)
CPPALLS_APPLICATION(udp::create, udp_acceptor)
