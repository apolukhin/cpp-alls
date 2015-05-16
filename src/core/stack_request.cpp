#include <cppalls/core/stack_request.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/utility/addressof.hpp>

namespace cppalls { 

stack_request::stack_request() noexcept
    : pimpl_()
    , data_extracted_(0)
{}

void stack_request::extract_data(unsigned char* data, std::size_t size) {
    if (pimpl_->size() < size + data_extracted_) {
        //boost::throw_exception();
    }

    std::memcpy(data, begin(), size);
    data_extracted_ += size;
}

const unsigned char* stack_request::begin() const noexcept {
    return boost::addressof(*pimpl_->begin()) + data_extracted_;
}

const unsigned char* stack_request::end() const noexcept {
    return boost::addressof(*pimpl_->begin()) + pimpl_->size();
}

void stack_request::clear() noexcept {
    pimpl_->clear();
    data_extracted_ = 0;
}

unsigned char* stack_request::begin() noexcept {
    return boost::addressof(*pimpl_->begin()) + data_extracted_;
}

unsigned char* stack_request::end() noexcept {
    return boost::addressof(*pimpl_->begin()) + pimpl_->size();
}

void stack_request::resize(std::size_t size) {
    pimpl_->resize(size);
}

stack_request::~stack_request() noexcept {}

} // namespace cppalls

