#ifndef CPPALLS_API_PROCESSOR
#define CPPALLS_API_PROCESSOR

#include "connection_processor.hpp"

namespace cppalls { namespace api {

class CPPALLS_EXPORT processor : public connection_processor {
public:
    virtual void operator()(request&, response&) = 0;

    virtual void operator()(connection&) final;
};

}} // namespace cppalls::api

#endif // CPPALLS_API_PROCESSOR
