#ifndef BOOST_TEXT_DETAIL_UTILITY_HPP
#define BOOST_TEXT_DETAIL_UTILITY_HPP

#include <boost/text/config.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>

#include <ostream>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <climits>


namespace boost { namespace text { namespace detail {

#ifdef BOOST_TEXT_NO_CXX14_CONSTEXPR

    inline std::ptrdiff_t strlen (char const * c_str) noexcept
    { return ::strlen(c_str); }

#else

    inline constexpr std::ptrdiff_t strlen (char const * c_str) noexcept
    {
        assert(c_str);
        std::size_t retval = 0;
        while (*c_str) {
            ++retval;
            ++c_str;
        }
        return retval;
    }

#endif

    inline BOOST_TEXT_CXX14_CONSTEXPR char const * strchr (
        char const * first,
        char const * last,
        char c
    ) noexcept {
        while (first != last) {
            if (*first == c)
                return first;
            ++first;
        }
        return last;
    }

    inline BOOST_TEXT_CXX14_CONSTEXPR char const * strrchr (
        char const * first,
        char const * last,
        char c
    ) noexcept {
        char const * it = last;
        while (first != it) {
            if (*--it == c)
                return it;
        }
        return last;
    }

    template <typename T>
    constexpr T min_ (T lhs, T rhs) noexcept
    { return lhs < rhs ? lhs : rhs; }

    template <typename T>
    constexpr T max_ (T lhs, T rhs) noexcept
    { return lhs < rhs ? rhs : lhs; }

#ifdef BOOST_TEXT_NO_CXX14_CONSTEXPR

    inline int compare_impl (
        char const * l_first, char const * l_last,
        char const * r_first, char const * r_last
    ) noexcept {
        auto const iters =
            algorithm::mismatch(l_first, l_last, r_first, r_last);
        if (iters.first == l_last) {
            if (iters.second == r_last)
                return 0;
            else
                return -1;
        } else if (iters.second == r_last) {
            return 1;
        } else if (*iters.first < *iters.second) {
            return -1;
        } else {
            return 1;
        }
    }

#else

    inline constexpr int compare_impl (
        char const * l_first, char const * l_last,
        char const * r_first, char const * r_last
    ) noexcept {
        auto const l_size = l_last - l_first;
        auto const r_size = r_last - r_first;
        assert(l_size <= INT_MAX);
        assert(r_size <= INT_MAX);

        int retval = 0;

        int const size = (int)detail::min_(l_size, r_size);
        if (size != 0) {
            char const * l_it = l_first;
            char const * l_it_end = l_first + size;
            char const * r_it = r_first;
            while (l_it != l_it_end) {
                char const l_c = *l_it;
                char const r_c = *r_it;
                if (l_c < r_c)
                    return -1;
                if (r_c < l_c)
                    return 1;
                ++l_it;
                ++r_it;
            }
        }

        if (retval == 0) {
            if (l_size < r_size) return -1;
            if (l_size == r_size) return 0;
            return 1;
        }

        return retval;
    }

#endif

} } }

#endif
