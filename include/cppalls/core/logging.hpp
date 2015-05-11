#ifndef CPPALLS_LOGGING_HPP
#define CPPALLS_LOGGING_HPP

#include <sstream>
#include "cppalls/api/logger.hpp"

namespace cppalls { namespace detail {

class logger_stream: public std::ostringstream {
    typedef std::ostringstream base_t;
    api::logger& log_;
    const api::logger::severity_level lvl_;
public:

    template <class T>
    explicit logger_stream(const std::shared_ptr<T>& log, api::logger::severity_level lvl)
        : log_(*log)
        , lvl_(lvl)
    {}

    explicit logger_stream(api::logger& log, api::logger::severity_level lvl) noexcept
        : log_(log)
        , lvl_(lvl)
    {}

    ~logger_stream() {
        if (base_t::str().empty()) {
            return;
        }

        try {
            log_.log(lvl_, base_t::str().c_str());
        } catch (...) {}
    }
};

#define LTRACE(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::TRACE)
#define LDEBUG(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::DEBUG)
#define LINFO(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::INFO)
#define LWARN(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::WARNING)
#define LERROR(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::ERROR)
#define LFATAL(L) ::cppalls::detail::logger_stream(L, cppalls::api::logger::severity_level::FATAL)

}} // namespace cppalls::detail

# endif // CPPALLS_LOGGING_HPP
