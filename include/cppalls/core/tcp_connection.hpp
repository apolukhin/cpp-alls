#ifndef CPPALLS_TCP_CONNECTION_HPP
#define CPPALLS_TCP_CONNECTION_HPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls {

class CPPALLS_EXPORT tcp_connection_raw final: public cppalls::detail::sync_connection<cppalls::detail::tcp_socket> {
public:
    typedef cppalls::detail::sync_connection<cppalls::detail::tcp_socket>  base_t;

    explicit tcp_connection_raw(const char* address, const char* port);

    void read(std::size_t size) override;
    void write() override;
    ~tcp_connection_raw();
};


class CPPALLS_EXPORT tcp_connection final: public cppalls::detail::sync_connection<cppalls::detail::tcp_socket> {
    void read(std::size_t size) override;
public:
    typedef cppalls::detail::sync_connection<cppalls::detail::tcp_socket>  base_t;

    explicit tcp_connection(const char* address, const char* port);
    ~tcp_connection();
    void write() override;
};

} // namespace cppalls

#endif // CPPALLS_TCP_CONNECTION_HPP
