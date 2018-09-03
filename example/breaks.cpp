#include <boost/text/grapheme_break.hpp>

#include <iostream>


int main ()
{

{
//[ grapheme_breaks

// U+0308 COMBINING ACUTE ACCENT
std::array<uint32_t, 3> cps = {'a', 0x0308, 'b'};

auto const first = cps.begin();
auto const last = cps.end();

auto at_or_before_1 =
    boost::text::prev_grapheme_break(first, first + 1, last);
assert(at_or_before_1 == first + 0);

auto at_or_before_2 = boost::text::prev_grapheme_break(cps, first + 2);
assert(at_or_before_2 == first + 2);

auto at_or_before_3 =
    boost::text::prev_grapheme_break(first, first + 3, last);
assert(at_or_before_3 == first + 2);

auto after_0 = boost::text::next_grapheme_break(first, last);
assert(after_0 == first + 2);

auto around_1 = boost::text::grapheme(cps, first + 1);
assert(around_1.begin() == first + 0);
assert(around_1.end() == first + 2);

// Prints [0, 2) [2, 3)
for (auto range : boost::text::graphemes(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ")\n";
}

// Prints [2, 3) [0, 2)
for (auto range : boost::text::reversed_graphemes(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ")\n";
}
//]
}

{
//[ word_breaks
//]
}

}
