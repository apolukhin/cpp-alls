#ifndef CPPALLS_CORE_RESPONSE_HPP
#define CPPALLS_CORE_RESPONSE_HPP

#include <cppalls/core/export.hpp>
#include <string>
#include <vector>

namespace cppalls {

class CORE_EXPORT response {
public:
    static const std::size_t npos = static_cast<std::size_t>(-1);

    response() = default;
    response(const response&) = delete;
    response(response&&) = delete;
    response& operator=(const response&) = delete;
    response& operator=(response&&) = delete;

    response& operator<< (bool val);
    response& operator<< (short val);
    response& operator<< (unsigned short val);
    response& operator<< (int val);
    response& operator<< (unsigned int val);
    response& operator<< (long val);
    response& operator<< (unsigned long val);
    response& operator<< (long long val);
    response& operator<< (unsigned long long val);
    response& operator<< (float val);
    response& operator<< (double val);
    response& operator<< (long double val);
    response& operator<< (const std::string& val);

    template <class T>
    response& operator<< (const std::vector<T>& val) {
        typedef typename std::vector<T>::const_iterator const_iterator;
        const unsigned int size = val.size();
        (*this) << size;

        const const_iterator end = val.cend();
        for (const_iterator it = val.cbegin(); it != end; ++it) {
            (*this) << *it;
        }

        return *this;
    }

    virtual void put_data(const unsigned char*, std::size_t size, std::size_t pos = npos) = 0;
    virtual void clear() noexcept = 0;
    virtual ~response() {}
};

} // namespace cppalls

#endif // CPPALLS_CORE_RESPONSE_HPP
