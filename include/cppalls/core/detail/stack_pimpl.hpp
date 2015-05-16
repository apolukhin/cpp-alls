#ifndef CPPALLS_CORE_STACK_PIMPL_HPP
#define CPPALLS_CORE_STACK_PIMPL_HPP

#include <type_traits>
#include <utility>
#include <new>

namespace cppalls { namespace detail {

template <std::size_t Size>
using size_constant = std::integral_constant<std::size_t, Size>;

template <class T>
struct lazy_size {
    static_assert(
        false && std::is_void<T>::value,
        "\n\n"
        " Specialize lazy_size<T> near the forward declaration of type, deriving lazy_size<T> from size_constant<size of object>\n"
        " Example:\n"
        "      class Foo;\n"
        "      template <> struct lazy_size<Foo> : size_constant<4> {};\n"
        " End of example\n"
        "\n"
    );
};

template <> struct lazy_size<unsigned char> : size_constant<sizeof(unsigned char)> {};
template <> struct lazy_size<char> : size_constant<sizeof(char)> {};

template <> struct lazy_size<int> : size_constant<sizeof(int)> {};
template <> struct lazy_size<unsigned int> : size_constant<sizeof(unsigned int)> {};

template <> struct lazy_size<short> : size_constant<sizeof(short)> {};
template <> struct lazy_size<unsigned short> : size_constant<sizeof(unsigned short)> {};

template <> struct lazy_size<long long> : size_constant<sizeof(long long)> {};
template <> struct lazy_size<unsigned long long> : size_constant<sizeof(unsigned long long)> {};



template <class T, bool Strict = false>
class stack_pimpl {
    typedef typename std::aligned_storage< lazy_size<T>::value >::type storage_t;

    storage_t storage_;

    T* as_held() noexcept {
        return reinterpret_cast<T*>(&storage_);
    }

    const T* as_held() const noexcept {
        return reinterpret_cast<const T*>(&storage_);
    }

public:
    stack_pimpl() noexcept(noexcept( T() )) {
        new (as_held()) T();
    }

    stack_pimpl(stack_pimpl&& v) noexcept(noexcept( T(std::declval<T>()) )) {
        new (as_held()) T(std::move(*v));
    }

    stack_pimpl(const stack_pimpl& v) noexcept(noexcept( T(std::declval<const T&>()) )) {
        new (as_held()) T(*v);
    }

    stack_pimpl& operator=(const stack_pimpl& rhs) noexcept(noexcept( std::declval<T&>() = std::declval<const T&>() )) {
        *as_held() = *rhs;
        return *this;
    }

    stack_pimpl& operator=(stack_pimpl&& rhs) noexcept(noexcept( std::declval<T&>() = std::declval<T>() )) {
        *as_held() = std::move(*rhs);
        return *this;
    }

    template <class... Args>
    explicit stack_pimpl(Args&&... args) noexcept(noexcept( T(std::declval<Args>()...) )) {
        new (as_held()) T(std::forward<Args>(args)...);
    }

    T* operator->() noexcept {
        return as_held();
    }

    const T* operator->() const noexcept {
        return as_held();
    }

    T& operator*() noexcept {
        return *as_held();
    }

    const T& operator*() const noexcept {
        return *as_held();
    }

    ~stack_pimpl() noexcept {
        static_assert(lazy_size<T>::value >= sizeof(T), "incorrect specialization of lazy_size<T>::value: lazy_size<T>::value is less than sizeof(T)");
        static_assert(lazy_size<T>::value == sizeof(T) || !Strict, "incorrect specialization of lazy_size<T>::value: lazy_size<T>::value and sizeof(T) missmatch");
        as_held()->~T();
    }
};

}} // namespace cppalls::detail

#endif // CPPALLS_CORE_STACK_PIMPL_HPP
