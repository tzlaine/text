#include <boost/text/algorithm.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <array>
#include <string>
#include <vector>

using namespace boost;

static_assert(text::detail::is_char_range<text::text_view>{}, "");
static_assert(text::detail::is_char_range<text::text_view const>{}, "");
static_assert(text::detail::is_char_range<text::text>{}, "");
static_assert(text::detail::is_char_range<text::text const>{}, "");
static_assert(text::detail::is_char_range<std::string>{}, "");
static_assert(text::detail::is_char_range<std::string const>{}, "");
static_assert(text::detail::is_char_range<std::vector<char>>{}, "");
static_assert(text::detail::is_char_range<std::vector<char> const>{}, "");
static_assert(text::detail::is_char_range<std::array<char, 5>>{}, "");
static_assert(text::detail::is_char_range<std::array<char, 5> const>{}, "");
static_assert(text::detail::is_char_range<iterator_range<char *>>{}, "");
static_assert(text::detail::is_char_range<iterator_range<char const *>>{}, "");
static_assert(text::detail::is_char_range<iterator_range<std::vector<char>::iterator>>{}, "");
static_assert(text::detail::is_char_range<iterator_range<std::vector<char>::iterator const>>{}, "");
// TODO: Fix this! static_assert(text::detail::is_char_range<char[5]>{}, "");

static_assert(!text::detail::is_char_range<std::vector<wchar_t>>{}, "");
static_assert(!text::detail::is_char_range<std::vector<int>>{}, "");
static_assert(!text::detail::is_char_range<std::array<float, 5>>{}, "");
static_assert(!text::detail::is_char_range<char>{}, "");
static_assert(!text::detail::is_char_range<int>{}, "");
static_assert(!text::detail::is_char_range<iterator_range<wchar_t *>>{}, "");
static_assert(!text::detail::is_char_range<iterator_range<std::vector<int>::iterator>>{}, "");
static_assert(!text::detail::is_char_range<wchar_t[5]>{}, "");
static_assert(!text::detail::is_char_range<int[5]>{}, "");
