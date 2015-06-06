#include "cppalls/api/connection_processor.hpp"
#include <cstring>
#include <functional>


// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

#include <string>
#include <iostream>

using namespace cppalls;

namespace {

template <class T, bool Close = true>
class echo_connection_processor : public cppalls::api::connection_processor {
public:
    typedef echo_connection_processor<T, Close> this_t;

    void on_write(connection& c, const std::error_code& ec) {
        if (ec) {
            //std::cerr << ec.message() << '\n';
            c.close();
        } else if (Close) {
            c.close();
        } else {
            (*this)(c);
        }
    }

    void on_read(connection& c, const std::error_code& ec) {
        if (ec) {
            //std::cerr << ec.message() << '\n';
            return;
        }

        T val;
        c >> val;
        c << val;
        c.async_write(
            std::bind(&this_t::on_write, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

    void operator()(connection& c) override {
        c.async_read(
            std::bind(&this_t::on_read, this, std::placeholders::_1, std::placeholders::_2),
            sizeof(T)
        );
    }

    void start(const YAML::Node& /*conf*/) override {}
    void stop() override {}

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new this_t());
    }
};


template <bool Close = true>
class echo_connection_processor_string : public cppalls::api::connection_processor {
public:
    typedef echo_connection_processor_string<Close> this_t;

    void on_write(connection& c, const std::error_code& ec) {
        if (ec) {
            //std::cerr << ec.message() << '\n';
            c.close();
        } else if (Close) {
            c.close();
        } else {
            (*this)(c);
        }
    }

    void on_read2(connection& c, const std::error_code& ec) {
        if (ec) {
            std::cerr << ec.message() << '\n';
            return;
        }

        std::string s(c.request().begin(), c.request().end());
        c << s;
        c.async_write(
            std::bind(&this_t::on_write, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

    void on_read1(connection& c, const std::error_code& ec) {
        if (ec) {
            //std::cerr << ec.message() << '\n';
            return;
        }

        unsigned int size;
        c >> size;
        c.async_read(
            std::bind(&this_t::on_read2, this, std::placeholders::_1, std::placeholders::_2),
            size
        );
    }

    void operator()(connection& c) override {
        c.async_read(
            std::bind(&this_t::on_read1, this, std::placeholders::_1, std::placeholders::_2),
            sizeof(unsigned int)
        );
    }

    void start(const YAML::Node& /*conf*/) override {}
    void stop() override {}

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new this_t());
    }
};

}

BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor<int>::create, echo_int_connection, cppalls)
BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor<short>::create, echo_short_connection, cppalls)
BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor_string<>::create, echo_string_connection, cppalls)


BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor<int, false>::create), echo_int_connection_noclose, cppalls)
BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor<short, false>::create), echo_short_connection_noclose, cppalls)
BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor_string<false>::create), echo_string_connection_noclose, cppalls)
