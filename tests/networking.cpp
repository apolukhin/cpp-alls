#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/tcp_connection.hpp"
#include "cppalls/core/tcp_ssl_connection.hpp"
#include "cppalls/core/udp_connection.hpp"
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
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
void do_networking_test(unsigned short port_base) {
    test_send_receive_generic<int, Connection>(-80000, -79900, boost::lexical_cast<std::string>(port_base).c_str());
    test_send_receive_generic<short, Connection>(800, 900, boost::lexical_cast<std::string>(port_base + 1u).c_str());
    test_send_receive_str_generic<Connection>("Hello ", 18, boost::lexical_cast<std::string>(port_base + 2u).c_str());
    test_send_receive_generic_noclose<int, Connection>(-80000, -79900, boost::lexical_cast<std::string>(port_base + 1000u).c_str());
    test_send_receive_generic_noclose<short, Connection>(800, 900, boost::lexical_cast<std::string>(port_base + 1001u).c_str());
    test_send_receive_str_generic_noclose<Connection>("Hell!O!", 18, boost::lexical_cast<std::string>(port_base + 1002u).c_str());

    run_async(&test_send_receive_generic<int, Connection>, -80000, -79900, boost::lexical_cast<std::string>(port_base + 0u).c_str());
    run_async(&test_send_receive_generic<short, Connection>, -80000, -79900, boost::lexical_cast<std::string>(port_base + 1u).c_str());
    run_async(&test_send_receive_str_generic<Connection>, "Hello ", 18, boost::lexical_cast<std::string>(port_base + 2u).c_str());

    run_async(&test_send_receive_generic_noclose<int, Connection>, -80000, -79900, boost::lexical_cast<std::string>(port_base + 1000u).c_str());
    run_async(&test_send_receive_generic_noclose<short, Connection>, -80000, -79900, boost::lexical_cast<std::string>(port_base + 1001u).c_str());
    run_async(&test_send_receive_str_generic_noclose<Connection>, "Hello ", 18, boost::lexical_cast<std::string>(port_base + 1002u).c_str());
}

TEST(boost_tcp_acceptor, tcp_send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp.yaml");
    do_networking_test<tcp_connection_raw>(18080);
}


TEST(boost_tcp_acceptor, tcp_ssl_send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_tcp_ssl.yaml");
    do_networking_test<tcp_ssl_connection_raw>(10080);
}

/*
TEST(boost_udp_acceptor, udp_send_receive) {
    server_guard guard("../../cpp-alls/tests/server_core_config_udp.yaml");
    do_networking_test<udp_connection>();
}
*/
