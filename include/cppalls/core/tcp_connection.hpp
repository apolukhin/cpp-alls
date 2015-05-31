#ifndef CPPALLS_TCP_CONNECTION_HPP
#define CPPALLS_TCP_CONNECTION_HPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls {

class CPPALLS_EXPORT tcp_connection final : public cppalls::detail::sync_connection<cppalls::detail::tcp_socket> {
public:
    typedef cppalls::detail::sync_connection<cppalls::detail::tcp_socket>  base_t;

    explicit tcp_connection(const char* address, const char* port);

    void write();
    void read(std::size_t size);
    void close();
    ~tcp_connection();
};

} // namespace cppalls

#endif // CPPALLS_TCP_CONNECTION_HPP
