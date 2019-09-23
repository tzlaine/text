#include <boost/text/string_builder.hpp>

#include <iostream>


int main ()
{

{
//[ string_builder

boost::text::string_builder builder;
builder += "some";
builder += " ";
builder += "text";

// Null termination fianlly happens here, and the array owned by builder is
// moved into result.
boost::text::string const result = builder.to_string();

// Prints "some text".
std::cout << result << "\n";
//]
}

{
#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

//[ static_string_builder
constexpr boost::text::static_string_builder builder;   // Empty builder.
constexpr auto builder_1 = builder + "some";            // A builder containing one string_view.
constexpr char const * space = " ";
constexpr auto builder_2 = builder_1 + space;           // A builder containing two string_views.
constexpr auto builder_3 = builder_2 + "text";          // A builder containing three string_views.

// A single allocation happens here.
boost::text::string const result = builder_3.to_string();

// Prints "some text".
std::cout << result << "\n";
//]

#endif
}

}
