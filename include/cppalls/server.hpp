#ifndef CPPALLS_SERVER_HPP
#define CPPALLS_SERVER_HPP

#include <boost/dll.hpp>
#include <boost/utility/string_ref.hpp>
#include <unordered_map>
#include "api/application.hpp"



namespace cppalls {

struct server {
    // this class must not be instantiated
    server() = delete;
    ~server() = delete;

    static void init(int argc, const char * const *argv);

    static std::vector<std::string> list_apps();

    static std::shared_ptr<api::application> get(const std::string& instance_name);
    static std::shared_ptr<api::application> get(const char* instance_name);

    template <class T>
    static std::shared_ptr<T> get(const std::string& instance_name) {
        return std::dynamic_pointer_cast<T>(
            get(instance_name)
        );
    }

    template <class T>
    static std::shared_ptr<T> get(const char* instance_name) {
        return std::dynamic_pointer_cast<T>(
            get(instance_name)
        );
    }
};

} // namespace cppalls

# endif // CPPALLS_ASERVER_HPP
