#include "cppalls/core/server.hpp"
#include "cppalls/core/exceptions.hpp"
#include "cppalls/core/logging.hpp"
#include "cppalls/api/logger.hpp"

#include "cpp_logger.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/program_options.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <exception>
#include <boost/type_index.hpp>
#include <boost/dll.hpp>
#include <boost/utility/string_ref.hpp>

using namespace boost;

namespace cppalls {

namespace {

    struct ptr_holding_application_deleter {
        std::shared_ptr<dll::shared_library> lib_;

        explicit ptr_holding_application_deleter(dll::shared_library&& lib)
            : lib_(std::make_shared<dll::shared_library>(std::move(lib)))
        {}

        inline void operator()(api::application* p) const noexcept {
            delete p;
        }
    };

    static inline std::string get_config_location_or_die_helping(int argc, const char * const *argv) {
        static const char default_config_path[] = "config.yaml";
        namespace po = boost::program_options;
        std::string config_path;

        po::options_description desc("Server basic options");
        desc.add_options()
           ("help", "produce help message")
           ("config",
            po::value<std::string>(&config_path)
                ->default_value(default_config_path),
                "set name and path to the configuration file")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
           std::cout << desc << "\n\n"
                << "All other options must be set up in a YAML config file.";
           exit(1);
        }

        return std::move(config_path);
    }

    class server_impl_t {
        typedef std::unordered_map<std::string, boost::filesystem::path>            app_to_path_t;
        typedef std::unordered_map<std::string, std::shared_ptr<api::application> > instances_t;

        app_to_path_t                   app_to_path_;
        instances_t                     instances_;
        std::string                     config_path_;
        std::shared_ptr<api::logger>    log_;


        void find_apps_in_entry(const std::string& entry, app_to_path_t& app_to_path) const {
            filesystem::path p(entry);
            if (p.is_relative()) {
                p =  boost::filesystem::path(config_path_).parent_path() / p;
            }

            const auto search_in_binary = [&app_to_path, this](const filesystem::path& p) {
                try {
                    dll::library_info info(p);
                    auto apps = info.symbols("cppalls");
                    for (auto& app: apps) {
                        app_to_path.emplace(std::move(app), p);
                    }
                } catch (const std::exception& e) {
                    LINFO(log_) << "Failed opening plugin `" << p << "`: " << e.what();
                }
            };

            system::error_code error;
            for (filesystem::directory_iterator it(p), end; it != end; ++it) {
                if (filesystem::is_directory(*it, error) || error) {
                    continue;
                }

                search_in_binary(*it);
            }

            if (filesystem::is_regular_file(p, error) && !error) {
                search_in_binary(p);
            }
        }

        void fill_apps_to_path(const YAML::Node& config, app_to_path_t& app_to_path) const {
            static const std::string default_apps_dir = "./apps/";

            dll::library_info info(dll::this_line_location());
            auto apps = info.symbols("cppalls");
            for (auto& app: apps) {
                app_to_path.emplace(std::move(app), dll::this_line_location());
            }

            YAML::Node bins = config["core"]["binaries"];
            if (bins.IsScalar()) {
                find_apps_in_entry(bins.as<std::string>(), app_to_path);
            } else if (bins.IsSequence()) {
                for (auto&& v : bins) {
                    find_apps_in_entry(v.as<std::string>(), app_to_path);
                }
            } else if (!bins) {
                find_apps_in_entry(default_apps_dir, app_to_path);
            } else {
                boost::throw_exception(error_runtime(
                    "Invalid configuration for `core: binaries:` section. Binaries must be a sinle path to binaries or a sequence of paths [path1, path2]"
                ));
            }
        }

        static std::shared_ptr<api::application> start_instance(const YAML::Node& app_node, const app_to_path_t& app_to_path, api::logger& log, instances_t& instances) {
            const auto app_type = app_node["type"].as<std::string>();
            const auto it = app_to_path.find(app_type);
            if (it == app_to_path.cend()) {
                boost::throw_exception(error_runtime(
                    "Failed to find binary that contains application of type '" + app_type + "'"
                ));
            }
            dll::shared_library lib(it->second);
            auto app = lib.get_alias<api::application::constructor_t>(app_type)();
            std::shared_ptr<api::application> shared_app(
                app.release(),
                ptr_holding_application_deleter(std::move(lib))
            );

            LDEBUG(log) << "Starting '" << app_type << "' with name '" << app_node["instance-name"].as<std::string>(app_type) << "'\n";
            shared_app->start(app_node);

            auto instance_name = app_node["instance-name"].as<std::string>(app_type);
            if (instances.count(instance_name)) {
                boost::throw_exception(error_runtime(
                    "Instance with name '" + instance_name + "' already exist."
                ));
            }

            instances.insert(std::make_pair(
                std::move(instance_name),
                shared_app
            ));

            return std::move(shared_app);
        }

        void init_logger(const YAML::Node& app_node) {
            auto old_logger = log_;


            if (app_node["core"]["logger"]) {
                log_ = app::get<api::logger>(app_node["core"]["logger"].as<std::string>());
            } else {
                log_ = std::make_shared<cpp_logger>();
            }

            if (auto p = std::dynamic_pointer_cast<delayed_logger>(old_logger)) {
                p->reset(*log_);
            }
        }

