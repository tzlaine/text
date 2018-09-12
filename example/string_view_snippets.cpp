#include <boost/text/string.hpp>
#include <boost/text/algorithm.hpp>


int main ()
{
//[ string_view_lotsa_constexpr
#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR // Requires C++14 constexpr support.
constexpr boost::text::string_view tv_1 = "Compile-time value.";
constexpr boost::text::string_view tv_2 = tv_1(0, 7); // Slice off a prefix.
static_assert(tv_2 == "Compile", "Something is really wrong.");
#endif
//]
#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR
(void)tv_1;
(void)tv_2;
#endif

//[ string_view_literal
using namespace boost::text::literals;
boost::text::string_view tv = "UDLs are, literally, the best idea."_sv;
//]
(void)tv;
}
