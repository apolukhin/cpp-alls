#ifndef CPPALLS_TCP_CONNECTION_HPP
#define CPPALLS_TCP_CONNECTION_HPP

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
#include <yaml-cpp/yaml.h>


namespace cppalls {

class CORE_EXPORT tcp_connection final: public cppalls::connection, public std::enable_shared_from_this<tcp_connection> {
    std::shared_ptr<api::logger>                log_;
    std::shared_ptr<api::connection_processor>  processor_;
    std::shared_ptr<api::io_service_provider>   io_service_;
    cppalls::detail::stack_pimpl<
        cppalls::detail::tcp_socket
    > s_;

    cppalls::stack_request                      request_;
    cppalls::stack_response                     response_;


    static inline std::error_code to_std_error_code(const boost::system::error_code& error) noexcept;

    explicit tcp_connection(std::shared_ptr<api::logger>&& log, std::shared_ptr<api::connection_processor>&& processor, std::shared_ptr<api::io_service_provider>&& io_service);

    template <class> friend class cppalls::detail::shared_allocator_friend;

public:
    void async_write(callback_t&& cb) override;
    void async_read(callback_t&& cb, std::size_t size) override;
    void write();
    void read(std::size_t size);
    void close() override;
    cppalls::detail::tcp_socket& socket() noexcept;
    ~tcp_connection() override;

    cppalls::request& request() noexcept override {
        return request_;
    }

    cppalls::response& response() noexcept override {
        return response_;
    }

    static std::shared_ptr<tcp_connection> create(std::shared_ptr<api::logger> log, std::shared_ptr<api::connection_processor> processor, std::shared_ptr<api::io_service_provider> io_serv);
};

} // namespace cppalls

#endif // CPPALLS_TCP_CONNECTION_HPP
