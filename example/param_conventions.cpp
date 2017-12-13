#include <boost/text/text.hpp>
#include <iostream>

//[ params_to_take_text_and_text_view_without

void print_string_without (boost::text::text_view str)
{ std::cout << str; }

void print_string_without (boost::text::text const & str)
{ print_string_without(boost::text::text_view(str)); }

void print_string_without (char const * str)
{ print_string_without(boost::text::text_view(str)); }

//]

//[ params_to_take_text_and_text_view_with

void print_string_with (boost::text::text_view str)
{ std::cout << str; }

//]

//[ params_to_take_anything

void print_any_text_type (boost::text::unencoded_rope_view str)
{ std::cout << str; }

//]

int main ()
{
    print_string_without("1\n");
    print_string_without(boost::text::text_view("2\n"));
    print_string_without(boost::text::text("3\n"));

    print_string_with("1\n");
    print_string_with(boost::text::text_view("2\n"));
    print_string_with(boost::text::text("3\n"));

//[ print_any_text_type_usage
    print_any_text_type("1\n");
    print_any_text_type(boost::text::text_view("2\n"));
    print_any_text_type(boost::text::text("3\n"));
    print_any_text_type(boost::text::repeat("4\n", 4));
    print_any_text_type(boost::text::unencoded_rope("5\n"));
//]
}
