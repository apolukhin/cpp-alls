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

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>

#include <yaml-cpp/yaml.h>

#include "handler_allocator.hpp"
#include "async_connection_fast.hpp"

namespace {

typedef cppalls::api::io_service_provider   io_service_t;
typedef cppalls::api::connection_processor  connection_processor_t;
typedef cppalls::api::logger                logger_t;

class tcp_ssl_connection_fast final: public network::async_connection_fast<cppalls::detail::tcp_ssl_socket> {
    std::shared_ptr<boost::asio::ssl::context>      context_;

    explicit tcp_ssl_connection_fast(
              std::shared_ptr<cppalls::api::application>&& parent
            , std::shared_ptr<connection_processor_t>&& processor
            , std::shared_ptr<io_service_t>&& io_service
            , std::shared_ptr<boost::asio::ssl::context>&& context)
        : async_connection_fast(std::move(parent), std::move(processor), std::move(io_service), *context)
        , context_(std::move(context))
    {}

    template <class> friend class cppalls::detail::shared_allocator_friend;

public:
    static std::shared_ptr<tcp_ssl_connection_fast> create(
          std::shared_ptr<cppalls::api::application> parent
        , std::shared_ptr<connection_processor_t> processor
        , std::shared_ptr<io_service_t> io_serv
        , std::shared_ptr<boost::asio::ssl::context> context)
    {
        return std::allocate_shared<tcp_ssl_connection_fast>(
            cppalls::detail::shared_allocator_friend<tcp_ssl_connection_fast>(),
            std::move(parent),
            std::move(processor),
            std::move(io_serv),
            std::move(context)
        );
    }

    void start() {
        socket().async_handshake(
            boost::asio::ssl::stream_base::server,
            network::make_custom_alloc_handler(
                allocator(),
                std::bind(
                    &tcp_ssl_connection_fast::handle_handshake,
                    std::static_pointer_cast<tcp_ssl_connection_fast>(shared_from_this()),
                    std::placeholders::_1
                )
            )
        );
    }

    void handle_handshake(const boost::system::error_code& error) {
        if (error) {
            LWARN(log_) << "Error during handshake: " << error;
            return;
        }

        processor_->operator()(*this);
    }

    void close() override {
        socket().shutdown();
    }

    ~tcp_ssl_connection_fast() {
        boost::system::error_code ignore;
        socket().shutdown(ignore);

    }
};



class tcp_ssl final: public cppalls::api::application {
    std::shared_ptr<logger_t>                       log_;
    std::shared_ptr<connection_processor_t>         processor_;
    std::shared_ptr<io_service_t>                   io_service_;

    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::shared_ptr<boost::asio::ssl::context>      context_;
    bool                                            nodelay_;

    std::shared_ptr<tcp_ssl> shared_from_this() {
        return std::static_pointer_cast<tcp_ssl>(cppalls::api::application::shared_from_this());
    }

    void accept() {
        auto c = tcp_ssl_connection_fast::create(cppalls::api::application::shared_from_this(), processor_, io_service_, context_);
        auto& socket = c->socket();
        auto& allocator = c->allocator();
        acceptor_->async_accept(
            socket.lowest_layer(),
            network::make_custom_alloc_handler(
                allocator,
                std::bind(
                    &tcp_ssl::on_accpet,
                    shared_from_this(),
                    std::move(c),
                    std::placeholders::_1, // error_code
                    acceptor_               // required for reload, context_ is kept alive inside tcp_ssl_connection_fast's `c`
                )
            )
        );
    }

    void on_accpet(std::shared_ptr<tcp_ssl_connection_fast>& c, const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::acceptor>& /*ignore*/) {
        if (error != boost::asio::error::operation_aborted) {
            accept();
        } else {
            return;
        }

        if (!error) {
            c->socket().lowest_layer().set_option(boost::asio::ip::tcp::no_delay(nodelay_));
            c->start();
        } else {
            LWARN(log_) << "Error during accept: " << error;
        }
    }

public:
    void start(const YAML::Node& conf) override {
        log_ = cppalls::app::get<logger_t>(
            conf["logger"].as<std::string>()
        );

        processor_ = cppalls::app::get<connection_processor_t>(
            conf["processor"].as<std::string>()
        );

        io_service_ = cppalls::app::get<io_service_t>(
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

        context_ = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

        if (conf["password"]) {
            auto p = conf["password"].as<std::string>("");
            context_->set_password_callback([p](std::size_t, boost::asio::ssl::context::password_purpose){
                return p;
            });
        }

        context_->use_certificate_chain_file(
            conf["certificate-chain-file"].as<std::string>("server.pem")
        );
        context_->use_private_key_file(
            conf["private-key-file"].as<std::string>("server.pem")
            , boost::asio::ssl::context::pem
        );
        context_->use_tmp_dh_file(
            conf["tmp-dh-file"].as<std::string>("dh512.pem")
        );

        accept();
    }

    void stop() override {
        if (acceptor_) {
            acceptor_->close();
            acceptor_.reset();
        }
        context_.reset();

        log_.reset();
        processor_.reset();
        io_service_.reset();
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new tcp_ssl());
    }

    ~tcp_ssl() override {
        stop();
    }
};

} // anonymous namespace

CPPALLS_APPLICATION(tcp_ssl::create, tcp_ssl_acceptor)
