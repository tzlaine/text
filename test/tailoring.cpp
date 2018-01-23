#include <boost/text/collation_tailoring.hpp>
#include <boost/text/collate.hpp>
#include <boost/text/data/all.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(tailoring, ja)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::ja::standard_collation_tailoring(),
            "ja::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    {
        uint32_t const cps_0[] = {0xff9e};
        uint32_t const cps_1[] = {0xff9f};
        EXPECT_EQ(
            text::collate(
                std::begin(cps_0),
                std::end(cps_0),
                std::begin(cps_1),
                std::end(cps_1),
                table,
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            -1);
    }
}
