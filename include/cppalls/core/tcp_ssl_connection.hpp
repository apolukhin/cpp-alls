#ifndef CPPALLS_TCP_SSL_CONNECTION_HPP
#define CPPALLS_TCP_SSL_CONNECTION_HPP

#include <cppalls/core/detail/sync_connection.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls {

namespace detail {

}

class CPPALLS_EXPORT tcp_ssl_connection_raw final:
        public cppalls::detail::stack_pimpl<boost::asio::ssl::context>,
        public cppalls::detail::sync_connection<cppalls::detail::tcp_ssl_socket>
{
    typedef cppalls::detail::stack_pimpl<boost::asio::ssl::context> context_t;
public:
    typedef cppalls::detail::sync_connection<cppalls::detail::tcp_ssl_socket>  base_t;

    explicit tcp_ssl_connection_raw(const char* address, const char* port);

    void read(std::size_t size) override;
    void write() override;
    ~tcp_ssl_connection_raw();
};


class CPPALLS_EXPORT tcp_ssl_connection final:
        public cppalls::detail::stack_pimpl<boost::asio::ssl::context>,
        public cppalls::detail::sync_connection<cppalls::detail::tcp_ssl_socket>
{
    typedef cppalls::detail::stack_pimpl<boost::asio::ssl::context> context_t;
    void read(std::size_t size) override;
public:
    typedef cppalls::detail::sync_connection<cppalls::detail::tcp_ssl_socket>  base_t;

    explicit tcp_ssl_connection(const char* address, const char* port);
    ~tcp_ssl_connection();
    void write() override;
};

} // namespace cppalls

#endif // CPPALLS_TCP_SSL_CONNECTION_HPP
