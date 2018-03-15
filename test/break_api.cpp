#include <boost/text/grapheme_break.hpp>

#include <boost/text/utf8.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


TEST(break_apis, grapheme_break)
{
    // ÷ 1F3FB × 0308 ÷ 1100 ÷	
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0] COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};

    {
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_grapheme_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps) - cps.begin(), 2);
    }

    {
        auto const range =
            boost::text::grapheme(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
    // Range API
    {
        auto const range = boost::text::grapheme(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }

    {
        auto const range =
            boost::text::grapheme(cps.begin(), cps.begin() + 1, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }

    {
        auto const range =
            boost::text::grapheme(cps.begin(), cps.begin() + 2, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 2);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }

    {
        auto const range =
            boost::text::grapheme(cps.begin(), cps.begin() + 3, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 2);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }

    {
        auto const all_graphemes =
            boost::text::graphemes(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_graphemes = boost::text::graphemes(cps);

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, grapheme_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // ÷ 1F3FB × 0308 ÷ 1100 ÷	
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0] COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    boost::text::string s;
    {
        // 4,2,3 code units, respectively.
        std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(s.size() == 9);
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    std::distance(
        u32_iter(s.begin(), s.begin(), s.end()),
        u32_iter(s.begin(), s.end(), s.end()));

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, two, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(two, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, three, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(two, end)), 3);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(cp_range)), 2);
    }

    {
        auto const range = boost::text::grapheme(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
    // Range API
    {
        auto const range = boost::text::grapheme(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }

    {
        auto const range = boost::text::grapheme(begin, one, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }

    {
        auto const range = boost::text::grapheme(begin, two, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 2);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }

    {
        auto const range = boost::text::grapheme(begin, three, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 2);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }

    {
        auto const all_graphemes = boost::text::graphemes(begin, end);

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_graphemes = boost::text::graphemes(cp_range);

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}
