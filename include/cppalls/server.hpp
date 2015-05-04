#ifndef CPPALLS_SERVER_HPP
#define CPPALLS_SERVER_HPP

#include <boost/dll.hpp>
#include <boost/utility/string_ref.hpp>
#include <unordered_map>
#include "api/application.hpp"



namespace cppalls {

class server {
    std::unordered_map<std::string, boost::filesystem::path>            app_to_path_;
    std::unordered_map<std::string, std::shared_ptr<api::application> > instances_;

public:
    server() = default;

    server(server&&) = delete;
    server(const server&) = delete;
    server& operator=(server&&) = delete;
    server& operator=(const server&) = delete;

    void init(int argc, const char * const *argv);

    std::vector<std::string> list_apps() const {
        std::vector<std::string> res;
        res.reserve(app_to_path_.size());
        for (auto& app: app_to_path_) {
            res.push_back(app.first);
        }

        return std::move(res);
    }

    class streamer_helper {
        std::shared_ptr<api::application> app_;
    public:

        explicit streamer_helper(std::shared_ptr<api::application> app) noexcept
            : app_(std::move(app))
        {}

        template <class T>
        std::shared_ptr<T> as() const {
            return std::dynamic_pointer_cast<T>(app_);
        }

        template <class T>
        const streamer_helper& operator >> (std::shared_ptr<T>& p) const {
            p = as<T>();
            return *this;
        }
    };


    streamer_helper operator[](const std::string& instance_name) const {
        return streamer_helper(instances_.at(instance_name));
    }
};

extern server $;

} // namespace cppalls

# endif // CPPALLS_ASERVER_HPP
