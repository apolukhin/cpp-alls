#include "cppalls/core/server.hpp"
#include "cppalls/core/exceptions.hpp"
#include "cppalls/api/logger.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <exception>
#include <boost/type_index.hpp>

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

    class server_impl_t {
        std::unordered_map<std::string, boost::filesystem::path>            app_to_path_;
        std::unordered_map<std::string, std::shared_ptr<api::application> > instances_;

        static inline std::string get_config_location_or_die_helping(int argc, const char * const *argv) {
            namespace po = boost::program_options;
            std::string config_path;

            po::options_description desc("Server basic options");
            desc.add_options()
               ("help", "produce help message")
               ("config",
                po::value<std::string>(&config_path)
                    ->default_value("config.yaml"),
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

        void fill_apps_to_path(const YAML::Node& config, std::ostringstream& oss) {
            static const std::string default_apps_dir = "./apps/";

            filesystem::directory_iterator end;
            system::error_code error;
            filesystem::directory_iterator it(
                config["core"]["apps-path"].as<std::string>(default_apps_dir)
            );


            for (; it != end; ++it) {
                if (filesystem::is_directory(*it, error) || error) {
                    continue;
                }

                try {
                    dll::library_info info(*it);
                    auto apps = info.symbols("cppalls");
                    for (auto& app: apps) {
                        app_to_path_.emplace(std::move(app), *it);
                    }
                } catch (const std::exception& e) {
                    oss << "\nFailed opening plugin `" << *it << "`: " << e.what();
                }
            }
        }

    public:

        void start(int argc, const char * const *argv) {
            const auto config_path = get_config_location_or_die_helping(argc, argv);
            YAML::Node config = YAML::LoadFile(config_path);

            std::ostringstream oss;
            fill_apps_to_path(config, oss);

            for (auto&& app_node : config["applications"]) {
                const auto app_name = app_node["type"].as<std::string>();
                dll::shared_library lib(app_to_path_[app_name]);
                auto app = lib.get_alias<api::application::constructor_t>(app_name)();
                std::shared_ptr<api::application> shared_app(
                    app.release(),
                    ptr_holding_application_deleter(std::move(lib))
                );

                shared_app->start(app_node["params"]);

                instances_.insert(std::make_pair(
                    app_node["instance-name"].as<std::string>(),
                    std::move(shared_app)
                ));
            }

            if (oss.str().empty()) {
                return;
            }

            oss << '\n';
            if (config["core"]["logger"]) {
                auto log = server::get<api::logger>(config["core"]["logger"].as<std::string>());
                log->log(api::logger::ERROR, oss.str().c_str());
            } else {
                std::cerr << oss;
            }
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

        ~server_impl_t() {
            stop();
        }
    };


    server_impl_t server_impl;
}


void server::start(int argc, const char * const *argv) {
    server_impl.start(argc, argv);
}

void server::stop() {
    server_impl.stop();
}

std::vector<std::string> server::available_apps() {
    return server_impl.available_apps();
}


std::shared_ptr<api::application> server::get(const char* instance_name) {
    return server_impl.get(instance_name);
}

std::shared_ptr<api::application> server::get(const std::string& instance_name) {
    return get(instance_name.c_str());
}

void server::exception_for_get(const std::type_info& type, const char* name) {
    boost::throw_exception(error_runtime(
        "Failed to call server::get< " + boost::typeindex::type_index(type).pretty_name() + " >(\"" + name + "\")"
    ));
}

} // namespace cppalls
