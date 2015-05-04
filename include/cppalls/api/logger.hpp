#ifndef CPPALLS_API_LOGGER_HPP
#define CPPALLS_API_LOGGER_HPP

#include <sstream>
#include "application.hpp"

namespace cppalls { namespace api {

class logger: public application {
public:
    enum severity_level {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    virtual void log(severity_level lvl, const char* msg) = 0;
};


}} // namespace cppalls::api

# endif // CPPALLS_API_ASERVER_HPP
