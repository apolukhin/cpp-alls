#ifndef CPPALLS_CORE_SLAB_ALLOCATOR_HPP
#define CPPALLS_CORE_SLAB_ALLOCATOR_HPP

#include <new>
#include <type_traits>
#include <algorithm>

namespace cppalls {

// Class to manage the memory to be used for custom allocation.
// It contains blocks of memory which may be returned for allocation
// requests. If the memory blocks are in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class slab_allocator {
public:
    inline slab_allocator() noexcept {
        std::fill(in_use_, in_use_ + storages_count_, static_cast<in_use_t>(0));
    }

    slab_allocator(const slab_allocator&) = delete;
    slab_allocator(slab_allocator&&) = delete;
    slab_allocator& operator=(const slab_allocator&) = delete;
    slab_allocator& operator=(slab_allocator&&) = delete;

    inline void* allocate(std::size_t size) {
        const std::size_t blocks_required = (size / (sizeof(storage_t) - 1)) + 1;

        return blocks_required == 1 ? allocate_single(size) : allocate_multiple(blocks_required, size);
    }

    inline void deallocate(void* pointer) noexcept {
        for (std::size_t i = 0; i < storages_count_; ++i) {
            if (storages_ + i == pointer) {
                const in_use_t to_free = in_use_[i];
                std::fill(in_use_ + i, in_use_ + i + to_free, static_cast<in_use_t>(0));
                return;
            }
        }

        ::operator delete(pointer);
    }

private:
    inline void* allocate_multiple(const std::size_t blocks_required, std::size_t size) {
        // Complexity in worst case: O(storages_count_)
        for (std::size_t i = 0; i <= storages_count_ - blocks_required; ++i) {
            if (!!in_use_[i]) {
                continue;
            }

            // We have a single free block... Searching for free `blocks_required`
            std::size_t free_blocks = 1;
            for (; free_blocks < blocks_required; ++free_blocks) {
                if (!!in_use_[i + free_blocks]) {
                    break;
                }
            }

            if (free_blocks != blocks_required) {
                i += free_blocks;
                continue;
            }

            // Gotcha!
            in_use_[i] = static_cast<in_use_t>(blocks_required);
            std::fill(in_use_ + i + 1, in_use_ + i + blocks_required, (std::numeric_limits<in_use_t>::max)());
            return storages_ + i;
        }

        return ::operator new(size);
    }

    inline void* allocate_single(std::size_t size) {
        for (std::size_t i = 0; i < storages_count_; ++i) {
            if (!in_use_[i]) {
                in_use_[i] = static_cast<in_use_t>(1u);
                return storages_ + i;
            }
        }

        return ::operator new(size);
    }

    static const std::size_t storages_min_size_ = 256u;
    static const std::size_t storages_count_ = 4u;

    // Storage space used for handler-based custom memory allocation.
    typedef typename std::aligned_storage<storages_min_size_>::type storage_t;
    storage_t storages_[storages_count_];

    // Whether the handler-based custom allocation storage has been used.
    typedef unsigned char in_use_t;
    static_assert(storages_count_ < 255 - 1, "`in_use_t` can not hold storages_count_");
    in_use_t in_use_[storages_count_];
};

} // namespace cppalls

#endif // CPPALLS_CORE_SLAB_ALLOCATOR_HPP

