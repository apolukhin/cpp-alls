#ifndef CPPALLS_CORE_STACK_REQUEST_HPP
#define CPPALLS_CORE_STACK_REQUEST_HPP

#include "request.hpp"
#include "detail/boost_small_vector_fwd.hpp"
#include "detail/stack_pimpl.hpp"
#include <cppalls/core/export.hpp>

namespace cppalls {

class CORE_EXPORT stack_request : public request {
    cppalls::detail::stack_pimpl<
        boost::container::small_vector<unsigned char, 128, boost::container::new_allocator<unsigned char> >
    > pimpl_;

    std::size_t data_extracted_;

public:
    stack_request() noexcept;
    void extract_data(unsigned char* data, std::size_t size) override;
    const unsigned char* begin() const noexcept override;
    const unsigned char* end() const noexcept override;
    void clear() noexcept override;


    unsigned char* begin() noexcept;
    unsigned char* end() noexcept;
    void resize(std::size_t size);

    ~stack_request() noexcept override;
};

} // namespace cppalls

#endif // CPPALLS_CORE_STACK_REQUEST_HPP
