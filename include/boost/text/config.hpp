#ifndef BOOST_TEXT_CONFIG_HPP
#define BOOST_TEXT_CONFIG_HPP

#include <boost/config.hpp>


#ifndef BOOST_TEXT_THREAD_UNSAFE
/** By default, rope uses atomic reference counts internally.  If you don't
care about thread safety, and want to remove the performance overhead of
using atomics, #define this macro to a nonzero value. */
#    define BOOST_TEXT_THREAD_UNSAFE 0
#endif

/** When you insert into a rope, the incoming sequence may be inserted as a
    new segment, or if it falls within an existing string-segment, it may be
    inserted into the string object used to represent that segment.  This only
    happens if the incoming sequence will fit within the existing segment's
    capacity, or if the segment is smaller than a certain limit.
    BOOST_TEXT_STRING_INSERT_MAX is that limit. */
#ifndef BOOST_TEXT_STRING_INSERT_MAX
#    define BOOST_TEXT_STRING_INSERT_MAX 4096
#endif

// Nothing before GCC 6 has proper C++14 constexpr support.
#if defined(__GNUC__) && __GNUC__ < 6 && !defined(__clang__)
#    define BOOST_TEXT_CXX14_CONSTEXPR
#    define BOOST_TEXT_NO_CXX14_CONSTEXPR
#elif defined(_MSC_VER) && _MSC_VER <= 1910
#    define BOOST_TEXT_CXX14_CONSTEXPR
#    define BOOST_TEXT_NO_CXX14_CONSTEXPR
#else
#    define BOOST_TEXT_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
#    if defined(BOOST_NO_CXX14_CONSTEXPR)
#        define BOOST_TEXT_NO_CXX14_CONSTEXPR
#    endif
#endif

#endif
