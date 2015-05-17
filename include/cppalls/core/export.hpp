#ifndef CPPALLS_CORE_EXPORT_HPP
#define CPPALLS_CORE_EXPORT_HPP

// Taken from Boost
#if __GNUC__ >= 4
#  if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
     // All Win32 development environments, including 64-bit Windows and MinGW, define
     // _WIN32 or one of its variant spellings. Note that Cygwin is a POSIX environment,
     // so does not define _WIN32 or its variants.
#    ifdef cppalls_core_EXPORTS
#       define CORE_EXPORT __attribute__((__dllexport__))
#    else
#       define CORE_EXPORT __attribute__((__dllimport__))
#    endif
#  else
#    ifdef cppalls_core_EXPORTS
#       define CORE_EXPORT __attribute__((__visibility__("default")))
#    else
#       define CORE_EXPORT
#    endif
#  endif
#else
#    ifdef cppalls_core_EXPORTS
#       define CORE_EXPORT __declspec(dllexport)
#    else
#       define CORE_EXPORT __declspec(dllimport)
#    endif
#endif

# endif // CPPALLS_CORE_EXPORT_HPP
