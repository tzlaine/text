#include <boost/text/trie.hpp>
#include <boost/text/string.hpp>

#include <gtest/gtest.h>

#include <array>


using namespace boost;

TEST(trie, ctors)
{
    {
        trie::trie<std::vector<int>, int> trie;
        trie::trie<std::vector<int>, int> trie_comp(std::less<>{});

        EXPECT_TRUE(trie.empty());
        EXPECT_TRUE(trie_comp.empty());
        EXPECT_EQ(trie.size(), 0);
        EXPECT_EQ(trie.begin(), trie.end());
        EXPECT_EQ(trie_comp.begin(), trie_comp.end());
        EXPECT_FALSE(trie[""]);
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
    trie::trie<std::vector<int>, int> trie_0(elements);
    trie::trie<std::vector<int>, int> trie_comp(elements, std::less<>{});

    {
        trie::trie<std::vector<int>, int> trie(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie<std::vector<int>, int> trie(other);

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie<std::vector<int>, int> trie(std::move(other));

        EXPECT_EQ(trie, trie_0);
    }
}

TEST(trie, assignement)
{
    trie::trie<std::vector<int>, int> trie_0 = {
        {{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};

    {
        trie::trie<std::vector<int>, int> trie;
        trie = {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie<std::vector<int>, int> trie;
        trie = other;

        EXPECT_EQ(trie, trie_0);
    }

    {
        trie::trie<std::vector<int>, int> other(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});
        trie::trie<std::vector<int>, int> trie;
        trie = std::move(other);

        EXPECT_EQ(trie, trie_0);
    }
}

TEST(trie, const_access)
{
    {
        trie::trie<std::vector<int>, int> const trie(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});

        EXPECT_FALSE(trie.empty());
        EXPECT_EQ(trie.size(), 3);
        EXPECT_EQ(trie.max_size(), PTRDIFF_MAX);

        {
            std::vector<trie::trie_element<std::vector<int>, int>> const
                expected_elements = {
                    {{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};
            std::vector<trie::trie_element<std::vector<int>, int>>
                copied_elements(trie.size());

            std::copy(trie.begin(), trie.end(), copied_elements.begin());
            EXPECT_EQ(copied_elements, expected_elements);

            std::copy(trie.rbegin(), trie.rend(), copied_elements.rbegin());
            EXPECT_EQ(copied_elements, expected_elements);
        }
    }

    {
        trie::trie<std::string, int> const trie(
            {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

        {
            EXPECT_TRUE(trie.contains(std::string("foo")));

            EXPECT_TRUE(trie.contains("foo"));

            std::array<char, 3> foo_array = {{'f', 'o', 'o'}};
            EXPECT_TRUE(trie.contains(foo_array));

            std::vector<char> foo_vec = {'f', 'o', 'o'};
            EXPECT_TRUE(trie.contains(foo_vec));

            text::string foo_str = "foo";
            EXPECT_TRUE(trie.contains(foo_str));
        }

        {
            EXPECT_FALSE(trie.contains(std::string("baz")));

            EXPECT_FALSE(trie.contains("baz"));

            std::array<char, 3> baz_array = {{'b', 'a', 'z'}};
            EXPECT_FALSE(trie.contains(baz_array));

            std::vector<char> baz_vec = {'b', 'a', 'z'};
            EXPECT_FALSE(trie.contains(baz_vec));

            text::string baz_str = "baz";
            EXPECT_FALSE(trie.contains(baz_str));
        }

        {
            auto const _it = trie.begin();
            EXPECT_EQ(trie.find(""), _it);
            auto const bar_it = trie.begin();
            EXPECT_EQ(trie.find("bar"), bar_it);
            auto const foo_it = ++trie.begin();
            EXPECT_EQ(trie.find("foo"), foo_it);
            auto const foos_it = ++++trie.begin();
            EXPECT_EQ(trie.find("foos"), foos_it);

            EXPECT_EQ(trie.find("X"), trie.end());
        }

        {
            // TODO
            trie.lower_bound("foo");
            trie.upper_bound("bar");
            trie.equal_range("bar");

            trie[""];
        }

        {
            auto const _match = trie.longest_match("");
            EXPECT_TRUE(_match.node != nullptr);
            EXPECT_EQ(_match.size, 0);
            EXPECT_EQ(_match.match, true);

            auto const _0_match = trie.extend_match(_match, 0);
            EXPECT_EQ(_0_match, _match);

            auto const f_match = trie.extend_match(_match, 0);
            EXPECT_TRUE(f_match.node != nullptr);
            EXPECT_EQ(f_match.size, 1);
            EXPECT_EQ(f_match.match, false);
        }

        {
            auto const fo_match = trie.longest_match("fo");
            EXPECT_TRUE(fo_match.node != nullptr);
            EXPECT_EQ(fo_match.size, 2);
            EXPECT_EQ(fo_match.match, false);
        }

        {
            auto const fa_match = trie.longest_match("fa");
            EXPECT_TRUE(fa_match.node != nullptr);
            EXPECT_EQ(fa_match.size, 1);
            EXPECT_EQ(fa_match.match, false);
        }

        {
            auto const bart_match = trie.longest_match("bart");
            EXPECT_TRUE(bart_match.node != nullptr);
            EXPECT_EQ(bart_match.size, 3);
            EXPECT_EQ(bart_match.match, true);
        }
    }
}

TEST(trie, mutable_access)
{
    {
        trie::trie<std::vector<int>, int> trie(
            {{{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}});

        EXPECT_FALSE(trie.empty());
        EXPECT_EQ(trie.size(), 3);
        EXPECT_EQ(trie.max_size(), PTRDIFF_MAX);

        {
            std::vector<trie::trie_element<std::vector<int>, int>> const
                expected_elements = {
                    {{0, 1, 3}, 13}, {{0}, 17}, {{0, 1, 2}, 19}};
            std::vector<trie::trie_element<std::vector<int>, int>>
                copied_elements(trie.size());

            std::copy(trie.begin(), trie.end(), copied_elements.begin());
            EXPECT_EQ(copied_elements, expected_elements);
            std::copy(trie.cbegin(), trie.cend(), copied_elements.begin());
            EXPECT_EQ(copied_elements, expected_elements);

            std::copy(trie.rbegin(), trie.rend(), copied_elements.rbegin());
            EXPECT_EQ(copied_elements, expected_elements);
            std::copy(trie.crbegin(), trie.crend(), copied_elements.rbegin());
            EXPECT_EQ(copied_elements, expected_elements);
        }
    }

    {
        trie::trie<std::string, int> trie(
            {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

        {
            EXPECT_TRUE(trie.contains(std::string("foo")));

            EXPECT_TRUE(trie.contains("foo"));

            std::array<char, 3> foo_array = {{'f', 'o', 'o'}};
            EXPECT_TRUE(trie.contains(foo_array));

            std::vector<char> foo_vec = {'f', 'o', 'o'};
            EXPECT_TRUE(trie.contains(foo_vec));

            text::string foo_str = "foo";
            EXPECT_TRUE(trie.contains(foo_str));
        }

        {
            EXPECT_FALSE(trie.contains(std::string("baz")));

            EXPECT_FALSE(trie.contains("baz"));

            std::array<char, 3> baz_array = {{'b', 'a', 'z'}};
            EXPECT_FALSE(trie.contains(baz_array));

            std::vector<char> baz_vec = {'b', 'a', 'z'};
            EXPECT_FALSE(trie.contains(baz_vec));

            text::string baz_str = "baz";
            EXPECT_FALSE(trie.contains(baz_str));
        }

        {
            auto const _it = trie.begin();
            EXPECT_EQ(trie.find(""), _it);
            auto const bar_it = trie.begin();
            EXPECT_EQ(trie.find("bar"), bar_it);
            auto const foo_it = ++trie.begin();
            EXPECT_EQ(trie.find("foo"), foo_it);
            auto const foos_it = ++++trie.begin();
            EXPECT_EQ(trie.find("foos"), foos_it);

            EXPECT_EQ(trie.find("X"), trie.end());
        }

        {
            // TODO
            trie.lower_bound("foo");
            trie.upper_bound("bar");
            trie.equal_range("bar");

            trie[""];
        }
    }
}

TEST(trie, index_operator)
{
    trie::trie<std::string, int> trie(
        {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

    EXPECT_TRUE(trie["foo"]);
    EXPECT_TRUE(trie["bar"]);
    EXPECT_TRUE(trie["foos"]);
    EXPECT_TRUE(trie[""]);

    EXPECT_EQ(*trie["foo"], 13);
    EXPECT_EQ(*trie["bar"], 17);
    EXPECT_EQ(*trie["foos"], 19);
    EXPECT_EQ(*trie[""], 42);

    *trie["foo"] = 0;
    *trie["bar"] = 1;
    *trie["foos"] = 2;
    *trie[""] = 3;

    EXPECT_EQ(*trie["foo"], 0);
    EXPECT_EQ(*trie["bar"], 1);
    EXPECT_EQ(*trie["foos"], 2);
    EXPECT_EQ(*trie[""], 3);
}

TEST(trie, insert)
{
    trie::trie<std::string, int> trie(
        {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

    // TODO
}

TEST(trie, erase)
{
    trie::trie<std::string, int> trie(
        {{"foo", 13}, {"bar", 17}, {"foos", 19}, {"", 42}});

    // TODO
    trie.erase("foo");
}

