#ifndef CPPALLS_UDP_CONNECTION_HPP
#define CPPALLS_UDP_CONNECTION_HPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls {

class CPPALLS_EXPORT udp_connection final : public cppalls::detail::sync_connection<cppalls::detail::udp_socket> {
    void read(std::size_t /*size*/) override;

public:
    typedef cppalls::detail::sync_connection<cppalls::detail::udp_socket>  base_t;

    explicit udp_connection(const char* address, const char* port);

    void write();
    ~udp_connection();
};


} // namespace cppalls

#endif // CPPALLS_UDP_CONNECTION_HPP
