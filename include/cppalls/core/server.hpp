#ifndef CPPALLS_CORE_SERVER_HPP
#define CPPALLS_CORE_SERVER_HPP

#include <unordered_map>
#include <typeinfo>

#include <cppalls/api/application.hpp>
#include <cppalls/core/export.hpp>

namespace cppalls {

struct CORE_EXPORT server {
    // this class must not be instantiated
    server() = delete;
    ~server() = delete;

    static void start(const char* path_to_config = "./config.yaml");
    static void stop();
    static void reload();
    static void reload(const char* path_to_config);
    static std::vector<std::string> available_apps();
};

struct CORE_EXPORT app {
    // this class must not be instantiated
    app() = delete;
    ~app() = delete;

    static std::shared_ptr<api::application> get(const std::string& instance_name);
    static std::shared_ptr<api::application> get(const char* instance_name);
    static std::shared_ptr<api::application> construct(const std::string& config);
    static std::shared_ptr<api::application> construct(const char* config);
    static void free(const std::string& config);
    static void free(const char* config);

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
