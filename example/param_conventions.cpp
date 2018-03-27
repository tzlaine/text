#include <boost/text/string.hpp>
#include <iostream>

//[ params_to_take_string_and_string_view_without

void print_string_without (boost::text::string_view str)
{ std::cout << str; }

void print_string_without (boost::text::string const & str)
{ print_string_without(boost::text::string_view(str)); }

void print_string_without (char const * str)
{ print_string_without(boost::text::string_view(str)); }

//]

//[ params_to_take_string_and_string_view_with

void print_string_with (boost::text::string_view str)
{ std::cout << str; }

//]

//[ params_to_take_anything

void print_any_string_type (boost::text::unencoded_rope_view str)
{ std::cout << str; }

//]

int main ()
{
    print_string_without("1\n");
    print_string_without(boost::text::string_view("2\n"));
    print_string_without(boost::text::string("3\n"));

    print_string_with("1\n");
    print_string_with(boost::text::string_view("2\n"));
    boost::text::string str_3_("3\n");
    print_string_with(str_3_);

//[ print_any_string_type_usage
    print_any_string_type("1\n");
    print_any_string_type(boost::text::string_view("2\n"));
    boost::text::string str_3("3\n");
    print_any_string_type(str_3);
    print_any_string_type(boost::text::repeat("4\n", 4));
    boost::text::unencoded_rope rope_5("5\n");
    print_any_string_type(rope_5);
//]
}
