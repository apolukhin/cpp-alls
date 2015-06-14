#ifndef CPPALLS_CORE_SLAB_ALLOCATOR_REF_HPP
#define CPPALLS_CORE_SLAB_ALLOCATOR_REF_HPP

#include <memory>
#include "slab_allocator.hpp"

namespace cppalls {

template <class T>
class slab_allocator_ref: public std::allocator<T> {
    using std::allocator<T>::rebind;
    using std::allocator<T>::allocator;

    slab_allocator* alloc_; 

public:
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef typename std::allocator<T>::pointer pointer;

    slab_allocator_ref() = delete;
    inline explicit slab_allocator_ref(slab_allocator& alloc) noexcept 
        : alloc_(&alloc)
    {}

    slab_allocator_ref(const allocator_ref&) noexcept = default;
    slab_allocator_ref& operator=(const allocator_ref&) noexcept = default;

    inline pointer allocate(size_type n, std::allocator<void>::const_pointer /*hint*/ = 0){
        return reinterpret_cast<pointer>(alloc_->allocate(n));
    }

    inline void deallocate(pointer p, size_type /*n*/ = 0) noexcept{
        alloc_->deallocate(p);
    }
};


} // namespace cppalls

#endif // CPPALLS_CORE_SLAB_ALLOCATOR_REF_HPP
