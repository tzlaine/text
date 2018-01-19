#define ENABLE_DUMP 0
#include "trie_tests.hpp"

#include <boost/text/trie_map.hpp>
#include <boost/text/string.hpp>

#include <gtest/gtest.h>

#include <array>


using namespace boost;

TEST(trie_map, ctors)
{
    {
        trie::trie_map<std::vector<int>, int> trie;
        trie::trie_map<std::vector<int>, int> trie_comp(trie::less{});

        EXPECT_TRUE(trie.empty());
        EXPECT_TRUE(trie_comp.empty());
        EXPECT_EQ(trie.size(), 0);
        EXPECT_EQ(trie.begin(), trie.end());
        EXPECT_EQ(trie_comp.begin(), trie_comp.end());
        EXPECT_FALSE(trie[std::vector<int>{}]);
    }

    struct my_element
    {
        std::vector<int> key;
        int value;
    };
    std::array<my_element, 3> const elements = {{
        {{0, 1, 3}, 13},
        {{0}, 17},
        {{0, 1, 2}, 19},
    }};
    trie::trie_map<std::vector<int>, int> trie_0(elements);
    trie::trie_map<std::vector<int>, int> trie_comp(elements, trie::less{});

    {
        trie::trie_map<std::vector<int>, int> trie(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie_map<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie_map<std::vector<int>, int> trie(other);

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie_map<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie_map<std::vector<int>, int> trie(std::move(other));

        EXPECT_EQ(trie, trie_0);
    }
}

TEST(trie_map, assignment)
{
    trie::trie_map<std::vector<int>, int> const trie_0 = {
        {{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};

    {
        trie::trie_map<std::vector<int>, int> trie;
        trie = {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie_map<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie_map<std::vector<int>, int> trie;
        trie = other;

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie_map<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie_map<std::vector<int>, int> trie;
        trie = std::move(other);

        EXPECT_EQ(trie, trie_0);
    }
}

TEST(trie_map, iterators)
{
    {
        trie::trie_map<std::string, int> trie(
            {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

#if ENABLE_DUMP
        // dump(std::cout, trie);
#endif

        auto const _it = trie.begin();
        EXPECT_EQ(_it->key, "");
        auto const bar_it = std::next(_it);
        EXPECT_EQ(bar_it->key, "bar");
        auto const foo_it = std::next(bar_it);
        EXPECT_EQ(foo_it->key, "foo");
        auto const foos_it = std::next(foo_it);
        EXPECT_EQ(foos_it->key, "foos");

        auto const end_it = std::next(foos_it);
        EXPECT_EQ(end_it, trie.end());

        auto const foos_it_2 = std::prev(end_it);
        EXPECT_EQ(foos_it_2->key, "foos");
        auto const foo_it_2 = std::prev(foos_it_2);
        EXPECT_EQ(foo_it_2->key, "foo");
        auto const bar_it_2 = std::prev(foo_it_2);
        EXPECT_EQ(bar_it_2->key, "bar");
        auto const _it_2 = std::prev(bar_it_2);
        EXPECT_EQ(_it_2->key, "");

        EXPECT_EQ(_it_2, trie.begin());
    }

    {
        trie::trie_map<std::string, int> const trie(
            {{"foo", 13}, {"bar", 17}, {"foos", 19}});

#if ENABLE_DUMP
        // dump(std::cout, trie);
#endif

        auto const bar_it = trie.begin();
        EXPECT_EQ(bar_it->key, "bar");
        auto const foo_it = std::next(bar_it);
        EXPECT_EQ(foo_it->key, "foo");
        auto const foos_it = std::next(foo_it);
        EXPECT_EQ(foos_it->key, "foos");

        auto const end_it = std::next(foos_it);
        EXPECT_EQ(end_it, trie.end());

        auto const foos_it_2 = std::prev(end_it);
        EXPECT_EQ(foos_it_2->key, "foos");
        auto const foo_it_2 = std::prev(foos_it_2);
        EXPECT_EQ(foo_it_2->key, "foo");
        auto const bar_it_2 = std::prev(foo_it_2);
        EXPECT_EQ(bar_it_2->key, "bar");

        EXPECT_EQ(bar_it_2, trie.begin());
    }

    {
        trie::trie_map<std::vector<int>, int> const trie(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});

#if ENABLE_DUMP
        // dump(std::cout, trie);
#endif

        auto const _0_it = trie.begin();
        EXPECT_EQ(_0_it->key, std::vector<int>{0});
        auto const _012_it = std::next(_0_it);
        EXPECT_EQ(_012_it->key, (std::vector<int>{0, 1, 2}));
        auto const _013_it = std::next(_012_it);
        EXPECT_EQ(_013_it->key, (std::vector<int>{0, 1, 3}));

        auto const end_it = std::next(_013_it);
        EXPECT_EQ(end_it, trie.end());

        auto const _013_it_2 = std::prev(end_it);
        EXPECT_EQ(_013_it_2->key, (std::vector<int>{0, 1, 3}));
        auto const _012_it_2 = std::prev(_013_it_2);
        EXPECT_EQ(_012_it_2->key, (std::vector<int>{0, 1, 2}));
        auto const _0_it_2 = std::prev(_012_it_2);
        EXPECT_EQ(_0_it_2->key, std::vector<int>{0});

        EXPECT_EQ(_0_it_2, trie.begin());
    }

    {
        trie::trie_map<std::string, int> const trie(
            {{"bar", 17}, {"foon", 19}, {"fool", 19}, {"foo", 13}, {"", 42}});

        auto it = trie.begin();
        EXPECT_EQ(it->key, "");
        ++it;
        EXPECT_EQ(it->key, "bar");
        ++it;
        EXPECT_EQ(it->key, "foo");
        ++it;
        EXPECT_EQ(it->key, "fool");
        ++it;
        EXPECT_EQ(it->key, "foon");
        ++it;
        EXPECT_EQ(it, trie.end());
    }

    {
        trie::trie_map<std::string, int> trie(
            {{"foo", 13}, {"bar", 17}, {"fool", 19}, {"foon", 19}, {"", 42}});

        auto it = trie.begin();
        EXPECT_EQ(it->key, "");
        ++it;
        EXPECT_EQ(it->key, "bar");
        ++it;
        EXPECT_EQ(it->key, "foo");
        ++it;
        EXPECT_EQ(it->key, "fool");
        ++it;
        EXPECT_EQ(it->key, "foon");
        ++it;
        EXPECT_EQ(it, trie.end());
    }

    {
        // Sequence generated by the fuzz test.
        trie::trie_map<std::string, int> trie;
        trie.insert("/r", 543252833);        // key.size()=2
        trie.insert("/r", 543252833);        // key.size()=2
        trie.insert("P ", 1595957601);       // key.size()=2
        trie.insert("P ", 1595957601);       // key.size()=2
        trie.insert("a aa /r", 1633771873);  // key.size()=7
        trie.insert("a aa /r", 1633771873);  // key.size()=7
        trie.insert("QQQ", 542181920);       // key.size()=3
        trie.insert("QQQ", 542181920);       // key.size()=3
        trie.insert("a _P _P ", 1633771873); // key.size()=8
        trie.insert("a _P _P ", 1633771873); // key.size()=8
        trie.insert(" _a _PP ", 1633771873); // key.size()=8
        trie.insert(" _a _PP ", 1633771873); // key.size()=8
        trie.insert("QAQ", 542181920);       // key.size()=3
        trie.insert("QAQ", 542181920);       // key.size()=3
        trie.insert("/r", 543236128);        // key.size()=2
        trie.insert("/ra", 543236128);       // key.size()=3

        auto const first = trie.begin();
        auto const last = trie.end();
        auto const incremental_last = std::next(first, trie.size());
        EXPECT_EQ(last, incremental_last);

        std::vector<trie::trie_map_element<std::string, int>> copied_elements(
            trie.size());
        std::copy(trie.begin(), trie.end(), copied_elements.begin());

        std::vector<trie::trie_map_element<std::string, int>>
            reversed_copied_elements(trie.size());
        std::copy(
            trie.rbegin(), trie.rend(), reversed_copied_elements.rbegin());

#if ENABLE_DUMP
        //dump(std::cout, trie);
#endif

        EXPECT_EQ(reversed_copied_elements, copied_elements);
    }
}
