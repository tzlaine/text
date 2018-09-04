#include <boost/text/grapheme_break.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/text.hpp>

#include <iostream>


int main ()
{

{
//[ grapheme_breaks

// U+0308 COMBINING ACUTE ACCENT
std::array<uint32_t, 3> cps = {'a', 0x0308, 'b'};

auto const first = cps.begin();
auto const last = cps.end();

auto at_or_before_1 = boost::text::prev_grapheme_break(first, first + 1, last);
assert(at_or_before_1 == first + 0);

auto at_or_before_2 = boost::text::prev_grapheme_break(cps, first + 2);
assert(at_or_before_2 == first + 2);

auto at_or_before_3 = boost::text::prev_grapheme_break(first, first + 3, last);
assert(at_or_before_3 == first + 2);

auto after_0 = boost::text::next_grapheme_break(first, last);
assert(after_0 == first + 2);

auto around_1 = boost::text::grapheme(cps, first + 1);
assert(around_1.begin() == first + 0);
assert(around_1.end() == first + 2);

// Prints "[0, 2) [2, 3)".
for (auto range : boost::text::graphemes(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ") ";
}
std::cout << "\n";

// Prints "[2, 3) [0, 2)".
for (auto range : boost::text::reversed_graphemes(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ") ";
}
std::cout << "\n";
//]
}

{
//[ word_breaks_1
// Using GraphemeRange/GraphemeIterator overloads...
boost::text::text cps("The quick (\“brown\”) fox can’t jump 32.3 feet, right?");

auto const first = cps.cbegin();

auto at_or_before_1 = boost::text::prev_word_break(cps, std::next(first, 1));
assert(at_or_before_1 == std::next(first, 0));

auto at_or_before_3 = boost::text::prev_word_break(cps, std::next(first, 3));
assert(at_or_before_3 == std::next(first, 3));

auto after_0 = boost::text::next_word_break(cps, first);
assert(after_0 == std::next(first, 3));

auto around_7 = boost::text::word(cps, std::next(first, 7));
assert(around_7.begin() == std::next(first, 4));
assert(around_7.end() == std::next(first, 9));

// Prints the indices of the words from the table above.
for (auto range : boost::text::words(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ") ";
}
std::cout << "\n";

// Prints the indices of the words from the table above, backward.
for (auto range : boost::text::reversed_words(cps)) {
    std::cout << '[' << std::distance(first, range.begin()) << ", "
              << std::distance(first, range.end()) << ") ";
}
std::cout << "\n";
//]
}

{
//[ word_breaks_2
boost::text::text cps("out-of-the-box");

// Prints "out - of - the - box".
for (auto range : boost::text::words(cps)) {
    std::cout << boost::text::text_view(range) << " ";
}
std::cout << "\n";

auto const custom_word_prop = [](uint32_t cp) {
    if (cp == '-')
        return boost::text::word_property::MidLetter; // '-' becomes a MidLetter
    return boost::text::word_prop(cp); // Otherwise, just use the default implementation.
};

// Prints "out-of-the-box".
for (auto range : boost::text::words(cps, custom_word_prop)) {
    std::cout << boost::text::text_view(range) << " ";
}
std::cout << "\n";
//]
}

{
//[ word_breaks_3
boost::text::text cps("snake_case camelCase");

// Prints "snake_case   camelCase".
for (auto range : boost::text::words(cps)) {
    std::cout << boost::text::text_view(range) << " ";
}
std::cout << "\n";

// Break up words into chunks as if they were identifiers in a popular
// programming language.
auto const identifier_break = [](uint32_t prev_prev,
                                 uint32_t prev,
                                 uint32_t curr,
                                 uint32_t next,
                                 uint32_t next_next) {
    if ((prev == '_') != (curr == '_'))
        return true;
    if (0x61 <= prev && prev <= 0x7a && 0x41 <= curr && curr <= 0x5a)
        return true;
    return false;
};

// Prints "snake _ case   camel Case".
for (auto range :
     boost::text::words(cps, boost::text::word_prop, identifier_break)) {
    std::cout << boost::text::text_view(range) << " ";
}
std::cout << "\n";
//]
}

{
//[ line_breaks
//]
}

}
