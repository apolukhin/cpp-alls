#ifndef CPPALLS_API_CONNECTION_PROCESSOR
#define CPPALLS_API_CONNECTION_PROCESSOR

#include <cppalls/core/connection.hpp>
#include <cppalls/api/application.hpp>
#include <memory>

namespace cppalls { namespace api {

class connection_processor : public application {
public:
    virtual void operator()(connection&) = 0;
};

}} // namespace cppalls::api

#endif // CPPALLS_API_CONNECTION_PROCESSOR