        void read_single_config_file(const std::string& file_or_path, YAML::Node& config_to_patch) const {
            boost::filesystem::path p(file_or_path);
            if (p.is_relative()) {
                p =  boost::filesystem::path(config_path_).parent_path() / p;
            }

            boost::container::small_vector<std::string, 64> configs;
            system::error_code error;
            if (filesystem::is_directory(p, error) && !error) {
                for (filesystem::directory_iterator it(p), end; it != end; ++it) {
                    if (filesystem::is_directory(*it, error) || error) {
                        continue;
                    }

                    configs.emplace_back(it->path().string());
                }
            }

            if (filesystem::is_regular_file(p, error) && !error) {
                configs.emplace_back(p.string());
            }

            std::sort(configs.begin(), configs.end());

            for (auto&& conf_path : configs) {
                try {
                    auto another_conf = YAML::LoadFile(conf_path);

                    for (auto&& app : another_conf["applications"]) {
                        config_to_patch["applications"].push_back(std::move(app));
                    }
                } catch (const std::exception& e) {
                    boost::throw_exception(error_runtime(
                        "Error while loading dependent configuration file '" + conf_path +  "' from directory '" + dll::program_location().parent_path().string()
                            + "':\n    " + e.what()
                    ));
                }
            }
        }

        YAML::Node read_config() const {
            YAML::Node config;

            try {
                config = YAML::LoadFile(config_path_);
            } catch (const std::exception& e) {
                boost::throw_exception(error_runtime(
                    "Error while loading configuration file '" + config_path_ +  "' from directory '" + dll::program_location().parent_path().string()
                        + "':\n    " + e.what()
                ));
            }

            if (!config["core"]["configs"]) {
                return std::move(config);
            }

            if (config["core"]["configs"].IsSequence()) {
                for (auto&& conf : config["core"]["configs"]) {
                    read_single_config_file(conf.as<std::string>(), config);
                }
            } else {
                read_single_config_file(config["core"]["configs"].as<std::string>(), config);
            }

            return std::move(config);
        }

    public:
        server_impl_t()
            : config_path_()
            , log_(std::make_shared<delayed_logger>())
        {}

        std::shared_ptr<api::application> instance_construct(const char* config) {
            return start_instance(YAML::Load(config), app_to_path_, *log_, instances_);
        }

        void instance_free(const char* instance_name) {
            instances_.erase(instance_name);
        }

        void start() {
            const YAML::Node config = read_config();

            fill_apps_to_path(config, app_to_path_);

            for (auto&& app_node : config["applications"]) {
                start_instance(app_node, app_to_path_, *log_, instances_);
            }

            init_logger(config);
        }

        void start(const char* path_to_config) {
            config_path_ = path_to_config;
            start();
        }

        std::vector<std::string> available_apps() {
            std::vector<std::string> res;
            res.reserve(app_to_path_.size());
            for (auto& app: app_to_path_) {
                res.push_back(app.first);
            }

            return std::move(res);
        }

        std::shared_ptr<api::application> get(const char* instance_name) {
            auto it = instances_.find(instance_name);
            if (it == instances_.end()) {
                boost::throw_exception(error_runtime(
                    "Faild to get() application with instance-name = '" + std::string(instance_name) + "'"
                ));
            }

            return it->second;
        }

        void stop() {
            for (auto&& inst : instances_) {
                inst.second->stop();
            }
            instances_.clear();
            app_to_path_.clear();
        }

        void reload() {
            const YAML::Node config = read_config();

            app_to_path_t new_app_to_path;
            fill_apps_to_path(config, new_app_to_path);
            if (!app_to_path_.empty() && new_app_to_path != app_to_path_) {
                boost::throw_exception(error_runtime(
                    "Currently server does not support reload() on changed application binaries"
                ));
            }

            instances_t new_instances;
            for (auto&& app_node : config["applications"]) {

                auto instance = (
                    app_node["instance-name"]
                    ? app_node["instance-name"].as<std::string>()
                    : app_node["type"].as<std::string>()
                );
                auto instance_it = instances_.find(instance);
                if (instance_it == instances_.cend()) {
                    start_instance(app_node, new_app_to_path, *log_, new_instances);
                    instances_.insert(*new_instances.find(instance));
                } else {
                    LDEBUG(log_) << "Reloading application instance '" << instance << "'\n";
                    instance_it->second->reload(app_node);
                    new_instances.insert(*instance_it);
                }
            }

            for (auto&& inst : instances_) {
                inst.second->stop();
            }

            instances_.swap(new_instances);
            app_to_path_.swap(new_app_to_path);
            init_logger(config);

            LDEBUG(log_) << "Resulting confilg file is following:\n" << config;
        }

        void reload(const char* path_to_config) {
            config_path_ = path_to_config;
            reload();
        }

        ~server_impl_t() {
            stop();
        }
    };


    server_impl_t server_impl;
}

void server::start(const char* path_to_config) {
    server_impl.start(path_to_config);
}

void server::stop() {
    server_impl.stop();
}

void server::reload() {
    server_impl.reload();
}

void server::reload(const char* path_to_config) {
    server_impl.reload(path_to_config);
}

std::vector<std::string> server::available_apps() {
    return server_impl.available_apps();
}


std::shared_ptr<api::application> app::get(const char* instance_name) {
    return server_impl.get(instance_name);
}

std::shared_ptr<api::application> app::get(const std::string& instance_name) {
    return server_impl.get(instance_name.c_str());
}

std::shared_ptr<api::application> app::construct(const char* config) {
    return server_impl.instance_construct(config);
}

std::shared_ptr<api::application> app::construct(const std::string& config) {
    return server_impl.instance_construct(config.c_str());
}

void app::free(const char* config) {
    server_impl.instance_free(config);
}

void app::free(const std::string& config) {
    server_impl.instance_free(config.c_str());
}

void app::exception_for_get(const std::type_info& type, const char* name) {
    boost::throw_exception(error_runtime(
        "Failed to call app::get< " + boost::typeindex::type_index(type).pretty_name() + " >(\"" + name + "\")"
    ));
}

} // namespace cppalls
