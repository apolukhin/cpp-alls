#include <cppalls/core/tcp_connection.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include "sync_connection.ipp"

namespace cppalls {

tcp_connection_raw::tcp_connection_raw(const char *address, const char *port)
    : base_t(address, port)
{
    s_->set_option(boost::asio::ip::tcp::no_delay(true));
}

void tcp_connection_raw::read(std::size_t size) {
    base_t::read_without_size(size);
}

void tcp_connection_raw::write() {
    base_t::write_witout_size();
}

tcp_connection_raw::~tcp_connection_raw() {
    try {
        close();
    } catch (...) {}
}


tcp_connection::tcp_connection(const char *address, const char *port)
    : base_t(address, port)
{
    s_->set_option(boost::asio::ip::tcp::no_delay(true));
}

void tcp_connection::read(std::size_t /*size*/) {
    base_t::read_with_size();
}

void tcp_connection::write() {
    base_t::write_with_size();
}

tcp_connection::~tcp_connection() {
    try {
        close();
    } catch (...) {}
}

} // namespace cppalls
