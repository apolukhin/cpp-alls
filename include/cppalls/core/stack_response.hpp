#ifndef CPPALLS_CORE_STACK_RESPONSE_HPP
#define CPPALLS_CORE_STACK_RESPONSE_HPP

#include "response.hpp"
#include "detail/boost_small_vector_fwd.hpp"
#include "detail/stack_pimpl.hpp"
#include <cppalls/core/export.hpp>

namespace cppalls {

class CORE_EXPORT stack_response : public response {
protected:
    cppalls::detail::stack_pimpl<
        boost::container::small_vector<unsigned char, 128, boost::container::new_allocator<unsigned char> >
    > pimpl_;

public:
    stack_response() noexcept;
    void put_data(const unsigned char* data, std::size_t size, std::size_t pos) override;

    unsigned char* begin() noexcept;
    unsigned char* end() noexcept;
    const unsigned char* begin() const noexcept;
    const unsigned char* end() const noexcept;
    void clear() noexcept override;


    void resize(std::size_t size);

    ~stack_response() noexcept override;
};

} // namespace cppalls

#endif // CPPALLS_CORE_STACK_RESPONSE_HPP
