#include <boost/text/case_mapping.hpp>

#include <boost/text/utf8.hpp>

#include <gtest/gtest.h>


using namespace boost::text;


TEST(case_mapping, TODO)
{
    // TODO: Only here for instantiation. Remove once other tests exist.
    {
        uint32_t const cps[] = {1, 2};
        std::vector<uint32_t> vec;
        auto out = to_title(cps, std::back_inserter(vec));
    }

    // TODO: Same here.
    {
        char const cus[] = {1, 2, 0};
        std::vector<uint32_t> vec;
        auto first = utf8::make_to_utf32_iterator(cus, cus, cus + 3);
        auto out = to_title(
            first, first, utf8::null_sentinel{}, std::back_inserter(vec));
    }

    // TODO: Tests.
}
