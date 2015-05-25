#ifndef CPPALLS_CORE_DETAIL_SHARED_ALLOCATOR_FRIEND_HPP
#define CPPALLS_CORE_DETAIL_SHARED_ALLOCATOR_FRIEND_HPP

#include <memory>

namespace cppalls { namespace detail {

template <class T>
class shared_allocator_friend : public std::allocator<T> {
    using std::allocator<T>::rebind;
    using std::allocator<T>::allocator;

public:
    template<typename... Args>
    inline void construct(void* p, Args&&... args) noexcept(noexcept( T(std::declval<Args>()...) )) {
        ::new(p) T(std::forward<Args>(args)...);
    }
};


}}

#endif // CPPALLS_CORE_DETAIL_SHARED_ALLOCATOR_FRIEND_HPP
