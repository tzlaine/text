#include <boost/text/text.hpp>
#include <boost/text/algorithm.hpp>


int main ()
{

{
//[ text_literal
using namespace boost::text::literals;
boost::text::text t = "UDL here."_t;
//]
(void)t;
}

try {
//[ char_rng_vs_iters_insertion]
// Two Unicode code points.
uint32_t const utf32[2] = {0x004d, 0x10302};
// The second one is truncated.
char const utf8[5] = {0x4d, char(0xf0), char(0x90), char(0x8c), '\0'};

std::string const str = utf8;

boost::text::text t1;
t1.insert(0, str.begin(), str.end()); // Ok.  The broken encoding is never checked.

boost::text::text t2;
t2.insert(0, str); // Error!  The encoding is broken at the end.
//]
(void)utf32;
} catch (...) {}

}
