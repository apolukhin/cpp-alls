#ifndef CPPALLS_DETAIL_BOOST_SMALL_VECTOR_FWD_HPP
#define CPPALLS_DETAIL_BOOST_SMALL_VECTOR_FWD_HPP

#include <cstdint>
#include "stack_pimpl.hpp"

namespace boost { namespace container {
    template <class T, std::size_t N, class Allocator>
    class small_vector;

    template <class T>
    class new_allocator;
}}

namespace cppalls { namespace detail {
    template <class T, std::size_t N, class Allocator>
    struct lazy_size<boost::container::small_vector<T, N, Allocator> > : size_constant<
        lazy_size<T>::value * N + 32 // 32 is for small_vector internals
    > {};
}}

#endif // CPPALLS_DETAIL_BOOST_SMALL_VECTOR_FWD_HPP
