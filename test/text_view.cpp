#include <boost/text/text.hpp>


int main ()
{
    boost::text::text_view const sv;
    find(sv, sv);
    find_view(sv, sv);
    rfind(sv, sv);
    rfind_view(sv, sv);
    substr(sv, 0, 0);
    compare(sv, sv);
}
