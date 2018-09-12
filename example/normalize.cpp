#include <boost/text/normalize.hpp>


int main ()
{

{
//[ normalize_1
// 쨰◌̴ᆮ HANGUL SYLLABLE JJYAE, COMBINING TILDE OVERLAY, HANGUL JONGSEONG TIKEUT
std::array<uint32_t, 4> const nfd = {{ 0x110D, 0x1164, 0x0334, 0x11AE }};
assert(boost::text::normalized_nfd(nfd.begin(), nfd.end())); // Iterator interface.

std::array<uint32_t, 3> nfc;
boost::text::normalize_to_nfc(nfd.begin(), nfd.end(), nfc.begin()); // Iterator interface.
assert(boost::text::normalized_nfc(nfc)); // Range interface.

boost::text::normalize_to_nfc(nfd, nfc.begin()); // Range interface.
assert(boost::text::normalized_nfc(nfc)); // Range interface.
//]
}

}
