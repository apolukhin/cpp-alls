#include <cppalls/api/logger.hpp>
#include <cppalls/core/export.hpp>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/log/sources/record_ostream.hpp>

namespace cppalls { namespace api {

    inline boost::log::formatting_ostream& operator<<(boost::log::formatting_ostream& os, logger::severity_level lvl) {
        switch (lvl) {
        case logger::TRACE:     return os << "TRACE";
        case logger::DEBUG:     return os << "DEBUG";
        case logger::INFO:      return os << "INFO ";
        case logger::WARNING:   return os << "WARN ";
        case logger::ERROR:     return os << "ERROR";
        case logger::FATAL:     return os << "FATAL";
        }
    }

    template <typename CharT, typename TraitsT>
    std::basic_istream<CharT, TraitsT>& operator>>(std::basic_istream<CharT, TraitsT>& strm, logger::severity_level& lvl) {
        int n;
        strm >> n;
        lvl = static_cast<logger::severity_level>(n);

        return strm;
    }

}}


//BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", cppalls::api::logger::severity_level)

namespace {

class boost_logger_core final: public cppalls::api::application {

    void start_helper(std::stringstream& oss, const YAML::Node& config) {
        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            oss << it->first.as<std::string>() << " = \"" << it->second.as<std::string>() << "\"\n";
        }
    }

    boost_logger_core() {}

public:
    void start(const YAML::Node& config) override {
        boost::log::register_simple_formatter_factory<cppalls::api::logger::severity_level, char>("Severity");
        boost::log::add_common_attributes();

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

        boost::log::init_from_stream(oss);
    }

    void stop() override {
        /*noop*/
    }


    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new boost_logger_core());
    }
   
    ~boost_logger_core() override {}
};

class boost_logger final: public cppalls::api::logger {
    typedef boost::log::sources::severity_logger_mt< cppalls::api::logger::severity_level > logger_t;
    logger_t logger_;

    boost_logger() noexcept {}

public:
    void start(const YAML::Node& config) override {
        if (config["tag"]) {
            logger_.add_attribute(
                "tag",
                boost::log::attributes::constant<std::string>(config["tag"].as<std::string>())
            );
        }
    }

    void stop() override {
        /*noop*/
    }

    void log(cppalls::api::logger::severity_level lvl, const char* msg) override {
        BOOST_LOG_SEV(logger_, lvl) << msg;
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new boost_logger());
    }

    ~boost_logger() override {}
};



} // namespace anonymous

CPPALLS_APPLICATION(boost_logger_core::create, boost_logger_core)
CPPALLS_APPLICATION(boost_logger::create, boost_logger)
