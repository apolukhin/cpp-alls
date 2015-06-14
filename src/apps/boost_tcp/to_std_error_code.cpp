#include "to_std_error_code.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/ssl/error.hpp>

namespace network {

namespace {

template <class Adopt>
class adopt_category : public Adopt, public std::error_category {
    const char* name() const noexcept override {
        return Adopt::name();
    }

    std::string message(int value) const override {
        return Adopt::message(value);
    }
};

static const adopt_category<boost::asio::error::detail::netdb_category>     netdb_category_instance;
static const adopt_category<boost::asio::error::detail::addrinfo_category>  addrinfo_category_instance;
static const adopt_category<boost::asio::error::detail::misc_category>      misc_category_instance;
static const adopt_category<boost::asio::error::detail::ssl_category>       ssl_category_instance;

} // anonymous namesapce

std::error_code to_std_error_code(const boost::system::error_code& error) noexcept {
    const auto* const category = &error.category();

    if (category == &boost::asio::error::get_system_category()) {
        return std::make_error_code( static_cast<std::errc>(error.value()) );
    } else if (category == &boost::asio::error::get_netdb_category()) {
        return std::error_code(error.value(), netdb_category_instance);
    } else if (category == &boost::asio::error::get_addrinfo_category()) {
        return std::error_code(error.value(), addrinfo_category_instance);
    } else if (category == &boost::asio::error::get_misc_category()) {
        return std::error_code(error.value(), misc_category_instance);
    } if (category == &boost::asio::error::get_ssl_category()) {
        return std::error_code(error.value(), ssl_category_instance);
    }

    return std::make_error_code( static_cast<std::errc>(error.value()) );
}

} // namespace network

