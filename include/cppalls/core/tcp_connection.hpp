#ifndef CPPALLS_TCP_CONNECTION_HPP
#define CPPALLS_TCP_CONNECTION_HPP

#include <cppalls/core/server.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/stack_request.hpp>
#include <cppalls/core/stack_response.hpp>
#include <cppalls/core/export.hpp>
#include <cppalls/core/detail/boost_asio_fwd.hpp>
#include <cppalls/core/detail/stack_pimpl.hpp>
#include <cppalls/core/detail/shared_allocator_friend.hpp>
#include <cppalls/api/io_service.hpp>
#include <cppalls/api/logger.hpp>
#include <memory>
#include <functional>
#include <yaml-cpp/yaml.h>


namespace cppalls {

class CORE_EXPORT tcp_connection final {
    std::shared_ptr<api::io_service_provider>   io_service_;
    cppalls::detail::stack_pimpl<
        cppalls::detail::tcp_socket
    > s_;

    cppalls::stack_request                      request_;
    cppalls::stack_response                     response_;

    explicit tcp_connection(std::shared_ptr<api::io_service_provider>&& io_service);
    template <class> friend class cppalls::detail::shared_allocator_friend;

public:
    void write();
    void read(std::size_t size);
    void close();
    cppalls::detail::tcp_socket& socket() noexcept;
    ~tcp_connection();

    template <class T>
    cppalls::request& operator>>(T& val) {
        return request_ >> val;
    }

    template <class T>
    cppalls::response& operator<<(const T& val) {
        return response_ << val;
    }

    static std::shared_ptr<tcp_connection> create(std::shared_ptr<api::io_service_provider> io_serv);
    static std::shared_ptr<tcp_connection> create(const char* address, const char* port);
};

} // namespace cppalls

#endif // CPPALLS_TCP_CONNECTION_HPP
