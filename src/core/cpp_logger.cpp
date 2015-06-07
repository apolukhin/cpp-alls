#include "cpp_logger.hpp"

#include <cppalls/api/logger.hpp>
#include <cppalls/core/export.hpp>
#include <yaml-cpp/yaml.h>
#include <utility>

namespace cppalls {

    const char* cpp_logger::severity(cppalls::api::logger::severity_level lvl) {
        switch (lvl) {
        case TRACE:     return "TRACE: ";
        case DEBUG:     return "DEBUG: ";
        case INFO:      return "INFO:  ";
        case WARNING:   return "WARN:  ";
        case ERROR:     return "ERROR: ";
        case FATAL:     return "FATAL: ";
        }
    }

    cppalls::api::logger::severity_level cpp_logger::severity(const std::string& sev) {
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

    cpp_logger::cpp_logger(std::ostream& logger)
        : logger_(&logger)
        , lvl_(cppalls::api::logger::TRACE)
    {}

    void cpp_logger::start(const YAML::Node& config) {
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

    void cpp_logger::stop() {
        /*noop*/
    }

    void cpp_logger::log(cppalls::api::logger::severity_level lvl, const char* msg) {
        if (lvl >= lvl_) {
            *logger_ << severity(lvl) << msg << '\n';
        }
    }

    std::unique_ptr<cppalls::api::application> cpp_logger::create() {
        return std::unique_ptr<cppalls::api::application>(new cpp_logger());
    }

    cpp_logger::~cpp_logger() {}



    delayed_logger::delayed_logger() noexcept {}

    void delayed_logger::reset(api::logger& l) {
        for (auto&& rec : records) {
            l.log(rec.first, rec.second.c_str());
        }

        records.clear();
    }

    void delayed_logger::log(cppalls::api::logger::severity_level lvl, const char* msg) {
        records.push_back(std::make_pair(
            lvl, msg
        ));
    }

    void delayed_logger::start(const YAML::Node& /*config*/) {}
    void delayed_logger::stop() {}

    delayed_logger::~delayed_logger() {
        cpp_logger l;
        reset(l);
    }

} // namespace cppalls

CPPALLS_APPLICATION(cppalls::cpp_logger::create, cpp_logger)

