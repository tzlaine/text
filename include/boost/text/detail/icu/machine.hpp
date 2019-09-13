#ifndef MACHINE_H_
#define MACHINE_H_

#include <cstdint>


namespace boost { namespace text { namespace detail { namespace icu {

    using UChar32 = int32_t;
    using UChar = char16_t;
    using UBool = int8_t;

    namespace {
        constexpr bool TRUE = true;
        constexpr bool FALSE = false;
        constexpr UChar32 U_SENTINEL = -1;
        constexpr int U8_MAX_LENGTH = 4;
        constexpr int U16_MAX_LENGTH = 2;
    }

}}}}

#endif
