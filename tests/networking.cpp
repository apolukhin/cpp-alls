#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/tcp_connection.hpp"
#include "cppalls/core/udp_connection.hpp"
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


template <class Connection>
void test_send_receive_str_generic(std::string s, size_t iterations, const char* port) {
    for (size_t i = 0; i < iterations; ++i) {
        Connection socket("127.0.0.1", port);
        socket << s;
        socket.write();

        socket.read(s.size() + sizeof(unsigned int));
        std::string res;
        socket >> res;
        ASSERT_EQ(res, s);
        s += s;
    }
}

template <class Connection>
void test_send_receive_str_generic_noclose(std::string s, size_t iterations, const char* port) {
    Connection socket("127.0.0.1", port);
    for (size_t i = 0; i < iterations; ++i) {
        socket << s;
        socket.write();

        socket.read(s.size() + sizeof(unsigned int));
        std::string res;
        socket >> res;
        ASSERT_EQ(res, s);
        s += s;
    }
}

template <class... Args>
void run_async(Args&&... args) {
    const unsigned threads_count = 8;

    boost::thread_group tg;
    for (unsigned i = 0; i < threads_count; ++i) {
        tg.create_thread(
            std::bind(std::forward<Args>(args)...)
        );
    }
    tg.join_all();
}

template <class Connection>
void do_networking_test() {
    test_send_receive_generic<int, Connection>(-80000, -79900, "18080");
    test_send_receive_generic<short, Connection>(800, 900, "18081");
    test_send_receive_str_generic<Connection>("Hello ", 18, "18082");

    test_send_receive_generic_noclose<int, Connection>(-80000, -79900, "19080");
    test_send_receive_generic_noclose<short, Connection>(800, 900, "19081");
    test_send_receive_str_generic_noclose<Connection>("Hell!O!", 18, "19082");

    run_async(&test_send_receive_generic<int, Connection>, -80000, -79900, "18080");
    run_async(&test_send_receive_generic<short, Connection>, -80000, -79900, "18081");
    run_async(&test_send_receive_str_generic<Connection>, "Hello ", 18, "18082");

    run_async(&test_send_receive_generic_noclose<int, Connection>, -80000, -79900, "19080");
    run_async(&test_send_receive_generic_noclose<short, Connection>, -80000, -79900, "19081");
    run_async(&test_send_receive_str_generic_noclose<Connection>, "Hello ", 18, "19082");
}

TEST(boost_tcp_acceptor, tcp_send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp.yaml");
    do_networking_test<tcp_connection>();
}
/*
TEST(boost_udp_acceptor, udp_send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_udp.yaml");
    do_networking_test<udp_connection>();
}
*/
