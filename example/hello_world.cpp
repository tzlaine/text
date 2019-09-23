//[ hello_world

#include <boost/text/string.hpp>

#include <iostream>


int main ()
{
    boost::text::string_view const string_view("Hello, world!\n");
    boost::text::string const string(u8"всем привет!\n");

    std::cout << string_view;
    std::cout << string;

    return 0;
}
//]
