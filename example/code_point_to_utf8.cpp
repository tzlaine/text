#include <boost/text/text.hpp>
#include <iostream>


void usage_error()
{
    std::cerr << "code_point_to_utf8: error: code_point_to_utf8 takes\n"
                 "    exactly one parameter, a hexadecimal code point.\n";
    exit(1);
}

int main(int argc, char * argv[])
{
    if (argc != 2)
        usage_error();

    auto input = std::string(argv[1]);
    if (2 < input.size()) {
        if (input[0] == '0' && input[1] == 'x')
            input.erase(input.begin(), input.begin() + 2);
    }

    uint32_t code_point[1] = {0};
    for (auto c : input) {
        uint32_t value = 0;
        if ('a' <= c && c <= 'f')
            value = 10 + c - 'a';
        else if ('A' <= c && c <= 'F')
            value = 10 + c - 'A';
        else if ('0' <= c && c <= '9')
            value = c - '0';
        else
            usage_error();
        code_point[0] *= 16;
        code_point[0] += value;
    }

    boost::text::utf8::from_utf32_iterator<uint32_t const *> const cp_first(
        code_point);
    boost::text::utf8::from_utf32_iterator<uint32_t const *> const cp_last(
        code_point + 1);
    unsigned char code_units[4];
    auto code_units_end = std::copy(cp_first, cp_last, code_units);

    for (unsigned char * it = code_units; it != code_units_end; ++it) {
        std::cout << "0x" << std::hex << int(*it) << " ";
    }
    std::cout << "\n";

#if 0
    boost::text::utf8::to_utf32_iterator<unsigned char const *> const cu_first(
        code_units);
    boost::text::utf8::to_utf32_iterator<unsigned char const *> const cu_last(
        code_units_end);
    std::copy(cu_first, cu_last, code_point);
    std::cout << "Round-trips back to: 0x" << std::hex << code_point[0] << "\n";
#endif
}
