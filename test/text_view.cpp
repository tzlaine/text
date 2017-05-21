#include <boost/text/text.hpp>

#include <iostream>


int main ()
{
    boost::text::text_view const sv;

    find(sv, sv);
    find_view(sv, sv);
    rfind(sv, sv);
    rfind_view(sv, sv);

    find_first_of(sv, sv);
    find_first_not_of(sv, sv);
    find_last_of(sv, sv);
    find_last_not_of(sv, sv);

    substr(sv, 0, 0);

    compare(sv, sv);

    std::cout << "sv=\"" << sv << "\"";
}
