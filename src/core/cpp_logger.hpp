#ifndef CPPALLS_SRC_CORE_CPP_LOGGER_HPP
#define CPPALLS_SRC_CORE_CPP_LOGGER_HPP

#include <cppalls/api/logger.hpp>
#include <cppalls/core/export.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <deque>

namespace cppalls {

class cpp_logger final: public cppalls::api::logger {
    std::ostream* logger_;
    cppalls::api::logger::severity_level lvl_;

    static const char* severity(cppalls::api::logger::severity_level lvl);
    cppalls::api::logger::severity_level severity(const std::string& sev);

public:
    cpp_logger(std::ostream& logger = std::cerr);
    void start(const YAML::Node& config) override;
    void stop() override;
    void log(cppalls::api::logger::severity_level lvl, const char* msg) override;
    static std::unique_ptr<cppalls::api::application> create();
    ~cpp_logger() override;
};


class delayed_logger final: public cppalls::api::logger {
    std::deque<std::pair<cppalls::api::logger::severity_level, std::string> > records;

public:
    delayed_logger() noexcept;
    void reset(cppalls::api::logger&);
    void start(const YAML::Node& config) override;
    void stop() override;
    void log(cppalls::api::logger::severity_level lvl, const char* msg) override;
    ~delayed_logger();
};

} // namespace cppalls

#endif // CPPALLS_SRC_CORE_CPP_LOGGER_HPP

