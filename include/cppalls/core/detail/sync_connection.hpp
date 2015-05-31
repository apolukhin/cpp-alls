#ifndef CPPALLS_CORE_DETAIL_SYNC_CONNECTION_HPP
#define CPPALLS_CORE_DETAIL_SYNC_CONNECTION_HPP

#include <cppalls/core/stack_request.hpp>
#include <cppalls/core/stack_response.hpp>
#include <cppalls/core/detail/boost_asio_fwd.hpp>
#include <cppalls/core/detail/stack_pimpl.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls { namespace detail {

template <class Socket>
class sync_connection {
    cppalls::detail::stack_pimpl<Socket>    s_;
    cppalls::stack_request                  request_;
    cppalls::stack_response                 response_;

public:
    explicit sync_connection(const char* address, const char* port);

    void write();
    void read(std::size_t size);
    void close();
    ~sync_connection();

    template <class T>
    cppalls::request& operator>>(T& val) {
        return request_ >> val;
    }

    template <class T>
    cppalls::response& operator<<(const T& val) {
        return response_ << val;
    }

    inline Socket& socket() noexcept {
        return *s_;
    }
};

}} // namespace cppalls::detail

#endif // CPPALLS_CORE_DETAIL_SYNC_CONNECTION_HPP
