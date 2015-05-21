#ifndef CPPALLS_CORE_EXPORT_HPP
#define CPPALLS_CORE_EXPORT_HPP

#include <memory>

// Taken from Boost
#if __GNUC__ >= 4
#   if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
     // All Win32 development environments, including 64-bit Windows and MinGW, define
     // _WIN32 or one of its variant spellings. Note that Cygwin is a POSIX environment,
     // so does not define _WIN32 or its variants.
#       define CPPALLS_EXPORT __attribute__((__dllexport__))
#       define CPPALLS_IMPORT __attribute__((__dllimport__))
#   else
#       define CPPALLS_EXPORT __attribute__((__visibility__("default")))
#       define CPPALLS_IMPORT
#   endif
#   define CPPALLS_SECTION __attribute__ ((section ("cppalls")))
#else
#   define CPPALLS_EXPORT __declspec(dllexport)
#   define CPPALLS_IMPORT __declspec(dllimport)
#   define CPPALLS_SECTION __pragma(section("cppalls", read)) __declspec(allocate("cppalls"))
#endif



#ifdef cppalls_core_EXPORTS
#   define CORE_EXPORT CPPALLS_EXPORT
#else
#   define CORE_EXPORT CPPALLS_IMPORT
#endif

namespace cppalls { namespace api {
    class application;
}} // namespace cppalls::api


namespace cppalls { namespace detail {

    struct validate_app_signature {
        typedef char    yes_t;
        struct no_t { char fake1; char fake2; };

        static yes_t test(std::unique_ptr<cppalls::api::application>());
        static no_t test(...);
    };

}} // namespace cppalls::detail


#define CPPALLS_APPLICATION(Function, ExportName)                                               \
    namespace _applications {                                                                   \
        static_assert(                                                                          \
            sizeof( ::cppalls::detail::validate_app_signature::test(Function))                  \
                == sizeof( ::cppalls::detail::validate_app_signature::yes_t),                   \
            "Function that construct applications must have signature std::unique_ptr<cppalls::api::application> Function()"    \
        );                                                                                      \
        extern "C" CPPALLS_EXPORT const void *ExportName;                                       \
        CPPALLS_SECTION                                                                         \
        const void * ExportName = reinterpret_cast<const void*>(reinterpret_cast<intptr_t>(     \
            &Function                                                                           \
        ));                                                                                     \
    } /* namespace _applications */                                                 \
    /**/

# endif // CPPALLS_CORE_EXPORT_HPP
