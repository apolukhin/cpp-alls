#ifndef CPPALLS_CORE_EXCEPTIONS_HPP
#define CPPALLS_CORE_EXCEPTIONS_HPP

#include <stdexcept>

namespace cppalls {

class error_base : public std::exception {};

class error_runtime : public error_base {
    std::string message_;

public:
    error_runtime() = default;
    error_runtime(const error_runtime&) = default;
    error_runtime(error_runtime&&) = default;

    explicit error_runtime(const std::string& message)
        : message_(message)
    {}

    explicit error_runtime(std::string&& message) noexcept
        : message_(std::move(message))
    {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

} // namespace cppalls

# endif // CPPALLS_CORE_EXCEPTIONS_HPP
