#ifndef CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
#define CPPALS_SRC_CORE_SYNC_CONNECTION_IPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include <boost/container/small_vector.hpp>

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
}

template <class Socket>
void sync_connection<Socket>::write() {
    const auto size = response_.end() - response_.begin();
    std::size_t offset = 0;
    while (offset != size) {
        offset += socket().send(
            boost::asio::buffer(response_.begin() + offset, size - offset)
        );
    }

    response_.clear();
}

template <class Socket>
void sync_connection<Socket>::read(std::size_t size) {
    write();

    request_t::shrink();
    request_t::resize(size);

    std::size_t offset = 0;
    while (offset != size) {
        offset += socket().receive(
            boost::asio::buffer(request_t::begin() + offset, size - offset)
        );
    }
}

template <class Socket>
void sync_connection<Socket>::extract_data(unsigned char *data, std::size_t size) {
    if (request_t::pimpl_->size() < size + request_t::data_extracted_) {
        read(size + request_t::data_extracted_ - request_t::pimpl_->size());
    }

    request_t::extract_data(data, size);
}

template <class Socket>
void sync_connection<Socket>::close() {
    write();

    s_->shutdown(Socket::shutdown_both);
    s_->close();
}


template <class Socket>
sync_connection<Socket>::~sync_connection() {
    write();

    boost::system::error_code ignore;
    s_->shutdown(Socket::shutdown_both, ignore);
    s_->close(ignore);
}


}} // namespace cppalls::detail

#endif // CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
