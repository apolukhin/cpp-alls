#ifndef CPPALLS_API_IO_SERVICE_HPP
#define CPPALLS_API_IO_SERVICE_HPP

#include "application.hpp"

namespace boost { namespace asio {
    class io_service;
}}

namespace cppalls { namespace api {

class io_service_provider: public application {
public:
    virtual boost::asio::io_service& io_service() noexcept = 0;
};


}} // namespace cppalls::api

# endif // CPPALLS_API_IO_SERVICE_HPP
