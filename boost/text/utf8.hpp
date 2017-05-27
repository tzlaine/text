#ifndef BOOST_TEXT_UTF8_HPP
#define BOOST_TEXT_UTF8_HPP


namespace boost { namespace text { namespace utf8 {

    struct unchecked_t {};

    namespace {

        constexpr unchecked_t unchecked;

    }

    constexpr int code_point_bytes (char first) noexcept
    {
        if ((first & 0b10000000) == 0)
            return 1;
        if ((first & 0b11100000) == 0b11000000)
            return 2;
        if ((first & 0b11110000) == 0b11100000)
            return 3;
        if ((first & 0b11111000) == 0b11110000)
            return 4;
        return -1;
    }

    constexpr bool continuation (char first) noexcept
    { return (first & 0xc0) == 0x80; }

    constexpr bool encoded (char const * first, char const * last) noexcept
    {
        while (first != last) {
            int const cp_bytes = code_point_bytes(*first);
            if (cp_bytes == -1)
                return false;

            ++first;

            for (int i = 1; i < cp_bytes; ++i, ++first) {
                if (first == last || !continuation(*first))
                    return false;
            }
        }

        return true;
    }

    constexpr bool starts_encoded (char const * first, char const * last) noexcept
    {
        int const cp_bytes = code_point_bytes(*first);
        if (cp_bytes == -1)
            return false;

        ++first;

        for (int i = 1; i < cp_bytes; ++i, ++first) {
            if (first == last || !continuation(*first))
                return false;
        }

        return true;
    }

} } }

#endif
