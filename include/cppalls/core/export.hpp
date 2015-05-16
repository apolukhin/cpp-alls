#ifndef CPPALLS_CORE_EXPORT_HPP
#define CPPALLS_CORE_EXPORT_HPP

#ifdef cppalls_core_EXPORTS
#   include <boost/config.hpp>
#   define CORE_EXPORT BOOST_SYMBOL_EXPORT
#else
#   define CORE_EXPORT
#endif

# endif // CPPALLS_CORE_EXPORT_HPP
