#include <boost/text/segmented_vector.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

TEST(segmented_vector, test_empty)
{
    text::segmented_vector<int> t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.begin(), t.end());

    t.swap(t);
    EXPECT_TRUE(t == t);

    EXPECT_EQ(t.begin(), std::begin(t));
    EXPECT_EQ(t.end(), std::end(t));

    EXPECT_EQ(t.rbegin(), std::rbegin(t));
    EXPECT_EQ(t.rend(), std::rend(t));

    t.clear();
}

TEST(segmented_vector, test_non_empty_const_interface)
{
    text::segmented_vector<int> t_a({0});
    text::segmented_vector<int> t_ab({1, 2});

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1);

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);

    EXPECT_EQ(t_ab[1], 2);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);

    text::segmented_vector<int> const old_t_a(t_a);
    text::segmented_vector<int> const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    EXPECT_EQ(t_a.begin(), std::begin(t_a));
    EXPECT_EQ(t_a.end(), std::end(t_a));

    EXPECT_EQ(t_a.rbegin(), std::rbegin(t_a));
    EXPECT_EQ(t_a.rend(), std::rend(t_a));
}

TEST(segmented_vector, test_ctors)
{
    text::segmented_vector<int> t;
    EXPECT_EQ(t, text::segmented_vector<int>{});
    EXPECT_EQ(text::segmented_vector<int>{}, t);

    text::segmented_vector<int> t2({7, 8, 9});
    EXPECT_EQ(t2[0], 7);
    EXPECT_EQ(t2[1], 8);
    EXPECT_EQ(t2[2], 9);

    text::segmented_vector<int> t3(t2);
    EXPECT_EQ(t3[0], 7);
    EXPECT_EQ(t3[1], 8);
    EXPECT_EQ(t3[2], 9);

    text::segmented_vector<int> t4(std::move(t2));
    EXPECT_EQ(t4[0], 7);
    EXPECT_EQ(t4[1], 8);
    EXPECT_EQ(t4[2], 9);
    EXPECT_EQ(t2, text::segmented_vector<int>{});
    EXPECT_EQ(text::segmented_vector<int>{}, t2);
}

TEST(segmented_vector, test_insert)
{
    std::vector<int> _789{7, 8, 9};

    {
        text::segmented_vector<int> const ct({0, 1, 2, 3, 4, 5});

        {
            text::segmented_vector<int> t0 = ct;
            t0.insert(t0.begin(), _789.begin(), _789.end());
            std::vector<int> const vec{7, 8, 9, 0, 1, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t0.begin(), t0.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t1 = ct;
            t1.insert(t1.begin() + 1, _789.begin(), _789.end());
            std::vector<int> const vec{0, 7, 8, 9, 1, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t1.begin(), t1.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t2 = ct;
            t2.insert(t2.begin() + 2, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 7, 8, 9, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t2.begin(), t2.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t3 = ct;
            t3.insert(t3.begin() + 3, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 7, 8, 9, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t3.begin(), t3.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t4 = ct;
            t4.insert(t4.begin() + 4, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 7, 8, 9, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t4.begin(), t4.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t5 = ct;
            t5.insert(t5.begin() + 5, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 4, 7, 8, 9, 5};
            EXPECT_TRUE(
                algorithm::equal(t5.begin(), t5.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t6 = ct;
            t6.insert(t6.begin() + 6, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 4, 5, 7, 8, 9};
            EXPECT_TRUE(
                algorithm::equal(t6.begin(), t6.end(), vec.begin(), vec.end()));
        }
    }
}

#if 0 // Crashes!
TEST(segmented_vector, test_erase)
{
    {
        text::segmented_vector<int> t({0, 1, 2, 3, 4, 5});
        t.erase(t.begin(), t.end());
        EXPECT_EQ(t, text::segmented_vector<int>{});
    }

    text::segmented_vector<int> const ct({0, 1, 2, 3, 4, 5});

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;

            text::segmented_vector<int> expected(ct.begin(), ct.begin() + i);
            expected.insert(expected.end(), ct.begin() + j, ct.end());

            t.erase(ct.begin() + i, ct.begin() + j);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j;
        }
    }
}
#endif

#if 0 // TODO
TEST(segmented_vector, test_replace)
{
    text::string_view const replacement("REP");
    // Explicitly null-terminated.
    text::string_view const replacement_with_null(
        replacement.begin(), replacement.size() + 1);

    {
        text::segmented_vector<int> t("string");
        text::segmented_vector<int>_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement_with_null);
        EXPECT_EQ(t, "REP");
    }

    {
        text::segmented_vector<int> t("string");
        text::segmented_vector<int>_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::segmented_vector<int> t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::segmented_vector<int> t("string");
        t.replace(t(0, 3), t(2, 6));
        EXPECT_EQ(t, "ringing");
    }

    {
        text::segmented_vector<int> t("string");
        t.replace(t(3, 6), t(0, 3));
        EXPECT_EQ(t, "strstr");
    }

    text::segmented_vector<int> const ct("string");

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;
            text::segmented_vector<int>_view const before = t(0, i);
            text::segmented_vector<int>_view const substr = t(i, j);
            text::segmented_vector<int>_view const after = t(j, t.size());

            text::segmented_vector<int> expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr, replacement);
            EXPECT_EQ(t, expected)
                << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    text::repeated_string_view const really_long_replacement(replacement, 10);

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;
            text::segmented_vector<int>_view const before = t(0, i);
            text::segmented_vector<int>_view const substr = t(i, j);
            text::segmented_vector<int>_view const after = t(j, t.size());

            text::segmented_vector<int> expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr, really_long_replacement);
            EXPECT_EQ(t, expected)
                << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }
}

TEST(segmented_vector, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);
    auto const final_cp =
        text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
    auto const last =
        text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

    text::segmented_vector<int> const ct_string("string");
    text::segmented_vector<int> const ct_text("text");

    {
        text::segmented_vector<int> t = ct_string;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::segmented_vector<int> t = ct_text;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::segmented_vector<int> t = ct_string;
        t.replace(t, first, last);
        EXPECT_EQ(t, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }

    for (int j = 0; j <= ct_string.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                text::segmented_vector<int> t = ct_string;
                text::segmented_vector<int>_view const before = t(0, i);
                text::segmented_vector<int>_view const substr = t(i, j);
                text::segmented_vector<int>_view const after = t(j, t.size());

                text::segmented_vector<int> expected(before);
                expected.insert(expected.size(), final_cp, last);
                expected += after;

                t.replace(substr, final_cp, last);
                EXPECT_EQ(t, expected)
                    << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }

            {
                text::segmented_vector<int> t = ct_string;
                text::segmented_vector<int>_view const before = t(0, i);
                text::segmented_vector<int>_view const substr = t(i, j);
                text::segmented_vector<int>_view const after = t(j, t.size());

                text::segmented_vector<int> expected(before);
                expected.insert(expected.size(), first, last);
                expected += after;

                t.replace(substr, first, last);
                EXPECT_EQ(t, expected)
                    << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }
        }
    }
}

TEST(segmented_vector, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (int i = 0; i < 5000; ++i) {
        utf32_repeated.insert(utf32_repeated.end(), utf32, utf32 + 4);
    }
    auto const first =
        text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin());
    auto const last =
        text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.end());

    {
        text::segmented_vector<int> t("string");
        t.replace(t, first, last);
        text::segmented_vector<int> const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text::segmented_vector<int> t;
        t.replace(t, first, last);
        text::segmented_vector<int> const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}
#endif
