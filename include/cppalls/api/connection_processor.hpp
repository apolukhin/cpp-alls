#ifndef CPPALLS_API_CONNECTION_PROCESSOR
#define CPPALLS_API_CONNECTION_PROCESSOR

#include <cppalls/api/application.hpp>
#include <cppalls/core/connection.hpp>
#include <memory>

namespace cppalls { namespace api {

class connection_processor: public application {
public:
    CPPALLS_EXPORT static const decltype(std::placeholders::_1)& place_connection;
    CPPALLS_EXPORT static const decltype(std::placeholders::_2)& place_error;

    virtual void operator()(connection&) = 0;
};

}} // namespace cppalls::api

#endif // CPPALLS_API_CONNECTION_PROCESSOR
