#ifndef CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP
#define CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP

#include "stack_pimpl.hpp"

namespace boost { namespace asio {

template <typename Protocol>
class stream_socket_service;

template <typename Protocol, typename StreamSocketService>
class basic_stream_socket;

}} // namespace boost::asio

namespace boost { namespace asio { namespace ip {
    class tcp;
}}} // namespace boost::asio::ip


namespace boost { namespace system {
    class error_code;
}} // namespace boost::system

namespace cppalls { namespace detail {

    typedef boost::asio::basic_stream_socket<
        boost::asio::ip::tcp,
        boost::asio::stream_socket_service<boost::asio::ip::tcp>
    > tcp_socket;

    template <>
    struct lazy_size<tcp_socket> : size_constant<
        32
    > {};

}} // namespace cppalls::detail

#endif // CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP
