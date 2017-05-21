#include <boost/text/text.hpp>

int main ()
{
    boost::text::text_view sv;
    find(sv, sv);
    substr(sv, 0, 0);
    compare(sv, sv);
}
