// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
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
