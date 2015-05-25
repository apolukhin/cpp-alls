#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/tcp_connection.hpp"
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
void test_send_receive_generic(T from, T to, const char* port) {
    for (T i = from; i < to; ++i) {
        auto socket = tcp_connection::create("127.0.0.1", port);
        *socket << i;
        socket->write();

        socket->read(sizeof(T));
        T res;
        *socket >> res;
        ASSERT_EQ(res, i);
    }
}

template <class T>
void test_send_receive_generic_noclose(T from, T to, const char* port) {
    auto socket = tcp_connection::create("127.0.0.1", port);

    for (T i = from; i < to; ++i) {
        *socket << i;
        socket->write();

        socket->read(sizeof(T));
        T res;
        *socket >> res;
        ASSERT_EQ(res, i);
    }
}

TEST(boost_tcp_acceptor, send_receive_closing) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp.yaml");

    test_send_receive_generic<int>(-80000, -79900, "18080");
    test_send_receive_generic<short>(800, 900, "18081");

    test_send_receive_generic_noclose<int>(-80000, -79900, "19080");
    test_send_receive_generic_noclose<short>(800, 900, "19081");
}

