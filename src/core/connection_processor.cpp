#include <cppalls/api/connection_processor.hpp>
namespace cppalls { namespace api {

    const decltype(std::placeholders::_1)& connection_processor::place_connection = std::placeholders::_1;
    const decltype(std::placeholders::_2)& connection_processor::place_error = std::placeholders::_2;

}} // namespace cppalls::api
