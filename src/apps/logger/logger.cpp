#include <cppalls/api/logger.hpp>
#include <memory>
#include <boost/log/trivial.hpp>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

#include <boost/log/sources/record_ostream.hpp>

namespace {

class boost_logger : public cppalls::api::logger {
    typedef boost::log::sources::severity_logger_mt< cppalls::api::logger::severity_level > logger_t;

    logger_t logger_;
public:
    boost_logger() {}

    void log(cppalls::api::logger::severity_level lvl, const char* msg) override {
        BOOST_LOG_SEV(logger_, lvl) << msg;
    }

    static std::unique_ptr<boost_logger> create() {
        return std::unique_ptr<boost_logger>(new boost_logger());
    }
   
    ~boost_logger() {}
};

} // namespace anonymous

BOOST_DLL_ALIAS_SECTIONED(boost_logger::create, boost_logger, cppalls)

