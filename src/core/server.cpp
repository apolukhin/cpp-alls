#include "cppalls/server.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>

using namespace boost;

namespace cppalls {

namespace {
    static const char config_default_path[] = "config.yaml";
    static const filesystem::path default_apps_dir = "../src/apps/logger/";

    struct ptr_holding_application_deleter {
        std::shared_ptr<dll::shared_library> lib_;

        explicit ptr_holding_application_deleter(dll::shared_library&& lib)
            : lib_(std::make_shared<dll::shared_library>(std::move(lib)))
        {}

        inline void operator()(api::application* p) const noexcept {
            delete p;
        }
    };
}


void server::init(int argc, const char * const *argv) {
    namespace po = boost::program_options;

    std::string config_path;

    po::options_description desc("Server basic options");
    desc.add_options()
       ("help", "produce help message")
       ("config",
        po::value<std::string>(&config_path)
            ->default_value(config_default_path),
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


    filesystem::directory_iterator end;
    system::error_code error;
    filesystem::directory_iterator it(default_apps_dir);

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
            std::cerr << "Failed opening plugin `" << *it << "`: " << e.what() << '\n';
        }
    }



    YAML::Node config = YAML::LoadFile(config_path);

    for (auto&& app_node : config["applications"]) {
        const auto app_name = app_node["name"].as<std::string>();
        dll::shared_library lib(app_to_path_[app_name]);
        auto app = lib.get_alias<api::application::constructor_t>(app_name)();
        std::shared_ptr<api::application> shared_app(
            app.release(),
            ptr_holding_application_deleter(std::move(lib))
        );

        instances_.insert(std::make_pair(
            app_node["instance"].as<std::string>(),
            std::move(shared_app)
        ));
    }
}



server $;

} // namespace cppalls

