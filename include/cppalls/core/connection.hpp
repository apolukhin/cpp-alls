#ifndef CPPALLS_CORE_CONNECTION_HPP
#define CPPALLS_CORE_CONNECTION_HPP

#include "request.hpp"
#include "response.hpp"
#include <functional>
#include <system_error>
#include <type_traits>
#include <system_error>

namespace cppalls { namespace api {
    class connection_processor;
}}

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
    virtual api::connection_processor& connection_processor() noexcept = 0;

    virtual void close() = 0;
    virtual cppalls::request& request() noexcept = 0;
    virtual cppalls::response& response() noexcept = 0;

    template <class T>
    cppalls::request& operator>>(T& val) {
        return request() >> val;
    }

    template <class T>
    cppalls::response& operator<<(const T& val) {
        return response() << val;
    }


    virtual void async_read(callback_t&&, std::size_t size) = 0;

    template <class Ret, class Processor, class ErrorCode>
    inline void async_read(Ret (Processor::*function)(connection&, ErrorCode), std::size_t size) {
        async_read_generic<Processor>(function, std::placeholders::_1, std::placeholders::_2, size);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_read(Ret (Processor::*function)(connection&, ErrorCode) const, std::size_t size) {
        async_read_generic<Processor>(function, std::placeholders::_1, std::placeholders::_2, size);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_read(Ret (Processor::*function)(ErrorCode, connection&), std::size_t size) {
        async_read_generic<Processor>(function, std::placeholders::_2, std::placeholders::_1, size);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_read(Ret (Processor::*function)(ErrorCode, connection&) const, std::size_t size) {
        async_read_generic<Processor>(function, std::placeholders::_2, std::placeholders::_1, size);
    }


    virtual void async_write(callback_t&& = callback_t()) = 0;

    template <class Ret, class Processor, class ErrorCode>
    inline void async_write(Ret (Processor::*function)(connection&, ErrorCode)) {
        async_write_generic<Processor>(function, std::placeholders::_1, std::placeholders::_2);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_write(Ret (Processor::*function)(connection&, ErrorCode) const) {
        async_write_generic<Processor>(function, std::placeholders::_1, std::placeholders::_2);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_write(Ret (Processor::*function)(ErrorCode, connection&)) {
        async_write_generic<Processor>(function, std::placeholders::_2, std::placeholders::_1);
    }

    template <class Ret, class Processor, class ErrorCode>
    inline void async_write(Ret (Processor::*function)(ErrorCode, connection&) const) {
        async_write_generic<Processor>(function, std::placeholders::_2, std::placeholders::_1);
    }

private:
    template <class Processor, class Function, class Arg0, class Arg1>
    inline void async_read_generic(Function function, const Arg0& arg0, const Arg1& arg1, std::size_t size) {
        static_assert(
            std::is_base_of<cppalls::api::connection_processor, Processor>::value,
            "Function overload `connection::async_read(member pointer function, size)` works only if meber pointer function belongs to the class retuned by `connection_processor()`"
        );

        Processor* p = dynamic_cast<Processor*>(&connection_processor());
        if (!p) {
            throw std::logic_error(
                "Function overload `connection::async_read(member pointer function, size)` works only if meber pointer function belongs to the class retuned by `connection_processor()`"
            );
        }

        async_read(
            std::bind(function, p, arg0, arg1),
            size
        );
    }

    template <class Processor, class Function, class Arg0, class Arg1>
    inline void async_write_generic(Function function, const Arg0& arg0, const Arg1& arg1) {
        static_assert(
            std::is_base_of<cppalls::api::connection_processor, Processor>::value,
            "Function overload `connection::async_write(member pointer function)` works only if meber pointer function belongs to the class retuned by `connection_processor()`"
        );

        Processor* p = dynamic_cast<Processor*>(&connection_processor());
        if (!p) {
            throw std::logic_error(
                "Function overload `connection::async_write(member pointer function)` works only if meber pointer function belongs to the class retuned by `connection_processor()`"
            );
        }

        async_write(
            std::bind(function, p, arg0, arg1)
        );
    }

};

} // namespace cppalls

#endif // CPPALLS_CORE_CONNECTION_HPP
