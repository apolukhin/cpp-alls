#include <cppalls/core/tcp_ssl_connection.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include "sync_connection.ipp"

namespace cppalls {

tcp_ssl_connection_raw::tcp_ssl_connection_raw(const char *address, const char *port)
    : context_t(boost::asio::ssl::context::sslv23_client)
    , base_t(address, port, *static_cast<context_t&>(*this))
{
    s_->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
    s_->handshake(boost::asio::ssl::stream_base::client);
}

void tcp_ssl_connection_raw::read(std::size_t size) {
    base_t::read_without_size(size);
}

void tcp_ssl_connection_raw::write() {
    base_t::write_witout_size();
}

tcp_ssl_connection_raw::~tcp_ssl_connection_raw() {
    try {
        close();
    } catch (...) {}
}


tcp_ssl_connection::tcp_ssl_connection(const char *address, const char *port)
    : context_t(boost::asio::ssl::context::sslv23_client)
    , base_t(address, port, *static_cast<context_t&>(*this))
{
    s_->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
    s_->handshake(boost::asio::ssl::stream_base::client);
}

void tcp_ssl_connection::read(std::size_t /*size*/) {
    base_t::read_with_size();
}

void tcp_ssl_connection::write() {
    base_t::write_with_size();
}

tcp_ssl_connection::~tcp_ssl_connection() {
    try {
        close();
    } catch (...) {}
}

} // namespace cppalls
