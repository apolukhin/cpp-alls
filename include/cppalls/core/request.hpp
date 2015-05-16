#ifndef CPPALLS_CORE_REQUEST_HPP
#define CPPALLS_CORE_REQUEST_HPP

#include <cppalls/core/export.hpp>
#include <string>
#include <vector>

namespace cppalls {

class CORE_EXPORT request {
public:
    request() = default;
    request(const request&) = delete;
    request(request&&) = delete;
    request& operator=(const request&) = delete;
    request& operator=(request&&) = delete;

    request& operator>> (bool& val);
    request& operator>> (char& val);
    request& operator>> (unsigned char& val);
    request& operator>> (short& val);
    request& operator>> (unsigned short& val);
    request& operator>> (int& val);
    request& operator>> (unsigned int& val);
    request& operator>> (long& val);
    request& operator>> (unsigned long& val);
    request& operator>> (long long& val);
    request& operator>> (unsigned long long& val);
    request& operator>> (float& val);
    request& operator>> (double& val);
    request& operator>> (long double& val);
    request& operator>> (std::string& val);

    template <class T>
    request& operator>> (std::vector<T>& val) {
        typedef typename std::vector<T>::const_iterator const_iterator;
        unsigned int size;
        (*this) >> size;

        val.clear();
        val.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            T v;
            (*this) >> v;
            val.push_back(std::move(v));
        }

        return *this;
    }

    virtual void extract_data(unsigned char* data, std::size_t size) = 0;
    virtual const unsigned char* begin() const noexcept = 0;
    virtual const unsigned char* end() const noexcept = 0;
    virtual void clear() noexcept = 0;
    virtual ~request() noexcept {}
};

} // namespace cppalls

#endif // CPPALLS_CORE_REQUEST_HPP
