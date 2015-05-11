#include <cppalls/api/logger.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

namespace {

class cpp_logger final: public cppalls::api::logger {
    std::ostream* logger_;
    cppalls::api::logger::severity_level lvl_;

    static const char* severity(cppalls::api::logger::severity_level lvl) {
        switch (lvl) {
        case TRACE:     return "TRACE: ";
        case DEBUG:     return "DEBUG: ";
        case INFO:      return "INFO:  ";
        case WARNING:   return "WARN:  ";
        case ERROR:     return "ERROR: ";
        case FATAL:     return "FATAL: ";
        }
    }

    cppalls::api::logger::severity_level severity(const std::string& sev) {
        if (sev == "TRACE" || sev == "trace") {
            return cppalls::api::logger::TRACE;
        } else if (sev == "DEBUG" || sev == "debug") {
            return cppalls::api::logger::DEBUG;
        } else if (sev == "INFO" || sev == "info") {
            return cppalls::api::logger::INFO;
        } else if (sev == "WARN" || sev == "warn") {
            return cppalls::api::logger::WARNING;
        } else if (sev == "ERROR" || sev == "error") {
            return cppalls::api::logger::ERROR;
        } else if (sev == "DEBUG" || sev == "debug") {
            return cppalls::api::logger::FATAL;
        }


        log(cppalls::api::logger::ERROR, ("Unsupported severity level '" + sev + "'").c_str());
        return cppalls::api::logger::TRACE;
    }

public:
    cpp_logger()
        : logger_(&std::cerr)
        , lvl_(cppalls::api::logger::TRACE)
    {}

    void start(const YAML::Node& config) override {
        const auto sink = config["sink"].as<std::string>("cerr");

        if (sink == "cerr") {
            logger_ = &std::cerr;
        } else if (sink == "cout") {
            logger_ = &std::cout;
        } else {
            std::string error = "Unknown sink type '" + sink + "', using std::cerr as fallback";
            log(cppalls::api::logger::ERROR, error.c_str());
        }

        lvl_ = severity(config["severity"].as<std::string>("trace"));
    }

    void stop() override {
        /*noop*/
    }

    void log(cppalls::api::logger::severity_level lvl, const char* msg) override {
        if (lvl >= lvl_) {
            *logger_ << severity(lvl) << msg << '\n';
        }
    }

    static std::unique_ptr<cpp_logger> create() {
        return std::unique_ptr<cpp_logger>(new cpp_logger());
    }

    ~cpp_logger() override {}
};

} // namespace anonymous

BOOST_DLL_ALIAS_SECTIONED(cpp_logger::create, cpp_logger, cppalls)

