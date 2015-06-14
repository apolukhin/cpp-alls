#ifndef CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
#define CPPALS_SRC_CORE_SYNC_CONNECTION_IPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace cppalls { namespace detail {

template <class Socket>
void connect(Socket& s, const char* address, const char* port) {
    typedef typename Socket::protocol_type protocol_type;

    typename protocol_type::resolver resolver(s.get_io_service());
    typename protocol_type::resolver::query query(address, port);
    typename protocol_type::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::connect(s, endpoint_iterator);
}

inline void connect(tcp_ssl_socket& s, const char* address, const char* port) {
    cppalls::detail::connect(s.lowest_layer(), address, port);
}


template <class Socket>
template <class... Args>
sync_connection<Socket>::sync_connection(const char* address, const char* port, Args&&... args)
    : ios_()
    , s_(*ios_, args...)
{
    connect(*s_, address, port);
}

namespace {
    template <class Socket>
    void write_witout_size_impl(Socket& s, const unsigned char* begin, std::size_t size) {
        std::size_t offset = 0;
        while (offset != size) {
            offset += s.send(
                boost::asio::buffer(begin + offset, size - offset)
            );
        }
    }

    inline void write_witout_size_impl(tcp_ssl_socket& s, const unsigned char* begin, std::size_t size) {
        std::size_t offset = 0;
        while (offset != size) {
            offset += s.write_some(
                boost::asio::buffer(begin + offset, size - offset)
            );
        }
    }
} // anonymous namespace

template <class Socket>
void sync_connection<Socket>::write_witout_size() {
    const unsigned int size = response_.end() - response_.begin();
    write_witout_size_impl(socket(), response_.begin(), size);
    response_.clear();
}

template <class Socket>
void sync_connection<Socket>::write_with_size() {
    if (response_.end() - response_.begin() == 0) {
        return;
    }

    const unsigned int size = response_.end() - response_.begin() + sizeof(unsigned int);
    const unsigned int little_size = boost::endian::native_to_little(size);
    response_.put_data(reinterpret_cast<const unsigned char*>(&little_size), sizeof(little_size), 0);

    write_witout_size_impl(socket(), response_.begin(), size);

    response_.clear();
}

namespace {
    template <class Socket>
    void read_without_size_impl(Socket& s, unsigned char* begin, std::size_t size) {
        std::size_t offset = 0;
        while (offset != size) {
            offset += s.receive(
                boost::asio::buffer(begin + offset, size - offset)
            );
        }
    }

    inline void read_without_size_impl(tcp_ssl_socket& s, unsigned char* begin, std::size_t size) {
        std::size_t offset = 0;
        while (offset != size) {
            offset += s.read_some(
                boost::asio::buffer(begin + offset, size - offset)
            );
        }
    }
} // anonymous namespace

template <class Socket>
void sync_connection<Socket>::read_without_size(std::size_t size) {
    write();

    request_t::shrink();
    request_t::resize(size);
    read_without_size_impl(socket(), begin(), size);
}

namespace {
    template <class Socket>
    std::size_t data_size(Socket& s) {
        return s.available();
    }

    inline std::size_t data_size(tcp_ssl_socket& s) {
        unsigned int size;
        s.read_some(boost::asio::buffer(&size, sizeof(size)));
        boost::endian::little_to_native_inplace(size);

        return size;
    }

    template <class Stream>
    void assert_actual_size(Stream& s, std::size_t size) {
        unsigned int actual_size;
        s >> actual_size;
        if (size != actual_size) {
            boost::throw_exception(std::logic_error(
                "Packet sizes missmatch"
            ));
        }
    }

    inline void assert_actual_size(sync_connection<tcp_ssl_socket>& /*s*/, std::size_t /*size*/) {
        // Noop
    }
} // anonymous namespace

template <class Socket>
void sync_connection<Socket>::read_with_size() {
    write();

    request_t::shrink();
    const std::size_t size = data_size(socket());
    request_t::resize(size);
    read_without_size_impl(socket(), begin(), size);

    assert_actual_size(*this, size);
}

template <class Socket>
void sync_connection<Socket>::extract_data(unsigned char *data, std::size_t size) {
    if (request_t::pimpl_->size() < size + request_t::data_extracted_) {
        read(size + request_t::data_extracted_ - request_t::pimpl_->size());
    }

    request_t::extract_data(data, size);
}

namespace {
    template <class Socket>
    void shutdown_and_close_impl(Socket& s) {
        boost::system::error_code ignore;
        s.shutdown(Socket::shutdown_both, ignore);
        s.close(ignore);
    }

    inline void shutdown_and_close_impl(tcp_ssl_socket& s) {
        boost::system::error_code ignore;
        s.shutdown(ignore);
    }
} // anonymous namespace

template <class Socket>
void sync_connection<Socket>::close() {
    write();
    shutdown_and_close_impl(socket());

}


template <class Socket>
sync_connection<Socket>::~sync_connection() {}


}} // namespace cppalls::detail

#endif // CPPALS_SRC_CORE_SYNC_CONNECTION_IPP
