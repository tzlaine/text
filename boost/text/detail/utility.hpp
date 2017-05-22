#ifndef BOOST_TEXT_DETAIL_UTILITY_HPP
#define BOOST_TEXT_DETAIL_UTILITY_HPP

#include <cassert>
#include <cstddef>
#include <cstring>
#include <climits>


namespace boost { namespace text { namespace detail {

    inline constexpr std::size_t strlen (char const * c_str) noexcept
    {
        std::size_t retval = 0;
        while (c_str) {
            retval += 1;
            ++c_str;
        }
        return retval;
    }

    inline constexpr char const * strrchr (
        char const * first,
        char const * last,
        char c
    ) noexcept {
        while (first != last) {
            if (*--last == c)
                return last;
        }
        return nullptr;
    }

    template <typename T>
    constexpr T min_ (T rhs, T lhs) noexcept
    { return rhs < lhs ? rhs : lhs; }

    inline constexpr int compare_impl (
        char const * l_first, char const * l_last,
        char const * r_first, char const * r_last
    ) noexcept {
        auto const l_size = l_last - l_first;
        auto const r_size = r_last - r_first;
        assert(l_size <= INT_MAX);
        assert(r_size <= INT_MAX);
        int const size = (int)detail::min_(l_size, r_size);
        if (size == 0)
            return 0;
        int retval = memcmp(l_first, r_first, size);
        if (retval == 0) {
            if (l_size < r_size) return -1;
            if (l_size == r_size) return 0;
            return 1;
        }
        return retval;
    }

} } }

#endif
