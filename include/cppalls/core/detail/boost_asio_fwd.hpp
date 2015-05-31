#ifndef CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP
#define CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP

#include "stack_pimpl.hpp"

namespace boost { namespace asio {

template <typename Protocol>
class stream_socket_service;

template <typename Protocol, typename StreamSocketService>
class basic_stream_socket;


template <typename Protocol>
class datagram_socket_service;

template <typename Protocol, typename DatagramSocketService>
class basic_datagram_socket;

}} // namespace boost::asio

namespace boost { namespace asio { namespace ip {
    class tcp;
    class udp;
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


    typedef boost::asio::basic_datagram_socket<
        boost::asio::ip::udp,
        boost::asio::datagram_socket_service<boost::asio::ip::udp>
    > udp_socket;

    template <>
    struct lazy_size<udp_socket> : size_constant<
        32
    > {};

}} // namespace cppalls::detail

#endif // CPPALLS_CORE_DETAIL_BOOST_ASIO_FWD_HPP
