#ifndef NETWORK_ASYNC_CONNECTION_FAST_HPP
#define NETWORK_ASYNC_CONNECTION_FAST_HPP

#include <cppalls/core/server.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/connection.hpp>
#include <cppalls/core/stack_request.hpp>
#include <cppalls/core/stack_response.hpp>
#include <cppalls/core/detail/boost_asio_fwd.hpp>
#include <cppalls/core/detail/stack_pimpl.hpp>
#include <cppalls/api/io_service.hpp>
#include <cppalls/api/logger.hpp>
#include <cppalls/api/connection_processor.hpp>
#include <memory>
#include <functional>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include "handler_allocator.hpp"
#include "to_std_error_code.hpp"

namespace network {

namespace detail {
    class async_connection_fast_base: public cppalls::connection, public std::enable_shared_from_this<async_connection_fast_base> {
    protected:
        cppalls::stack_request                  request_;
        cppalls::stack_response                 response_;
        network::handler_allocator              allocator_;

    public:
        inline network::handler_allocator& allocator() noexcept {
            return allocator_;
        }

        inline cppalls::request& request() noexcept override final {
            return request_;
        }

        inline cppalls::response& response() noexcept override final {
            return response_;
        }
    };


    struct callback_read_wrapped {
        cppalls::connection::callback_t                 callback_;
        std::shared_ptr<async_connection_fast_base>     connection_;

        inline callback_read_wrapped(cppalls::connection::callback_t&& callback, std::shared_ptr<async_connection_fast_base>&& connection)
            : callback_(std::move(callback))
            , connection_(std::move(connection))
        {}

        inline void operator()(const boost::system::error_code& error, std::size_t /*bytes_received*/) {
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };

    struct callback_write_wrapped {
        cppalls::connection::callback_t                 callback_;
        std::shared_ptr<async_connection_fast_base>     connection_;

        inline callback_write_wrapped(cppalls::connection::callback_t&& callback, std::shared_ptr<async_connection_fast_base>&& connection)
            : callback_(std::move(callback))
            , connection_(std::move(connection))
        {}

        inline void operator()(const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            connection_->async_connection_fast_base::response().clear();
            if (callback_) {
                callback_(*connection_, to_std_error_code(error));
            }
        }
    };
} // namespace detail

template <class Protocol>
class async_connection_fast: public ::network::detail::async_connection_fast_base {
protected:
    typedef cppalls::api::io_service_provider   io_service_t;
    typedef cppalls::api::connection_processor  connection_processor_t;
    typedef cppalls::api::logger                logger_t;

    // We do keep parent_ alive to make sure, that DLL won't be unloaded while tcp_connection_fasts are running.
    std::shared_ptr<cppalls::api::application> parent_;

    std::shared_ptr<logger_t>               log_;
    std::shared_ptr<connection_processor_t> processor_;
    std::shared_ptr<io_service_t>           io_service_; // keeping pointer to work right during restarts
    Protocol                                s_;

    typedef async_connection_fast<Protocol> this_t;

    template <class... Args>
    explicit async_connection_fast(std::shared_ptr<cppalls::api::application>&& parent,std::shared_ptr<connection_processor_t>&& processor, std::shared_ptr<io_service_t>&& io_service, Args&&... args)
        : parent_(std::move(parent))
        , processor_(std::move(processor))
        , io_service_(std::move(io_service))
        , s_(io_service_->io_service(), std::forward<Args>(args)...)
    {}

public:
    void async_write(cppalls::connection::callback_t&& cb) override {
        boost::asio::async_write(
            socket(),
            boost::asio::buffer(response_.begin(), response_.end() - response_.begin()),
            network::make_custom_alloc_handler(allocator_, detail::callback_write_wrapped(std::move(cb), this_t::shared_from_this()))
        );
    }

    void async_read(cppalls::connection::callback_t&& cb, std::size_t size) override {
        request_.clear();
        request_.resize(size);
        boost::asio::async_read(
            socket(),
            boost::asio::buffer(request_.begin(), size),
            network::make_custom_alloc_handler(allocator_, detail::callback_read_wrapped(std::move(cb), this_t::shared_from_this()))
        );
    }

    void close() override {
        socket().shutdown(Protocol::shutdown_both);
        socket().close();
    }

    inline Protocol& socket() noexcept {
        return s_;
    }

    void start() {
        auto& processor = *processor_;
        processor(*this);
    }

    ~async_connection_fast() override {
        boost::system::error_code ignore;
        socket().shutdown(Protocol::shutdown_both, ignore);
        socket().close(ignore);
    }
};

} // namespace network

#endif // NETWORK_ASYNC_CONNECTION_FAST_HPP
