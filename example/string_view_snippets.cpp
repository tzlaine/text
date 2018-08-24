#include <boost/text/string.hpp>
#include <boost/text/algorithm.hpp>


int main ()
{
//[ string_view_literal
using namespace boost::text::literals;
boost::text::string_view tv = "UDLs are, literally, the best idea."_sv;
//]
(void)tv;
}
