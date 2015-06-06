#include <cppalls/core/server.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/connection.hpp>
#include <cppalls/core/export.hpp>
#include <cppalls/core/detail/boost_asio_fwd.hpp>
#include <cppalls/core/detail/stack_pimpl.hpp>
#include <cppalls/core/detail/shared_allocator_friend.hpp>
#include <cppalls/api/io_service.hpp>
#include <cppalls/api/logger.hpp>
#include <cppalls/api/connection_processor.hpp>
#include <memory>
#include <functional>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>

#include <boost/functional/hash.hpp>
#include <boost/exception/all.hpp>

#include <yaml-cpp/yaml.h>

#include "handler_allocator.hpp"
#include "async_connection_fast.hpp"

namespace {

typedef cppalls::api::io_service_provider   io_service_t;
typedef cppalls::api::connection_processor  connection_processor_t;
typedef cppalls::api::logger                logger_t;

struct endpoint_hash {
    std::size_t operator()(const boost::asio::ip::udp::endpoint& e) const noexcept {
        const auto it = reinterpret_cast<const char*>(e.data());
        return boost::hash_range(it, it + e.size());
    }
};


class udp_helper: public boost::asio::socket_base {
    std::shared_ptr<boost::asio::io_service::strand>    socket_strand_;
    std::shared_ptr<cppalls::detail::udp_socket>        socket_;
    boost::asio::ip::udp::endpoint                      endpoint_;

public:
    udp_helper() = delete;
    udp_helper(const udp_helper&) = delete;
    udp_helper(udp_helper&&) = delete;
    udp_helper& operator=(const udp_helper&) = delete;
    udp_helper& operator=(udp_helper&&) = delete;

    udp_helper(
                boost::asio::io_service& ,
                std::shared_ptr<boost::asio::io_service::strand>&& socket_strand,
                std::shared_ptr<cppalls::detail::udp_socket>&& socket,
                boost::asio::ip::udp::endpoint&& endpoint)
        : socket_strand_(std::move(socket_strand))
        , socket_(std::move(socket))
        , endpoint_(std::move(endpoint))
    {}

    template <class... Args>
    void shutdown(Args&&... /*ignore*/) {
        endpoint_ = boost::asio::ip::udp::endpoint();
    }

    template <class... Args>
    void close(Args&&... /*ignore*/) {
        endpoint_ = boost::asio::ip::udp::endpoint();
    }

    template<class MutableBufferSequence, class ReadHandler>
    void async_write_some(const MutableBufferSequence& buffer, ReadHandler&& handler) {
        socket_->async_send_to(
            buffer, endpoint_, socket_strand_->wrap(std::forward<ReadHandler>(handler))
        );
    }

    template<class ConstBufferSequence, class ReadHandler>
    void async_read_some(const ConstBufferSequence& buffer, ReadHandler&& handler) {
        socket_->async_receive_from(
            buffer, endpoint_, socket_strand_->wrap(std::forward<ReadHandler>(handler))
        );
    }
};


class udp_connection_fast final: public network::async_connection_fast<udp_helper> {
    template <class... Args>
    explicit udp_connection_fast(std::shared_ptr<cppalls::api::application>&& parent,std::shared_ptr<connection_processor_t>&& processor, std::shared_ptr<io_service_t>&& io_service, Args&&... args)
        : async_connection_fast(std::move(parent), std::move(processor), std::move(io_service), std::forward<Args>(args)...)
    {}

    template <class> friend class cppalls::detail::shared_allocator_friend;

public:
    static std::shared_ptr<udp_connection_fast> create(
        std::shared_ptr<cppalls::api::application> parent,
        std::shared_ptr<connection_processor_t> processor,
        std::shared_ptr<io_service_t> io_serv,
        // connection related stuff:
        std::shared_ptr<boost::asio::io_service::strand> socket_strand,
        std::shared_ptr<cppalls::detail::udp_socket> socket,
        boost::asio::ip::udp::endpoint endpoint)
{
        return std::allocate_shared<udp_connection_fast>(
            cppalls::detail::shared_allocator_friend<udp_connection_fast>(),
            std::move(parent),
            std::move(processor),
            std::move(io_serv),
            std::move(socket_strand), std::move(socket), std::move(endpoint)
        );
    }
};


class udp final: public cppalls::api::application {
    std::shared_ptr<logger_t>                           log_;
    std::shared_ptr<connection_processor_t>             processor_;
    std::shared_ptr<io_service_t>                       io_service_;

