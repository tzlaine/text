#ifndef BOOST_TEXT_CONFIG_HPP
#define BOOST_TEXT_CONFIG_HPP

#include <boost/config.hpp>


#if defined(__GNUC__)
# if 6 <= __GNUC__
#  define BOOST_TEXT_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
# else // Nothing before GCC 6 has proper C++14 constexpr support.
#  define BOOST_TEXT_CXX14_CONSTEXPR
#  define BOOST_TEXT_NO_CXX14_CONSTEXPR
# endif
#else
# define BOOST_TEXT_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
# if defined(BOOST_NO_CXX14_CONSTEXPR)
#   define BOOST_TEXT_NO_CXX14_CONSTEXPR
# endif
#endif

#endif
