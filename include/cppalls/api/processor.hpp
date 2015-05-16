#ifndef CPPALLS_API_PROCESSOR
#define CPPALLS_API_PROCESSOR

#include <cppalls/connection.hpp>
#include <cppalls/request.hpp>
#include <cppalls/response.hpp>
#include "application.hpp"

namespace cppalls { namespace api {

class processor : public application {
public:
    virtual void operator()(request&, response&) = 0;
};

}} // namespace cppalls::api

#endif // CPPALLS_API_PROCESSOR
