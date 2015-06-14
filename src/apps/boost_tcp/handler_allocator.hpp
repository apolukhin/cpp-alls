#ifndef NETWORK_HANDLER_ALLOCATOR_HPP
#define NETWORK_HANDLER_ALLOCATOR_HPP

#include <new>
#include <type_traits>
#include <algorithm>
#include <cppalls/core/slab_allocator.hpp>

namespace network {

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler {
public:
    template <class H>
    inline custom_alloc_handler(cppalls::slab_allocator& a, H&& h)
        : allocator_(a)
        , handler_(std::forward<H>(h))
    {}

    template <typename... Args>
    inline void operator()(Args&&... args) {
        handler_(std::forward<Args>(args)...);
    }

    inline friend void* asio_handler_allocate(std::size_t size, custom_alloc_handler<Handler>* this_handler) {
        return this_handler->allocator_.allocate(size);
    }

    inline friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/, custom_alloc_handler<Handler>* this_handler) noexcept {
        this_handler->allocator_.deallocate(pointer);
    }

private:
    cppalls::slab_allocator& allocator_;
    Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<typename std::remove_reference<Handler>::type> make_custom_alloc_handler(cppalls::slab_allocator& a, Handler&& h) {
    return custom_alloc_handler<typename std::remove_reference<Handler>::type>(a, std::forward<Handler>(h));
}

} // namespace network

#endif // NETWORK_HANDLER_ALLOCATOR_HPP

