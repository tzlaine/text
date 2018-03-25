#include <boost/text/string.hpp>
#include <boost/text/algorithm.hpp>


#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR
//[ string_view_lotsa_constexpr
constexpr int find_spaces (boost::text::string_view tv)
{
    int retval = 0;
    boost::text::string_view space = boost::text::find_view(tv, " ");
    while (!space.empty()) {
        ++retval;
        tv = boost::text::string_view(space.end(), tv.end() - space.end());
        space = boost::text::find_view(tv, " ");
    }
    return retval;
}
//<-
#endif

int main ()
{

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR
{
//->

//=int main ()
//={
    constexpr boost::text::string_view tv =
        "When writing a specialization, "
        "be careful about its location; "
        "or to make it compile "
        "will be such a trial "
        "as to kindle its self-immolation";

    constexpr int spaces_found = find_spaces(tv);
    static_assert(spaces_found == 23, "");
//=}
//]
}
#endif

{
//[ string_view_literal
using namespace boost::text::literals;
boost::text::string_view tv = "UDLs are, literally, the best idea."_sv;
//]
(void)tv;
}

}
