#ifndef CPPALLS_API_APPLICATION_HPP
#define CPPALLS_API_APPLICATION_HPP

#include <memory>
#include <yaml-cpp/node/node.h>

namespace cppalls { namespace api {

class application: public std::enable_shared_from_this<application> {
public:
    typedef std::unique_ptr<application>(constructor_t)();

    application() = default;
    application(const application&) = delete;
    application(application&&) = delete;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;

    virtual ~application() noexcept {}

    struct version_t {
        unsigned short major;
        unsigned short minor;
        unsigned int patch;
    };

    virtual version_t version() const noexcept {
        const version_t v = { 0, 1, 0 };
        return v;
    }

    virtual void start(const YAML::Node& config) = 0;
    virtual void stop() = 0;
    virtual void reload(const YAML::Node& config) {
        stop();
        start(config);
    }
};

inline bool operator == (const application::version_t& lhs, const application::version_t& rhs) noexcept {
    return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
}

}} // namespace cppalls::api

# endif // CPPALLS_API_APPLICATION_HPP
