#include <boost/text/text.hpp>


int main ()
{
    boost::text::text_view const sv;
    find(sv, sv);
    rfind(sv, sv);
    substr(sv, 0, 0);
    compare(sv, sv);
}
