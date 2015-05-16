#include <cppalls/api/io_service.hpp>
#include <cppalls/core/logging.hpp>
#include <cppalls/core/server.hpp>
#include <yaml-cpp/yaml.h>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <atomic>
#include <limits>
#include <boost/exception/all.hpp>

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS

namespace {

class boost_io_service final: public cppalls::api::io_service_provider {
    boost::asio::io_service                         io_service_;
    std::unique_ptr<boost::asio::io_service::work>  work_;
    boost::thread_group                             threads_;
    std::shared_ptr<cppalls::api::logger>           log_;

    typedef unsigned short          threads_count_t;
    std::atomic<threads_count_t>    required_threads_count_;

    boost_io_service() {}

    void run(threads_count_t thread_index) noexcept {
        LINFO(log_) << "Started thread with index = " << thread_index;

        while (required_threads_count_.load(std::memory_order_relaxed) > thread_index) {
            try {
                const bool is_stopped = !io_service_.run_one();
                if (is_stopped) {
                    break;
                }
            } catch (const std::exception& e) {
                LERROR(log_) << "Exception, while processing tasks in boost_io_service:" << e.what();
            } catch (...) {
                LERROR(log_) << "Exception while processing tasks in boost_io_service:" << boost::current_exception_diagnostic_information();
            }
        }
    }

    static threads_count_t get_threads_count(const YAML::Node& config) {
        std::size_t thread_count = config["threads"].as<std::size_t>(0);
        if (!thread_count) {
            thread_count = boost::thread::hardware_concurrency();
        }

        if (!thread_count) {
            thread_count = 2u;
        }

        const threads_count_t res =
            thread_count > (std::numeric_limits<threads_count_t>::max)()
            ? (std::numeric_limits<threads_count_t>::max)()
            : static_cast<threads_count_t>(thread_count);

        return res;
    }

    std::shared_ptr<boost_io_service> shared_from_this() {
        return std::static_pointer_cast<boost_io_service>(application::shared_from_this());
    }

public:
    void start(const YAML::Node& config) override {
        reload(config);
    }

    void reload(const YAML::Node& config) override {
        const auto threads_count_old = required_threads_count_.load();
        const auto threads_count_new = get_threads_count(config);
        if (!work_) {
            work_.reset(new boost::asio::io_service::work(io_service_));
        }

        log_ = cppalls::server::get<cppalls::api::logger>(config["logger"].as<std::string>());
        required_threads_count_.store(threads_count_new, std::memory_order_acq_rel);

        if (threads_count_old < threads_count_new) {
            for (threads_count_t i = threads_count_old; i < threads_count_new; ++i) {
                threads_.create_thread(std::bind(
                    &boost_io_service::run,
                    shared_from_this(),
                    i
                ));
            }
        }
    }

    void stop() override {
        if (log_) {
            LINFO(log_) << "boost_io_service stopping";
        }
        work_.reset();
        required_threads_count_.store(0, std::memory_order_acq_rel);
        io_service_.stop();
        threads_.join_all();
        io_service_.reset();
        log_.reset();
    }

    static std::unique_ptr<cppalls::api::application> create() {
        return std::unique_ptr<cppalls::api::application>(new boost_io_service());
    }

    boost::asio::io_service& io_service() noexcept override {
        return io_service_;
    }

    ~boost_io_service() override {}
};

} // namespace anonymous

BOOST_DLL_ALIAS_SECTIONED(boost_io_service::create, boost_io_service, cppalls)
