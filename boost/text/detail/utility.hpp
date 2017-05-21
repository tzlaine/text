#ifndef BOOST_TEXT_DETAIL_UTILITY_HPP
#define BOOST_TEXT_DETAIL_UTILITY_HPP

#include <cstddef>


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

} } }

#endif
