#include <cppalls/core/tcp_connection.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include "sync_connection.ipp"

namespace cppalls {

tcp_connection::tcp_connection(const char *address, const char *port)
    : base_t(address, port)
{
    s_->set_option(boost::asio::ip::tcp::no_delay(true));
}

void tcp_connection::close() {
    base_t::close();
}

void tcp_connection::read(std::size_t size) {
    base_t::read(size);
}

void tcp_connection::write() {
    base_t::write();
}

tcp_connection::~tcp_connection(){}

} // namespace cppalls
