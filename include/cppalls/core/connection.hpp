#ifndef CPPALLS_CORE_CONNECTION_HPP
#define CPPALLS_CORE_CONNECTION_HPP

#include "request.hpp"
#include "response.hpp"

namespace cppalls {

class connection {
public:
    connection() = default;
    connection(const connection&) = delete;
    connection(connection&&) = delete;
    connection& operator=(const connection&) = delete;
    connection& operator=(connection&&) = delete;

    typedef std::function<void(connection&, const std::error_code&)> callback_t;

    virtual ~connection() {}

    virtual void async_read(callback_t&&, std::size_t size) = 0;
    virtual void async_write(callback_t&&) = 0;
    virtual void close() = 0;
    virtual request& request() noexcept = 0;
    virtual response& response() noexcept = 0;

    template <class T>
    request& operator>>(const T& val) {
        return request() >> val;
    }

    template <class T>
    response& operator<<(const T& val) {
        return response() >> val;
    }
};

} // namespace cppalls

#endif // CPPALLS_CORE_CONNECTION_HPP
