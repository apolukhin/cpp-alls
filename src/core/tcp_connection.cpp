#include <cppalls/core/tcp_connection.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>

namespace cppalls {

namespace {

    typedef cppalls::connection::callback_t callback_t;

    static inline std::error_code to_std_error_code(const boost::system::error_code& error) noexcept {
        return std::make_error_code( static_cast<std::errc>(error.value()) );
    }

    struct callback_read_wrapped {
        callback_t                              callback_;
        std::shared_ptr<tcp_connection>         connection_;

        callback_read_wrapped(callback_t&& callback, std::shared_ptr<tcp_connection>&& connection) noexcept
            : callback_(std::move(callback))
            , connection_(std::move(connection))
        {}

        void operator()(const boost::system::error_code& error, std::size_t /*bytes_received*/) {
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };

    struct callback_write_wrapped {
        callback_t                              callback_;
        std::shared_ptr<tcp_connection>         connection_;

        callback_write_wrapped(callback_t&& callback, std::shared_ptr<tcp_connection>&& connection) noexcept
            : callback_(std::move(callback))
            , connection_(std::move(connection))
        {}

        void operator()(const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            connection_->response().clear();
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };


    struct io_service_impl final: api::io_service_provider {
        boost::asio::io_service io_;

        boost::asio::io_service& io_service() noexcept override{
            return io_;
        }

        virtual void start(const YAML::Node&) override {}
        virtual void stop() override {}
    };
} // anonymous namespace

tcp_connection::tcp_connection(std::shared_ptr<api::connection_processor>&& processor, std::shared_ptr<api::io_service_provider>&& io_service)
    : processor_(std::move(processor))
    , io_service_(std::move(io_service))
    , s_(io_service_->io_service())
{}

void tcp_connection::async_write(callback_t&& cb) {
    boost::asio::async_write(
        socket(),
        boost::asio::buffer(response_.begin(), response_.end() - response_.begin()),
        callback_write_wrapped(std::move(cb), shared_from_this())
    );
}

void tcp_connection::async_read(tcp_connection::callback_t&& cb, std::size_t size) {
    request_.clear();
    request_.resize(size);
    boost::asio::async_read(
        socket(),
        boost::asio::buffer(request_.begin(), size),
        callback_read_wrapped(std::move(cb), shared_from_this())
    );
}


void tcp_connection::write() {
    boost::asio::write(
        socket(),
        boost::asio::buffer(response_.begin(), response_.end() - response_.begin())
    );

    response_.clear();
}

void tcp_connection::read(std::size_t size) {
    request_.clear();
    request_.resize(size);
    boost::asio::read(
        socket(),
        boost::asio::buffer(request_.begin(), size)
    );
}

void tcp_connection::close() {
    s_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    s_->close();
}

cppalls::detail::tcp_socket& tcp_connection::socket() noexcept {
    return *s_;
}

std::shared_ptr<tcp_connection> tcp_connection::create(std::shared_ptr<api::connection_processor> processor, std::shared_ptr<api::io_service_provider> io_serv) {
    return std::allocate_shared<tcp_connection>(
        cppalls::detail::shared_allocator_friend<tcp_connection>(),
        std::move(processor),
        std::move(io_serv)
    );
}


std::shared_ptr<tcp_connection> tcp_connection::create(const char* address, const char* port) {
    using boost::asio::ip::tcp;

    auto io = std::make_shared<io_service_impl>();
    tcp::resolver resolver(io->io_service());
    tcp::resolver::query query(address, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    auto s = std::allocate_shared<tcp_connection>(
        cppalls::detail::shared_allocator_friend<tcp_connection>(),
        std::shared_ptr<api::connection_processor>(),
        std::move(io)
    );

    boost::asio::connect(s->socket(), endpoint_iterator);

    return std::move(s);
}

tcp_connection::~tcp_connection() {
    boost::system::error_code ignore;
    s_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    s_->close(ignore);
}

} // namespace cppalls

