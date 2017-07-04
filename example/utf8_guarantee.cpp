#include <boost/text/text.hpp>


int main ()
{

try {
//[ ctor_encoding_check
    // Two Unicode code points.
    uint32_t const utf32[2] = {0x004d, 0x10302};
    char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), char(0x82)};

    boost::text::text_view tv1(utf8, 5); // Ok.
    boost::text::text_view tv2(utf8, 4); // Error! The second code point got chopped.
//]
    (void)utf32;
} catch (...) {}

{
//[ opt_in_skipped_check
    // Two Unicode code points.
    uint32_t const utf32[2] = {0x004d, 0x10302};
    char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), char(0x82)};

    boost::text::text_view tv1(utf8, 5); // Ok.
    boost::text::text_view tv2(utf8, 4, boost::text::utf8::unchecked); // Ok, check skipped.
//]
    (void)utf32;
}

{
//[ check_endpoints
    // Three Unicode code points.
    uint32_t const utf32[3] = {0x004d, 0x10302, 0x004d};
    // The middle one is broken.
    char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), /*char(0x82), */0x4d};

    boost::text::text_view tv(utf8, 5); // Ok. Breakage is not at the ends.
//]
    (void)utf32;
}

try {
//[ opt_in_full_check
    // Three Unicode code points.
    uint32_t const utf32[3] = {0x004d, 0x10302, 0x004d};
    // The middle one is broken.
    char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), /*char(0x82), */0x4d};

    boost::text::text_view tv(utf8, 5); // Ok. Breakage is not at the ends.
    tv = boost::text::checked_encoding(tv); // Error.
//]
    (void)utf32;
} catch (...) {}

{
//[ broken_construction_ok
    // Three Unicode code points.
    uint32_t const utf32[3] = {0x004d, 0x10302, 0x004d};
    // The middle one is broken.
    char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), /*char(0x82), */0x4d};

    boost::text::text_view tv1(utf8, 5); // Ok. Breakage is not at the ends.
    boost::text::text_view tv2(tv1); // Ok. No checks are done on existing Boost.Text values.
//]
    (void)utf32;
}

}
