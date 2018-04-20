#include <boost/text/algorithm.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utf8.hpp>
#include <boost/text/utility.hpp>

#include <array>
#include <deque>
#include <string>
#include <vector>

using namespace boost;

// positive tests

static_assert(text::detail::is_cp_iter<uint32_t *>{}, "");
static_assert(text::detail::is_cp_iter<uint32_t const *>{}, "");

static_assert(text::detail::is_cp_iter<std::array<uint32_t, 10>::iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::const_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::const_reverse_iterator>{}, "");

static_assert(text::detail::is_cp_iter<std::list<uint32_t>::iterator>{}, "");
static_assert(text::detail::is_cp_iter<std::list<uint32_t>::const_iterator>{}, "");
static_assert(text::detail::is_cp_iter<std::list<uint32_t>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::list<uint32_t>::const_reverse_iterator>{}, "");

static_assert(text::detail::is_cp_iter<text::utf8::to_utf32_iterator<char const *>>{}, "");
static_assert(text::detail::is_cp_iter<text::utf32_range::iterator>{}, "");

static_assert(std::is_same<int *, text::detail::cp_iter_ret_t<int *, text::utf32_range::iterator>>{}, "");



// negative tests

static_assert(!text::detail::is_cp_iter<char *>{}, "");
static_assert(!text::detail::is_cp_iter<char const *>{}, "");

static_assert(!text::detail::is_cp_iter<int *>{}, "");
static_assert(!text::detail::is_cp_iter<int const *>{}, "");

static_assert(!text::detail::is_cp_iter<text::string_view::iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::string_view::const_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::string_view::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::string_view::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<text::string::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::string::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::string::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::string::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<text::unencoded_rope_view::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::unencoded_rope_view::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::unencoded_rope_view::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::unencoded_rope_view::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<text::unencoded_rope::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::unencoded_rope::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<text::unencoded_rope::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<text::unencoded_rope::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<std::string::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::string::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<std::vector<char>::iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::vector<char>::const_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::vector<char>::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::vector<char>::const_reverse_iterator>{},
    "");

static_assert(!text::detail::is_cp_iter<std::array<char, 5>::iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::array<char, 5>::const_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::array<char, 5>::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::array<char, 5>::const_reverse_iterator>{},
    "");

static_assert(!text::detail::is_cp_iter<std::list<char>::iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::list<char>::const_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::list<char>::reverse_iterator>{}, "");
static_assert(!
    text::detail::is_cp_iter<std::list<char>::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<char>{}, "");
static_assert(!text::detail::is_cp_iter<int>{}, "");
static_assert(!text::detail::is_cp_iter<wchar_t *>{}, "");
static_assert(!text::detail::is_cp_iter<std::vector<int>::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<wchar_t[5]>{}, "");
static_assert(!text::detail::is_cp_iter<int[5]>{}, "");
