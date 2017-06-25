#ifndef BOOST_TEXT_CONFIG_HPP
#define BOOST_TEXT_CONFIG_HPP

#include <boost/config.hpp>


// Nothing before GCC 6 has proper C++14 constexpr support.
#if defined(__GNUC__) && __GNUC__ < 6
# define BOOST_TEXT_CXX14_CONSTEXPR
# define BOOST_TEXT_NO_CXX14_CONSTEXPR
#else
# define BOOST_TEXT_CXX14_CONSTEXPR BOOST_CXX14_CONSTEXPR
# if defined(BOOST_NO_CXX14_CONSTEXPR)
#   define BOOST_TEXT_NO_CXX14_CONSTEXPR
# endif
#endif

#endif
