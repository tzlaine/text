//[hello_world

#include <boost/text/text.hpp>

#include <iostream>


int main ()
{
    boost::text::text_view const text_view("Hello, world!\n");
    boost::text::text const text("всем привет!\n");

    std::cout << text_view;
    std::cout << text;

    return 0;
}
//]
