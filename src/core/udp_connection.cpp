#include <cppalls/core/udp_connection.hpp>
#include <boost/asio/ip/udp.hpp>
#include "sync_connection.ipp"

namespace cppalls {

udp_connection::udp_connection(const char *address, const char *port)
    : base_t(address, port)
{}


void udp_connection::read(std::size_t /*size*/) {
    base_t::read_with_size();
}

void udp_connection::write() {
    base_t::write_with_size();
}

udp_connection::~udp_connection() {
    try {
        close();
    } catch (...) {}
}

} // namespace cppalls
