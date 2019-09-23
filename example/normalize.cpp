#include <boost/text/normalize.hpp>


int main ()
{

{
//[ normalize_1
// 쨰◌̴ᆮ HANGUL SYLLABLE JJYAE, COMBINING TILDE OVERLAY, HANGUL JONGSEONG TIKEUT
std::array<uint32_t, 4> const nfd = {{ 0x110D, 0x1164, 0x0334, 0x11AE }};
assert(boost::text::normalized_nfd(nfd.begin(), nfd.end())); // Iterator interface.

{
    std::vector<uint32_t> nfc;
    boost::text::normalize_to_nfc(
        nfd.begin(), nfd.end(), std::back_inserter(nfc)); // Iterator interface.
    assert(boost::text::normalized_nfc(nfc));             // Range interface.
}

{
    std::vector<uint32_t> nfc;
    boost::text::normalize_to_nfc(
        nfd, std::back_inserter(nfc));        // Range interface.
    assert(boost::text::normalized_nfc(nfc)); // Range interface.
}
//]
}

}
