#ifndef BOOST_TEXT_CONFIG_HPP
#define BOOST_TEXT_CONFIG_HPP

#include <boost/config.hpp>


#ifndef BOOST_TEXT_THREAD_UNSAFE
/** By default, rope uses atomic reference counts internally.  If you don't
    care about thread safety, and want to remove the performance overhead of
    using atomics, #define this macro to a nonzero value. */
#define BOOST_TEXT_THREAD_UNSAFE 0
#endif

// Nothing before GCC 6 has proper C++14 constexpr support.
#if defined(__GNUC__) && __GNUC__ < 6 && !defined(__clang__)
# define BOOST_TEXT_CXX14_CONSTEXPR
# define BOOST_TEXT_NO_CXX14_CONSTEXPR
#else
# define BOOST_TEXT_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
# if defined(BOOST_NO_CXX14_CONSTEXPR)
#   define BOOST_TEXT_NO_CXX14_CONSTEXPR
# endif
#endif

#endif
