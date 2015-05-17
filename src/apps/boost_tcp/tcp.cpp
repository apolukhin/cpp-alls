#include <cppalls/core/server.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/connection.hpp>
#include <cppalls/core/stack_request.hpp>
#include <cppalls/core/stack_response.hpp>
#include <cppalls/api/io_service.hpp>
#include <cppalls/api/logger.hpp>
#include <cppalls/api/connection_processor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>
#include <memory>
#include <functional>
#include <yaml-cpp/yaml.h>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

#include <iostream>

namespace {

class tcp;
class tcp_connection;

typedef cppalls::connection::callback_t     callback_t;
typedef cppalls::api::io_service_provider   io_service_t;
typedef cppalls::api::connection_processor  connection_processor_t;
typedef cppalls::api::logger                logger_t;


class tcp_connection : public cppalls::connection, public std::enable_shared_from_this<tcp_connection> {
    // We do not keep parent_ alive to make sure, that DLL won't be unloaded while tcp_connections are running.
    // This application server must take care of it!
    // std::shared_ptr<tcp>                    parent_;

    std::shared_ptr<logger_t>               log_;
    std::shared_ptr<connection_processor_t> processor_;
    std::shared_ptr<io_service_t>           io_service_;
    boost::asio::ip::tcp::socket            s_;

    cppalls::stack_request                  request_;
    cppalls::stack_response                 response_;



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
            connection_->response_.clear();
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };

public:
    explicit tcp_connection(std::shared_ptr<logger_t>&& log, std::shared_ptr<connection_processor_t>&& processor, std::shared_ptr<io_service_t>&& io_service)
        : log_(std::move(log))
        , processor_(std::move(processor))
        , io_service_(std::move(io_service))
        , s_(io_service_->io_service())
    {}

    void async_write(callback_t&& cb) override {
        boost::asio::async_write(
            socket(),
            boost::asio::buffer(response_.begin(), response_.end() - response_.begin()),
            callback_write_wrapped(std::move(cb), shared_from_this())
        );
    }

    void async_read(callback_t&& cb, std::size_t size) override {
        request_.clear();
        request_.resize(size);
        boost::asio::async_read(
            socket(),
            boost::asio::buffer(request_.begin(), size),
            callback_read_wrapped(std::move(cb), shared_from_this())
        );
    }

    cppalls::request& request() noexcept override {
        return request_;
    }

    cppalls::response& response() noexcept override {
        return response_;
    }

    void close() override {
        s_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        s_.close();
    }

    boost::asio::ip::tcp::socket& socket() noexcept {
        return s_;
    }

    static std::shared_ptr<tcp_connection> create(std::shared_ptr<logger_t> log, std::shared_ptr<connection_processor_t> processor, std::shared_ptr<io_service_t> io_serv) {
        return std::make_shared<tcp_connection>(std::move(log), std::move(processor), std::move(io_serv));
    }

    void start() {
        auto& processor = *processor_;
        processor(*shared_from_this());
    }

    ~tcp_connection() override {
        boost::system::error_code ignore;
        s_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
        s_.close(ignore);
    }
};

class tcp : public cppalls::api::application {
    std::shared_ptr<logger_t>                       log_;
    std::shared_ptr<connection_processor_t>         processor_;
    std::shared_ptr<io_service_t>                   io_service_;

    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

    std::shared_ptr<tcp> shared_from_this() {
        return std::static_pointer_cast<tcp>(cppalls::api::application::shared_from_this());
    }

    void accept() {
        auto c = tcp_connection::create(log_, processor_, io_service_);
        auto& socket = c->socket();
        acceptor_->async_accept(
            socket,
            std::bind(&tcp::on_accpet, shared_from_this(), std::move(c), std::placeholders::_1)
        );
    }

    void on_accpet(std::shared_ptr<tcp_connection> c, const boost::system::error_code& error) {
        accept();

        if (!error) {
            c->start();
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

        accept();
    }

    void stop() {
        if (acceptor_) {
            acceptor_->close();
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

} // anonymous namespace

BOOST_DLL_ALIAS_SECTIONED(tcp::create, tcp_acceptor, cppalls)