    std::shared_ptr<boost::asio::io_service::strand>    socket_strand_;
    std::shared_ptr<cppalls::detail::udp_socket>        socket_;
    boost::asio::ip::udp::endpoint                      new_endpoint_;
    char                                                tmp_buf_[1];

    // no mutex required, because we ensure in logic that only single acceptor thread exists and 
    // operates on connections_ or allocator_
    std::unordered_map<
        boost::asio::ip::udp::endpoint,
        std::weak_ptr<udp_connection_fast>,
        endpoint_hash
    >   connections_;
    network::handler_allocator allocator_; 

    std::shared_ptr<udp> shared_from_this() {
        return std::static_pointer_cast<udp>(cppalls::api::application::shared_from_this());
    }

    void accept() {
        socket_->async_receive_from(
            boost::asio::buffer(tmp_buf_),
            new_endpoint_,
            cppalls::detail::udp_socket::message_peek,
            socket_strand_->wrap(
                network::make_custom_alloc_handler(allocator_, std::bind(
                    &udp::on_accpet,
                    shared_from_this(),
                    std::placeholders::_1,  // error_code
                    std::placeholders::_2,  // bytes_received
                    socket_                 // keeping socket for correct reload calls
                ))
            )
        );
    }

    void process_incoming_packet(const boost::system::error_code& error) {
        if (error){
            LWARN(log_) << "Error during UDP accept: " << error;
            return;
        }

        const auto it = connections_.find(new_endpoint_);
        if (it == connections_.cend() || !it->second.lock()) {
            auto c = udp_connection_fast::create(cppalls::api::application::shared_from_this(), processor_, io_service_, socket_strand_, socket_, new_endpoint_);
            std::weak_ptr<udp_connection_fast> weak(c);
            connections_.insert(std::make_pair(
                std::move(new_endpoint_),
                std::move(weak)
            ));

            // We do not whant to block accepting data from different endpoints
            // while this one is being processed, so posting a task.
            auto p = processor_;
            io_service_->io_service().post(
                [p, c](){ (*p)(*c); }
            );
        }

        // We have datagram for an already known endpoint. Here's a good place to do some cleanup
        // before accepting next datagram. This will give some time for a processor_ to take care of connection
        for (auto it = connections_.begin(); it != connections_.end(); /* intentional: no increment for `it`! */) {
            if (!it->second.lock()) {
                it = connections_.erase(it);
            } else {
                ++ it;
            }
        }
    }

    void on_accpet(const boost::system::error_code& error, std::size_t /*bytes_received*/, std::shared_ptr<cppalls::detail::udp_socket>& /*ignore*/) {
        try {
            process_incoming_packet(error);
        } catch (...) {
            LERROR(log_) << "Error during UDP accept: " << boost::current_exception_diagnostic_information();
        }

        if (error != boost::asio::error::operation_aborted) {
            accept();
        }
    }

    void reset_ptrs_common() noexcept {
        log_.reset();
        processor_.reset();
        socket_strand_.reset();
        socket_.reset();
        io_service_.reset();
        connections_.clear();
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


        auto listen_endpoint = boost::asio::ip::udp::endpoint(
            boost::asio::ip::address::from_string(conf["listen-address"].as<std::string>("0.0.0.0")),
            conf["listen-port"].as<unsigned short>()
        );

        socket_ = std::make_shared<boost::asio::ip::udp::socket>(
            io_service_->io_service(),
            std::move(listen_endpoint)
        );

        socket_strand_ = std::make_shared<boost::asio::io_service::strand>(
            io_service_->io_service()
        );

        if (conf["reuse-address"].as<bool>(true)) {
            socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
        }

        accept();
    }

    void reload(const YAML::Node& conf) override {
        reset_ptrs_common();
        start(conf);
    }

    void stop() override {
        if (socket_) {
            auto s = socket_;
            io_service_->io_service().post(
                socket_strand_->wrap([s]() {
                    boost::system::error_code ignore;
                    s->shutdown(boost::asio::ip::udp::socket::shutdown_both, ignore);
                    s->close(ignore);
                })
            );
        }

        reset_ptrs_common();
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new udp());
    }

    ~udp() override {
        stop();
    }
};

} // anonymous namespace

CPPALLS_APPLICATION(udp::create, udp_acceptor)
