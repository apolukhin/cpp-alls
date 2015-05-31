#ifndef CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
#define CPPALS_SRC_CORE_SYNC_CONNECTION_IPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>

namespace {
    boost::asio::io_service io_service;
} // anonymous namespace

namespace cppalls { namespace detail {

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


}} // namespace cppalls::detail

#endif // CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
