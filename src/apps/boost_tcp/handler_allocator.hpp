#ifndef NETWORK_HANDLER_ALLOCATOR_HPP
#define NETWORK_HANDLER_ALLOCATOR_HPP

#include <new>
#include <type_traits>
#include <algorithm>


namespace network {

// Class to manage the memory to be used for handler-based custom allocation.
// It contains block s of memory which may be returned for allocation
// requests. If the memory blocks are in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_allocator {
public:
    inline handler_allocator() noexcept {
        std::fill(in_use_, in_use_ + storages_count_, false);
    }

    handler_allocator(const handler_allocator&) = delete;
    handler_allocator& operator=(const handler_allocator&) = delete;

    inline void* allocate(std::size_t size) {
        if (size <= sizeof(storage_t)) {
            const auto end = in_use_ + storages_count_;
            const auto it = std::find(in_use_, end, false);
            if (it != end) {
                *it = true;
                return storages_ + (it - in_use_);
            }
        }

        return ::operator new(size);
    }

    inline void deallocate(void* pointer) noexcept {
        for (std::size_t i = 0; i < storages_count_; ++i) {
            if (storages_ + i == pointer) {
                in_use_[i] = false;
                return;
            }
        }

        ::operator delete(pointer);
    }

private:
    static const std::size_t storages_min_size_ = 512u;
    static const std::size_t storages_count_ = 2u;

    // Storage space used for handler-based custom memory allocation.
    typedef typename std::aligned_storage<storages_min_size_>::type storage_t;
    storage_t storages_[storages_count_];

    // Whether the handler-based custom allocation storage has been used.
    bool in_use_[storages_count_];
};


// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler {
public:
    template <class H>
    inline custom_alloc_handler(handler_allocator& a, H&& h)
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
    handler_allocator& allocator_;
    Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<typename std::remove_reference<Handler>::type> make_custom_alloc_handler(handler_allocator& a, Handler&& h) {
    return custom_alloc_handler<typename std::remove_reference<Handler>::type>(a, std::forward<Handler>(h));
}

} // namespace network

#endif // NETWORK_HANDLER_ALLOCATOR_HPP

