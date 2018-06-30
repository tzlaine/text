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
    auto it = collation_search(str, substr, default_table);
    (void)it;
}
