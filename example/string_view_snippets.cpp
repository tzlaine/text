#include <boost/text/string.hpp>
#include <boost/text/algorithm.hpp>


int main ()
{
//[ string_view_lotsa_constexpr
constexpr boost::text::string_view tv_1 = "Compile-time value.";
constexpr boost::text::string_view tv_2 = tv_1(0, 7); // Slice off the front.
static_assert(tv_2 == "Compile");
//]
(void)tv_1;
(void)tv_2;

//[ string_view_literal
using namespace boost::text::literals;
boost::text::string_view tv = "UDLs are, literally, the best idea."_sv;
//]
(void)tv;
}
