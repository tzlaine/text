#include <boost/text/text_view.hpp>

#include <iostream>


int main ()
{
    boost::text::text_view const tv;

    {
        using namespace boost::text::literals;
        boost::text::text_view const tv2 = ""_tv;
        (void)tv2;
    }

    find(tv, tv);
    find_view(tv, tv);
    rfind(tv, tv);
    rfind_view(tv, tv);

    find_first_of(tv, tv);
    find_first_not_of(tv, tv);
    find_last_of(tv, tv);
    find_last_not_of(tv, tv);

    substr(tv, 0, 0);

    compare(tv, tv);

    std::cout << "tv=\"" << tv << "\"";
}
