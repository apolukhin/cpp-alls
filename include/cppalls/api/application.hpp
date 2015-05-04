#ifndef CPPALLS_API_APPLICATION_HPP
#define CPPALLS_API_APPLICATION_HPP

#include <memory>

namespace cppalls { namespace api {

class application {
public:
    typedef std::unique_ptr<application>(constructor_t)();
    virtual ~application() noexcept {}

    struct version_t {
        unsigned short major;
        unsigned short minor;
        unsigned int patch;
    };

    virtual version_t version() const noexcept {
        version_t v = { 0, 1, 0 };
        return v;
    }
};

inline bool operator == (const application::version_t& lhs, const application::version_t& rhs) noexcept {
    return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
}

}} // namespace cppalls::api

# endif // CPPALLS_API_APPLICATION_HPP
