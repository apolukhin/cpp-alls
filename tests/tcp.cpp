#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/logging.hpp"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/endian/arithmetic.hpp>
#include <memory>

using namespace cppalls;

struct server_guard {
    server_guard(const char* config_path = "../../cpp-alls/tests/server_core_config.yaml") {
        server::start(config_path);
    }

    ~server_guard() {
        server::stop();
    }
};

template <class T>
void test_send_receive_generic(boost::asio::io_service& io_service, T from, T to, const char* port) {
    using boost::asio::ip::tcp;
    for (T i = from; i < to; ++i) {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::asio::ip::tcp::tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        auto val = boost::endian::native_to_little(i);
        boost::array<unsigned char, sizeof(val)> buf;
        std::memcpy(&buf[0], &val, sizeof(val));
        val = 0;

        size_t len = socket.send(boost::asio::buffer(buf, sizeof(val)));
        ASSERT_TRUE(len == sizeof(val));
        std::memset(&buf[0], 0, sizeof(val));

        boost::asio::read(socket, boost::asio::buffer(buf));
        std::memcpy(&val, &buf[0], sizeof(val));

        const auto res = boost::endian::little_to_native(val);
        ASSERT_EQ(res, i);
    }
}

template <class T>
void test_send_receive_generic_noclose(boost::asio::io_service& io_service, T from, T to, const char* port) {
    using boost::asio::ip::tcp;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("127.0.0.1", port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::ip::tcp::tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    for (T i = from; i < to; ++i) {
        auto val = boost::endian::native_to_little(i);
        boost::array<unsigned char, sizeof(val)> buf;
        std::memcpy(&buf[0], &val, sizeof(val));
        val = 0;

        size_t len = socket.send(boost::asio::buffer(buf, sizeof(val)));
        ASSERT_TRUE(len == sizeof(val));
        std::memset(&buf[0], 0, sizeof(val));

        boost::asio::read(socket, boost::asio::buffer(buf));
        std::memcpy(&val, &buf[0], sizeof(val));

        const auto res = boost::endian::little_to_native(val);
        ASSERT_EQ(res, i);
    }
}

TEST(boost_tcp_acceptor, send_receive_closing) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp.yaml");

    boost::asio::io_service io_service;
    test_send_receive_generic<int>(io_service, -80000, -79900, "18080");
    test_send_receive_generic<short>(io_service, 800, 900, "18081");

    test_send_receive_generic_noclose<int>(io_service, -80000, -79900, "19080");
    test_send_receive_generic_noclose<short>(io_service, 800, 900, "19081");
}

