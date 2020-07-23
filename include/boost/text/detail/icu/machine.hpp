#ifndef MACHINE_H_
#define MACHINE_H_

#include <cstdint>


namespace boost { namespace text { namespace detail { namespace icu {

    using UChar32 = int32_t;
    using UChar = char16_t;
    using UBool = int8_t;

    namespace {
        constexpr UChar32 u_sentinel = -1;
        constexpr int u8_max_length = 4;
        constexpr int u16_max_length = 2;
    }

}}}}

#endif
