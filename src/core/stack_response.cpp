#include <cppalls/core/stack_response.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/utility/addressof.hpp>

namespace cppalls {

stack_response::stack_response() noexcept
    : pimpl_()
{}

void stack_response::put_data(const unsigned char* data, std::size_t size) {
    const auto old_size = pimpl_->size();
    pimpl_->resize(old_size + size);
    std::memcpy(begin() + old_size, data, size);
}

const unsigned char* stack_response::begin() const noexcept {
    return boost::addressof(*pimpl_->begin());
}

const unsigned char* stack_response::end() const noexcept {
    return boost::addressof(*pimpl_->begin()) + pimpl_->size();
}

void stack_response::clear() noexcept {
    pimpl_->clear();
}

unsigned char* stack_response::begin() noexcept {
    return boost::addressof(*pimpl_->begin());
}

unsigned char* stack_response::end() noexcept {
    return boost::addressof(*pimpl_->begin()) + pimpl_->size();
}

void stack_response::resize(std::size_t size) {
    pimpl_->resize(size);
}

stack_response::~stack_response() noexcept {}

} // namespace cppalls

