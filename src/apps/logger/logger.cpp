#include <cppalls/api/logger.hpp>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/from_stream.hpp>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

#include <boost/log/sources/record_ostream.hpp>

namespace {

class boost_logger final: public cppalls::api::logger {
    typedef boost::log::sources::severity_logger_mt< cppalls::api::logger::severity_level > logger_t;

    logger_t logger_;

    void start_helper(std::stringstream& oss, const YAML::Node& config) {
        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            oss << it->first.as<std::string>() << " = \"" << it->second.as<std::string>() << "\"\n";
        }
    }

public:
    boost_logger() {}

    void start(const YAML::Node& config) override {

        std::stringstream oss;

        if (config["Core"]) {
            oss << "[Core]\n";
            start_helper(oss, config["Core"]);
        }

        YAML::Node sinks = config["Sinks"];
        if (sinks) {
            for (YAML::const_iterator sinks_it = sinks.begin(); sinks_it != sinks.end(); ++sinks_it) {
                oss << "\n[Sinks." << sinks_it->first.as<std::string>() << "]\n";
                start_helper(oss, sinks_it->second);
            }
        }

        //logger_.set_attributes();

        boost::log::init_from_stream(oss);
    }

    void stop() override {
        /*noop*/
    }

    void log(cppalls::api::logger::severity_level lvl, const char* msg) override {
        BOOST_LOG_SEV(logger_, lvl) << msg;
    }

    static std::unique_ptr<boost_logger> create() {
        return std::unique_ptr<boost_logger>(new boost_logger());
    }
   
    ~boost_logger() override {}
};

} // namespace anonymous

BOOST_DLL_ALIAS_SECTIONED(boost_logger::create, boost_logger, cppalls)

