#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/tcp_connection.hpp"
#include <boost/array.hpp>
#include <boost/thread.hpp>
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

template <class T, class Connection>
void test_send_receive_generic(T from, T to, const char* port) {
    for (T i = from; i < to; ++i) {
        Connection socket("127.0.0.1", port);
        socket << i;
        socket.write();

        socket.read(sizeof(T));
        T res;
        socket >> res;
        ASSERT_EQ(res, i);
    }
}

template <class T, class Connection>
void test_send_receive_generic_noclose(T from, T to, const char* port) {
    Connection socket("127.0.0.1", port);

    for (T i = from; i < to; ++i) {
        socket << i;
        socket.write();

        socket.read(sizeof(T));
        T res;
        socket >> res;
        ASSERT_EQ(res, i);
    }
}

TEST(boost_tcp_acceptor, send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp.yaml");

    test_send_receive_generic<int, tcp_connection>(-80000, -79900, "18080");
    test_send_receive_generic<short, tcp_connection>(800, 900, "18081");

    test_send_receive_generic_noclose<int, tcp_connection>(-80000, -79900, "19080");
    test_send_receive_generic_noclose<short, tcp_connection>(800, 900, "19081");


    const unsigned threads_count = 8;
    boost::thread_group tg;
    for (unsigned i = 0; i < threads_count; ++i) {
        tg.create_thread(
            std::bind(&test_send_receive_generic<int, tcp_connection>, -80000, -79900, "18080")
        );
    }
    tg.join_all();

    for (unsigned i = 0; i < threads_count; ++i) {
        tg.create_thread(
            std::bind(&test_send_receive_generic<short, tcp_connection>, -80000, -79900, "18081")
        );
    }
    tg.join_all();


    for (unsigned i = 0; i < threads_count; ++i) {
        tg.create_thread(
            std::bind(&test_send_receive_generic_noclose<int, tcp_connection>, -80000, -79900, "19080")
        );
    }
    tg.join_all();

    for (unsigned i = 0; i < threads_count; ++i) {
        tg.create_thread(
            std::bind(&test_send_receive_generic_noclose<short, tcp_connection>, -80000, -79900, "19081")
        );
    }
    tg.join_all();
}
