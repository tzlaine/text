#include <boost/text/collation_search.hpp>

#include <gtest/gtest.h>


using namespace boost::text;


collation_table const default_table = default_collation_table();

// TODO: Other tables too.

TEST(collation_search, default_)
{
    // TODO: This is just here to instantiate templates.
    std::array<uint32_t, 16> str = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    std::array<uint32_t, 4> substr = {{0, 0, 0, 0}};

    // TODO: These all crash!
#if 1
    {
        auto r = collation_search(str, substr, default_table);
        (void)r;
    }
#endif

#if 0
    {
        auto r = collation_search(
            str, make_default_collation_searcher(substr, default_table));
        (void)r;
    }

    {
        auto r = collation_search(
            str, make_boyer_moore_collation_searcher(substr, default_table));
        (void)r;
    }

    {
        auto r = collation_search(
            str,
            make_boyer_moore_horspool_collation_searcher(
                substr, default_table));
        (void)r;
    }
#endif
}
