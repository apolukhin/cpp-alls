#include <cppalls/core/tcp_connection.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>

namespace cppalls {

namespace {
    boost::asio::io_service io_service;
} // anonymous namespace




namespace detail {


template <class Socket>
sync_connection<Socket>::sync_connection(const char* address, const char* port)
    : s_(io_service)
{
    typedef typename Socket::protocol_type protocol_type;

    typename protocol_type::resolver resolver(io_service);
    typename protocol_type::resolver::query query(address, port);
    typename protocol_type::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::connect(*s_, endpoint_iterator);
    //s_->set_option(boost::asio::ip::tcp::no_delay(true));
}

template <class Socket>
void sync_connection<Socket>::write() {
    socket().send(
        boost::asio::buffer(response_.begin(), response_.end() - response_.begin())
    );

    response_.clear();
}

template <class Socket>
void sync_connection<Socket>::read(std::size_t size) {
    request_.clear();
    request_.resize(size);
    socket().receive(
        boost::asio::buffer(request_.begin(), size)
    );
}

template <class Socket>
void sync_connection<Socket>::close() {
    s_->shutdown(Socket::shutdown_both);
    s_->close();
}


template <class Socket>
sync_connection<Socket>::~sync_connection() {
    boost::system::error_code ignore;
    s_->shutdown(Socket::shutdown_both, ignore);
    s_->close(ignore);
}


} // namespace detail


tcp_connection::tcp_connection(const char *address, const char *port)
    : base_t(address, port)
{}

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
