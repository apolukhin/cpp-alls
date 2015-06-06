#ifndef NETWORK_TO_STD_ERROR_CODE_HPP
#define NETWORK_TO_STD_ERROR_CODE_HPP

#include <boost/system/error_code.hpp>
#include <system_error>

namespace network {
    std::error_code to_std_error_code(const boost::system::error_code& error) noexcept;
}

#endif // NETWORK_TO_STD_ERROR_CODE_HPP
