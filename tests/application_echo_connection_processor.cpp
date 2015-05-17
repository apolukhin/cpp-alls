#include "cppalls/api/connection_processor.hpp"
#include <cstring>
#include <functional>


// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

#include <iostream>

using namespace cppalls;

namespace {

template <class T, bool Close = true>
class echo_connection_processor : public cppalls::api::connection_processor {
public:
    typedef echo_connection_processor<T, Close> this_t;

    std::shared_ptr<this_t> shared_from_this() {
        return std::static_pointer_cast<this_t>(cppalls::api::application::shared_from_this());
    }

    void on_write(connection& c) {
        if (Close) {
            c.close();
        } else {
            (*this)(c);
        }
    }

    void on_read(connection& c, const std::error_code& ec) {
        if (ec) {
            // TODO:
        }

        T val;
        c >> val;
        c << val;
        c.async_write(
            std::bind(&this_t::on_write, shared_from_this(), std::placeholders::_1)
        );
    }

    void operator()(connection& c) override {
        c.async_read(
            std::bind(&this_t::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2),
            sizeof(T)
        );
    }

    void start(const YAML::Node& /*conf*/) override {}
    void stop() override {}

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new this_t());
    }

    ~echo_connection_processor() {
        int i = 9;
        (void)i;
    }
};

}

BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor<int>::create, echo_int_connection, cppalls)
BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor<short>::create, echo_short_connection, cppalls)
BOOST_DLL_ALIAS_SECTIONED(echo_connection_processor<std::string>::create, echo_string_connection, cppalls)


BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor<int, false>::create), echo_int_connection_noclose, cppalls)
BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor<short, false>::create), echo_short_connection_noclose, cppalls)
BOOST_DLL_ALIAS_SECTIONED((echo_connection_processor<std::string, false>::create), echo_string_connection_noclose, cppalls)
