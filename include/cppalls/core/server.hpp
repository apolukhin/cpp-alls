#ifndef CPPALLS_CORE_SERVER_HPP
#define CPPALLS_CORE_SERVER_HPP

#include <boost/dll.hpp>
#include <boost/utility/string_ref.hpp>
#include <unordered_map>
#include <typeinfo>

#include "cppalls/api/application.hpp"

namespace cppalls {

struct server {
    // this class must not be instantiated
    server() = delete;
    ~server() = delete;


    static void start();
    static void start(int argc, const char * const *argv);

    static void stop();

    static void reload();
    static void reload(int argc, const char * const *argv);

    static std::vector<std::string> available_apps();

    static std::shared_ptr<api::application> get(const std::string& instance_name);
    static std::shared_ptr<api::application> get(const char* instance_name);

    template <class T>
    static std::shared_ptr<T> get(const char* instance_name) {
        std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(
            get(instance_name)
        );

        if (!ret) {
            exception_for_get(typeid(T), instance_name);
        }

        return std::move(ret);
    }


    template <class T>
    static std::shared_ptr<T> get(const std::string& instance_name) {
        return get<T>(instance_name.c_str());
    }

private:
    static void exception_for_get(const std::type_info& type, const char* name);
};

} // namespace cppalls

# endif // CPPALLS_CORE_SERVER_HPP
