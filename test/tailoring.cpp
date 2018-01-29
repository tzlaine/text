#include <boost/text/collation_tailoring.hpp>
#include <boost/text/collate.hpp>
#include <boost/text/data/all.hpp>

#include <gtest/gtest.h>


using namespace boost;

namespace std {
    ostream & operator<<(ostream & os, std::array<uint32_t, 1> cp)
    {
        return os << hex << "0x" << cp[0] << dec;
    }
    ostream &
    operator<<(ostream & os, container::static_vector<uint32_t, 16> const & vec)
    {
        os << "{ " << hex;
        for (uint32_t cp : vec) {
            os << "0x" << cp << " ";
        }
        os << "}" << dec;
        return os;
    }
    ostream & operator<<(ostream & os, text::utf32_range const & r)
    {
        os << '"' << hex;
        for (uint32_t cp : r) {
            if (cp < 0x80)
                os << (char)cp;
            else if (cp <= 0xffff)
                os << "\\u" << setw(4) << setfill('0') << cp;
            else
                os << "\\U" << setw(8) << setfill('0') << cp;
        }
        os << '"' << dec;
        return os;
    }
}

// First two and last two of each reorder group, and a sampling of implicits.
constexpr std::array<uint32_t, 1> space[4] = {
    {{0x0009}}, {{0x000A}}, {{0x2007}}, {{0x202F}}};
constexpr std::array<uint32_t, 1> digit[4] = {
    {{0x09F4}}, {{0x09F5}}, {{0x32C8}}, {{0x3361}}};
constexpr std::array<uint32_t, 1> Latn[4] = {
    {{0x0061}}, {{0xFF41}}, {{0x02AC}}, {{0x02AD}}};
constexpr std::array<uint32_t, 1> Grek[4] = {
    {{0x03B1}}, {{0x1D6C2}}, {{0x03F8}}, {{0x03F7}}};
constexpr std::array<uint32_t, 1> Copt[4] = {
    {{0x2C81}}, {{0x2C80}}, {{0x2CE3}}, {{0x2CE2}}};
constexpr std::array<uint32_t, 1> Hani[4] = {
    {{0x2F00}}, {{0x3280}}, {{0x2F88F}}, {{0x2FA1D}}};

constexpr std::array<uint32_t, 1> implicit[4] = {
    {{0x2a700}}, {{0x2b740}}, {{0x2b820}}, {{0x2ebe0}}};

struct reordering_t
{
    text::string_view name_;
    std::array<uint32_t, 1> const * cps_;

    friend bool operator<(reordering_t lhs, reordering_t rhs)
    {
        return lhs.name_ < rhs.name_;
    }
};


TEST(tailoring, reordering)
{
    std::array<reordering_t, 5> reorderings{{
        {"space", space},
        {"digit", digit},
        {"Latn", Latn},
        {"Grek", Grek},
        //{"Copt", Copt}, // This works, but makes the test take way too long.
        {"Hani", Hani},
    }};

    std::sort(reorderings.begin(), reorderings.end());

    text::string reordering_str;
    std::vector<std::array<uint32_t, 1>> cps;
    do {
        reordering_str = "[reorder";
        cps.clear();
        for (auto reorder : reorderings) {
            reordering_str += " ";
            reordering_str += reorder.name_;
            cps.insert(cps.end(), reorder.cps_, reorder.cps_ + 4);
            if (reorder.name_ == "Hani")
                cps.insert(cps.end(), implicit, implicit + 4);
        }
        reordering_str += "]";

        text::tailored_collation_element_table const table =
            text::make_tailored_collation_element_table(
                reordering_str,
                "reorderings",
                [](text::string const & s) { std::cout << s; },
                [](text::string const & s) { std::cout << s; });

        for (int i = 0, end = (int)cps.size() - 1; i != end; ++i) {
            EXPECT_LE(
                text::collate(
                    cps[i],
                    cps[i + 1],
                    table,
                    text::collation_strength::primary,
                    text::variable_weighting::non_ignorable),
                0)
                << reordering_str << " " << cps[i] << " " << cps[i + 1];
        }
    } while (std::next_permutation(reorderings.begin(), reorderings.end()));
}

// TODO: Create a tailoring for each of the strings priovided by #include
// <boost/text/data/all.hpp> to make sure they're at least stable.

TEST(tailoring, de)
{
    // Looks like the default de collation is the default collation.

    int const cases = 12;

    std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
        {{0x47, 0x72, 0x00f6, 0x00df, 0x65},
         {0x61, 0x62, 0x63},
         {0x54, 0x00f6, 0x6e, 0x65},
         {0x54, 0x00f6, 0x6e, 0x65},
         {0x54, 0x00f6, 0x6e, 0x65},
         {0x61, 0x0308, 0x62, 0x63},
         {0x00e4, 0x62, 0x63},
         {0x00e4, 0x62, 0x63},
         {0x53, 0x74, 0x72, 0x61, 0x00df, 0x65},
         {0x65, 0x66, 0x67},
         {0x00e4, 0x62, 0x63},
         {0x53, 0x74, 0x72, 0x61, 0x00df, 0x65}}};

    std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
        {{0x47, 0x72, 0x6f, 0x73, 0x73, 0x69, 0x73, 0x74},
         {0x61, 0x0308, 0x62, 0x63},
         {0x54, 0x6f, 0x6e},
         {0x54, 0x6f, 0x64},
         {0x54, 0x6f, 0x66, 0x75},
         {0x41, 0x0308, 0x62, 0x63},
         {0x61, 0x0308, 0x62, 0x63},
         {0x61, 0x65, 0x62, 0x63},
         {0x53, 0x74, 0x72, 0x61, 0x73, 0x73, 0x65},
         {0x65, 0x66, 0x67},
         {0x61, 0x65, 0x62, 0x63},
         {0x53, 0x74, 0x72, 0x61, 0x73, 0x73, 0x65}}};

    std::array<int, cases> const primary_result = {
        {-1, 0, 1, 1, 1, 0, 0, -1, 0, 0, -1, 0}};

    std::array<int, cases> const tertiary_result = {
        {-1, -1, 1, 1, 1, -1, 0, -1, 1, 0, -1, 1}};

    for (int i = 0; i < cases; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                text::collation_strength::primary,
                text::variable_weighting::non_ignorable),
            primary_result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            tertiary_result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }
}

TEST(tailoring, en)
{
    // The standard English collation is just the default collation.

    {
        int const cases = 49;

        std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
            {{0x0061, 0x0062},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x002d,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0020,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x002d,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0048, 0x0065, 0x006c, 0x006c, 0x006f},
             {0x0041, 0x0042, 0x0043},
             {0x0061, 0x0062, 0x0063},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x002d,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x002d,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9},
             {0x00c4, 0x0042, 0x0308, 0x0043, 0x0308},
             {0x0061, 0x0308, 0x0062, 0x0063},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x0072, 0x006f, 0x006c, 0x0065, 0x0073},
             {0x0061, 0x0062, 0x0063},
             {0x0041},
             {0x0041},
             {0x0061, 0x0062},
             {0x0074,
              0x0063,
              0x006f,
              0x006d,
              0x0070,
              0x0061,
              0x0072,
              0x0065,
              0x0070,
              0x006c,
              0x0061,
              0x0069,
              0x006e},
             {0x0061, 0x0062},
             {0x0061, 0x0023, 0x0062},
             {0x0061, 0x0023, 0x0062},
             {0x0061, 0x0062, 0x0063},
             {0x0041, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x0061, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x0061, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00e6, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00e4, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0063, 0x0048, 0x0063},
             {0x0061, 0x0308, 0x0062, 0x0063},
             {0x0074, 0x0068, 0x0069, 0x0302, 0x0073},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x00e6, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x00e6, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9}}};

        std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
            {{0x0061, 0x0062, 0x0063},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x002d,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0062, 0x006c, 0x0061, 0x0063, 0x006b},
             {0x0068, 0x0065, 0x006c, 0x006c, 0x006f},
             {0x0041, 0x0042, 0x0043},
             {0x0041, 0x0042, 0x0043},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0062,
              0x0069,
              0x0072,
              0x0064,
              0x0073},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0062,
              0x0069,
              0x0072,
              0x0064,
              0x0073},
             {0x0062,
              0x006c,
              0x0061,
              0x0063,
              0x006b,
              0x0062,
              0x0069,
              0x0072,
              0x0064},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x00c4, 0x0042, 0x0308, 0x0043, 0x0308},
             {0x0041, 0x0308, 0x0062, 0x0063},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x0065},
             {0x0072, 0x006f, 0x0302, 0x006c, 0x0065},
             {0x0041, 0x00e1, 0x0063, 0x0064},
             {0x0041, 0x00e1, 0x0063, 0x0064},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0054,
              0x0043,
              0x006f,
              0x006d,
              0x0070,
              0x0061,
              0x0072,
              0x0065,
              0x0050,
              0x006c,
              0x0061,
              0x0069,
              0x006e},
             {0x0061, 0x0042, 0x0063},
             {0x0061, 0x0023, 0x0042},
             {0x0061, 0x0026, 0x0062},
             {0x0061, 0x0023, 0x0063},
             {0x0061, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00c4, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00e4, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00c4, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x00c4, 0x0062, 0x0063, 0x0064, 0x0061},
             {0x0061, 0x0062, 0x0023, 0x0063},
             {0x0061, 0x0062, 0x0063},
             {0x0061, 0x0062, 0x003d, 0x0063},
             {0x0061, 0x0062, 0x0064},
             {0x00e4, 0x0062, 0x0063},
             {0x0061, 0x0043, 0x0048, 0x0063},
             {0x00e4, 0x0062, 0x0063},
             {0x0074, 0x0068, 0x00ee, 0x0073},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9},
             {0x0061, 0x0042, 0x0043},
             {0x0061, 0x0062, 0x0064},
             {0x00e4, 0x0062, 0x0063},
             {0x0061, 0x00c6, 0x0063},
             {0x0061, 0x0042, 0x0064},
             {0x00e4, 0x0062, 0x0063},
             {0x0061, 0x00c6, 0x0063},
             {0x0061, 0x0042, 0x0064},
             {0x00e4, 0x0062, 0x0063},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065}}};

        std::array<int, cases> const result = {
            {-1, -1, -1, 1,  1,  0,  -1, -1, -1, -1, 1,  -1, 0,  -1, 1, 1, 1,
             -1, -1, -1, -1, -1, -1, 1,  1,  1,  -1, -1, 1,  -1, 1,  0, 1, -1,
             -1, -1, 0,  0,  0,  0,  -1, 0,  0,  -1, -1, 0,  -1, -1, -1}};

        for (int i = 38; i < 43; ++i) {
            EXPECT_EQ(
                text::collate(
                    lhs[i],
                    rhs[i],
                    text::collation_strength::primary,
                    text::variable_weighting::non_ignorable),
                result[i])
                << "CASE " << i << "\n"
                << lhs[i] << "\n"
                << rhs[i] << "\n";
        }
        for (int i = 43; i < 49; ++i) {
            EXPECT_EQ(
                text::collate(
                    lhs[i],
                    rhs[i],
                    text::collation_strength::secondary,
                    text::variable_weighting::non_ignorable),
                result[i])
                << "CASE " << i << "\n"
                << lhs[i] << "\n"
                << rhs[i] << "\n";
        }
        for (int i = 0; i < 38; ++i) {
            EXPECT_EQ(
                text::collate(
                    lhs[i],
                    rhs[i],
                    text::collation_strength::tertiary,
                    text::variable_weighting::non_ignorable),
                result[i])
                << "CASE " << i << "\n"
                << lhs[i] << "\n"
                << rhs[i] << "\n";
        }
    }

    {
        int const cases = 10;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            primary_less = {{{0x61},
                             {0x41},
                             {0x65},
                             {0x45},
                             {0x00e9},
                             {0x00e8},
                             {0x00ea},
                             {0x00eb},
                             {0x65, 0x61},
                             {0x78}}};

        for (int i = 0; i < cases; ++i) {
            for (int j = i + 1; j < cases; ++j) {
                EXPECT_EQ(
                    text::collate(
                        primary_less[i],
                        primary_less[j],
                        text::collation_strength::tertiary,
                        text::variable_weighting::non_ignorable),
                    -1)
                    << "CASE " << i << "\n"
                    << primary_less[i] << "\n"
                    << primary_less[j] << "\n";
            }
        }
    }

    {
        int const cases = 8;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            strings = {{{0x0061, 0x0065},
                        {0x00E6},
                        {0x00C6},
                        {0x0061, 0x0066},
                        {0x006F, 0x0065},
                        {0x0153},
                        {0x0152},
                        {0x006F, 0x0066}}};

        for (int i = 0; i < cases; ++i) {
            for (int j = 0; j < cases; ++j) {
                int expected = 0;
                if (i < j)
                    expected = -1;
                if (j < i)
                    expected = 1;
                EXPECT_EQ(
                    text::collate(
                        strings[i],
                        strings[j],
                        text::collation_strength::tertiary,
                        text::variable_weighting::non_ignorable),
                    expected)
                    << "CASE " << i << "\n"
                    << strings[i] << "\n"
                    << strings[j] << "\n";
            }
        }
    }

    {
        int const cases = 25;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            strings = {{{0x65, 0x65},
                        {0x65, 0x65, 0x0301},
                        {0x65, 0x65, 0x0301, 0x0300},
                        {0x65, 0x65, 0x0300},
                        {0x65, 0x65, 0x0300, 0x0301},
                        {0x65, 0x0301, 0x65},
                        {0x65, 0x0301, 0x65, 0x0301},
                        {0x65, 0x0301, 0x65, 0x0301, 0x0300},
                        {0x65, 0x0301, 0x65, 0x0300},
                        {0x65, 0x0301, 0x65, 0x0300, 0x0301},
                        {0x65, 0x0301, 0x0300, 0x65},
                        {0x65, 0x0301, 0x0300, 0x65, 0x0301},
                        {0x65, 0x0301, 0x0300, 0x65, 0x0301, 0x0300},
                        {0x65, 0x0301, 0x0300, 0x65, 0x0300},
                        {0x65, 0x0301, 0x0300, 0x65, 0x0300, 0x0301},
                        {0x65, 0x0300, 0x65},
                        {0x65, 0x0300, 0x65, 0x0301},
                        {0x65, 0x0300, 0x65, 0x0301, 0x0300},
                        {0x65, 0x0300, 0x65, 0x0300},
                        {0x65, 0x0300, 0x65, 0x0300, 0x0301},
                        {0x65, 0x0300, 0x0301, 0x65},
                        {0x65, 0x0300, 0x0301, 0x65, 0x0301},
                        {0x65, 0x0300, 0x0301, 0x65, 0x0301, 0x0300},
                        {0x65, 0x0300, 0x0301, 0x65, 0x0300},
                        {0x65, 0x0300, 0x0301, 0x65, 0x0300, 0x0301}}};

        for (int i = 0; i < cases; ++i) {
            for (int j = 0; j < cases; ++j) {
                int expected = 0;
                if (i < j)
                    expected = -1;
                if (j < i)
                    expected = 1;
                EXPECT_EQ(
                    text::collate(
                        strings[i],
                        strings[j],
                        text::collation_strength::secondary,
                        text::variable_weighting::non_ignorable),
                    expected)
                    << "CASE " << i << "\n"
                    << strings[i] << "\n"
                    << strings[j] << "\n";
            }
        }
    }
}

TEST(tailoring, es)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::es::standard_collation_tailoring(),
            "es::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    int const cases = 9;
    std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {{
        {0x61, 0x6c, 0x69, 0x61, 0x73},
        {0x45, 0x6c, 0x6c, 0x69, 0x6f, 0x74},
        {0x48, 0x65, 0x6c, 0x6c, 0x6f},
        {0x61, 0x63, 0x48, 0x63},
        {0x61, 0x63, 0x63},
        {0x61, 0x6c, 0x69, 0x61, 0x73},
        {0x61, 0x63, 0x48, 0x63},
        {0x61, 0x63, 0x63},
        {0x48, 0x65, 0x6c, 0x6c, 0x6f},
    }};

    std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {{
        {0x61, 0x6c, 0x6c, 0x69, 0x61, 0x73},
        {0x45, 0x6d, 0x69, 0x6f, 0x74},
        {0x68, 0x65, 0x6c, 0x6c, 0x4f},
        {0x61, 0x43, 0x48, 0x63},
        {0x61, 0x43, 0x48, 0x63},
        {0x61, 0x6c, 0x6c, 0x69, 0x61, 0x73},
        {0x61, 0x43, 0x48, 0x63},
        {0x61, 0x43, 0x48, 0x63},
        {0x68, 0x65, 0x6c, 0x6c, 0x4f},
    }};

    std::array<int, cases> const result = {{-1, -1, 1, -1, -1, -1, 0, -1, 0}};

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                table,
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }

    for (int i = 5; i < cases; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                table,
                text::collation_strength::primary,
                text::variable_weighting::non_ignorable),
            result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }
}

TEST(tailoring, fi)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::fi::standard_collation_tailoring(),
            "fi::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    int const cases = 5;
    std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
        {{0x77, 0x61, 0x74},
         {0x76, 0x61, 0x74},
         {0x61, 0x00FC, 0x62, 0x65, 0x63, 0x6b},
         {0x4c, 0x00E5, 0x76, 0x69},
         {0x77, 0x61, 0x74}}};

    std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
        {{0x76, 0x61, 0x74},
         {0x77, 0x61, 0x79},
         {0x61, 0x78, 0x62, 0x65, 0x63, 0x6b},
         {0x4c, 0x00E4, 0x77, 0x65},
         {0x76, 0x61, 0x74}}};

    std::array<int, cases> const tertiary_result = {{1, -1, 1, -1, 1}};

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                table,
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            tertiary_result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }

    EXPECT_EQ(
        text::collate(
            lhs[4],
            rhs[4],
            table,
            text::collation_strength::primary,
            text::variable_weighting::non_ignorable),
        tertiary_result[4])
        << "CASE " << 4 << "\n"
        << lhs[4] << "\n"
        << rhs[4] << "\n";
}

TEST(tailoring, fr)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::fr_CA::standard_collation_tailoring(),
            "fr_CA::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    {
        int const cases = 12;
        std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
            {{0x0061, 0x0062, 0x0063},
             {0x0043, 0x004f, 0x0054, 0x0045},
             {0x0063, 0x006f, 0x002d, 0x006f, 0x0070},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x0048, 0x0065, 0x006c, 0x006c, 0x006f},
             {0x01f1},
             {0xfb00},
             {0x01fa},
             {0x0101}}};

        std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
            {{0x0041, 0x0042, 0x0043},
             {0x0063, 0x00f4, 0x0074, 0x0065},
             {0x0043, 0x004f, 0x004f, 0x0050},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9},
             {0x0070, 0x00e9, 0x0063, 0x0068, 0x00e9},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065},
             {0x0070, 0x00ea, 0x0063, 0x0068, 0x0065, 0x0072},
             {0x0068, 0x0065, 0x006c, 0x006c, 0x004f},
             {0x01ee},
             {0x25ca},
             {0x00e0},
             {0x01df}}};

        std::array<int, cases> const tertiary_result = {
            {-1, -1, -1, -1, 1, 1, -1, 1, -1, 1, -1, -1}};

        for (int i = 0; i < cases; ++i) {
            EXPECT_EQ(
                text::collate(
                    lhs[i],
                    rhs[i],
                    table,
                    text::collation_strength::tertiary,
                    text::variable_weighting::shifted,
                    text::l2_weight_order::backward),
                tertiary_result[i])
                << "CASE " << i << "\n"
                << lhs[i] << "\n"
                << rhs[i] << "\n";
        }
    }

    {
        int const cases = 10;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            tertiary_less = {{{0x0061},
                              {0x0041},
                              {0x0065},
                              {0x0045},
                              {0x00e9},
                              {0x00e8},
                              {0x00ea},
                              {0x00eb},
                              {0x0065, 0x0061},
                              {0x0078}}};

        for (int i = 0; i < cases - 1; ++i) {
            for (int j = i + 1; j < cases; ++j) {
                EXPECT_EQ(
                    text::collate(
                        tertiary_less[i],
                        tertiary_less[j],
                        table,
                        text::collation_strength::tertiary,
                        text::variable_weighting::non_ignorable),
                    -1)
                    << "CASE " << i << "\n"
                    << tertiary_less[i] << "\n"
                    << tertiary_less[j] << "\n";
            }
        }
    }


    {
        int const cases = 25;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            strings = {{{0x0065, 0x0065},
                        {0x0065, 0x0301, 0x0065},
                        {0x0065, 0x0300, 0x0301, 0x0065},
                        {0x0065, 0x0300, 0x0065},
                        {0x0065, 0x0301, 0x0300, 0x0065},
                        {0x0065, 0x0065, 0x0301},
                        {0x0065, 0x0301, 0x0065, 0x0301},
                        {0x0065, 0x0300, 0x0301, 0x0065, 0x0301},
                        {0x0065, 0x0300, 0x0065, 0x0301},
                        {0x0065, 0x0301, 0x0300, 0x0065, 0x0301},
                        {0x0065, 0x0065, 0x0300, 0x0301},
                        {0x0065, 0x0301, 0x0065, 0x0300, 0x0301},
                        {0x0065, 0x0300, 0x0301, 0x0065, 0x0300, 0x0301},
                        {0x0065, 0x0300, 0x0065, 0x0300, 0x0301},
                        {0x0065, 0x0301, 0x0300, 0x0065, 0x0300, 0x0301},
                        {0x0065, 0x0065, 0x0300},
                        {0x0065, 0x0301, 0x0065, 0x0300},
                        {0x0065, 0x0300, 0x0301, 0x0065, 0x0300},
                        {0x0065, 0x0300, 0x0065, 0x0300},
                        {0x0065, 0x0301, 0x0300, 0x0065, 0x0300},
                        {0x0065, 0x0065, 0x0301, 0x0300},
                        {0x0065, 0x0301, 0x0065, 0x0301, 0x0300},
                        {0x0065, 0x0300, 0x0301, 0x0065, 0x0301, 0x0300},
                        {0x0065, 0x0300, 0x0065, 0x0301, 0x0300},
                        {0x0065, 0x0301, 0x0300, 0x0065, 0x0301, 0x0300}}};

        for (int i = 0; i < cases; ++i) {
            for (int j = 0; j < cases; ++j) {
                int expected = 0;
                if (i < j)
                    expected = -1;
                if (j < i)
                    expected = 1;
                EXPECT_EQ(
                    text::collate(
                        strings[i],
                        strings[j],
                        text::collation_strength::secondary,
                        text::variable_weighting::shifted,
                        text::l2_weight_order::backward),
                    expected)
                    << "CASE " << i << "\n"
                    << strings[i] << "\n"
                    << strings[j] << "\n";
            }
        }
    }
}

TEST(tailoring, ja)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::ja::standard_collation_tailoring(),
            "ja::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    {
        int const cases = 6;
        std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
            {{0xff9e},
             {0x3042},
             {0x30a2},
             {0x3042, 0x3042},
             {0x30a2, 0x30fc},
             {0x30a2, 0x30fc, 0x30c8}}};

        std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
            {{0xff9f},
             {0x30a2},
             {0x3042, 0x3042},
             {0x30a2, 0x30fc},
             {0x30a2, 0x30fc, 0x30c8},
             {0x3042, 0x3042, 0x3068}}};

        std::array<int, cases> const tertiary_result = {{-1, 0, -1, 1, -1, -1}};

        for (int i = 0; i < cases; ++i) {
            // TODO: [caseLevel on]
            EXPECT_EQ(
                text::collate(
                    lhs[i],
                    rhs[i],
                    table,
                    text::collation_strength::tertiary,
                    text::variable_weighting::non_ignorable),
                tertiary_result[i])
                << "CASE " << i << "\n"
                << lhs[i] << "\n"
                << rhs[i] << "\n";
        }
    }

    {
        int const cases = 4;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            primary_less = {
                {{0x30ab}, {0x30ab, 0x30ad}, {0x30ad}, {0x30ad, 0x30ad}}};

        for (int i = 0; i < cases - 1; ++i) {
            EXPECT_EQ(
                text::collate(
                    primary_less[i],
                    primary_less[i + 1],
                    table,
                    text::collation_strength::primary,
                    text::variable_weighting::non_ignorable),
                -1)
                << "CASE " << i << "\n"
                << primary_less[i] << "\n"
                << primary_less[i + 1] << "\n";
        }
    }

    {
        int const cases = 4;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            secondary_less = {{{0x30cf, 0x30ab},
                               {0x30d0, 0x30ab},
                               {0x30cf, 0x30ad},
                               {0x30d0, 0x30ad}}};

        for (int i = 0; i < cases - 1; ++i) {
            EXPECT_EQ(
                text::collate(
                    secondary_less[i],
                    secondary_less[i + 1],
                    table,
                    text::collation_strength::secondary,
                    text::variable_weighting::non_ignorable),
                -1)
                << "CASE " << i << "\n"
                << secondary_less[i] << "\n"
                << secondary_less[i + 1] << "\n";
        }
    }

    {
        int const cases = 4;
        std::array<container::static_vector<uint32_t, 16>, cases> const
            tertiary_less = {{{0x30c3, 0x30cf},
                              {0x30c4, 0x30cf},
                              {0x30c3, 0x30d0},
                              {0x30c4, 0x30d0}}};

        for (int i = 0; i < cases - 1; ++i) {
            // TODO: [caseLevel on]
            EXPECT_EQ(
                text::collate(
                    tertiary_less[i],
                    tertiary_less[i + 1],
                    table,
                    text::collation_strength::tertiary,
                    text::variable_weighting::non_ignorable),
                -1)
                << "CASE " << i << "\n"
                << tertiary_less[i] << "\n"
                << tertiary_less[i + 1] << "\n";
        }
    }

    {
        int const cases = 4;
        // Kataga and Hiragana
        std::array<container::static_vector<uint32_t, 16>, cases> const
            quaternary_less = {{{0x3042, 0x30c3},
                                {0x30a2, 0x30c3},
                                {0x3042, 0x30c4},
                                {0x30a2, 0x30c4}}};

        for (int i = 0; i < cases - 1; ++i) {
            // TODO: [caseLevel on]
            EXPECT_EQ(
                text::collate(
                    quaternary_less[i],
                    quaternary_less[i + 1],
                    table,
                    text::collation_strength::quaternary,
                    text::variable_weighting::non_ignorable),
                -1)
                << "CASE " << i << "\n"
                << quaternary_less[i] << "\n"
                << quaternary_less[i + 1] << "\n";
        }
    }

    {
        int const cases = 8;
        // Chooon and Kigoo
        std::array<container::static_vector<uint32_t, 16>, cases> const
            quaternary_less = {{{0x30AB, 0x30FC, 0x3042},
                                {0x30AB, 0x30FC, 0x30A2},
                                {0x30AB, 0x30A4, 0x3042},
                                {0x30AB, 0x30A4, 0x30A2},
                                {0x30AD, 0x30FC, 0x3042},
                                {0x30AD, 0x30FC, 0x30A2},
                                {0x3042, 0x30A4, 0x3042},
                                {0x30A2, 0x30A4, 0x30A2}}};

        for (int i = 0; i < cases - 1; ++i) {
            // TODO: Needs a fix
            if (i == 3 || i == 5)
                continue;
            // TODO: [caseLevel on]
            EXPECT_EQ(
                text::collate(
                    quaternary_less[i],
                    quaternary_less[i + 1],
                    table,
                    text::collation_strength::quaternary,
                    text::variable_weighting::non_ignorable),
                -1)
                << "CASE " << i << "\n"
                << quaternary_less[i] << "\n"
                << quaternary_less[i + 1] << "\n";
        }
    }
}

TEST(tailoring, th)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::th::standard_collation_tailoring(),
            "th::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    // TODO: Read riwords.txt, check that each line collates as <= the next
    // (tertiary).

    {
        int const cases = 13;
        std::array<text::string_view, cases> const lhs = {
            {"\u0e01",
             "\u0e01\u0e32",
             "\u0e01\u0e32",
             "\u0e01\u0e32\u0e01\u0e49\u0e32",
             "\u0e01\u0e32",
             "\u0e01\u0e32-",
             "\u0e01\u0e32",
             "\u0e01\u0e32\u0e46",
             "\u0e24\u0e29\u0e35",
             "\u0e26\u0e29\u0e35",
             "\u0e40\u0e01\u0e2d",
             "\u0e01\u0e32\u0e01\u0e48\u0e32",
             "\u0e01.\u0e01."}};

        std::array<text::string_view, cases> const rhs = {
            {"\u0e01\u0e01",
             "\u0e01\u0e49\u0e32",
             "\u0e01\u0e32\u0e4c",
             "\u0e01\u0e48\u0e32\u0e01\u0e49\u0e32",
             "\u0e01\u0e32-",
             "\u0e01\u0e32\u0e01\u0e32",
             "\u0e01\u0e32\u0e46",
             "\u0e01\u0e32\u0e01\u0e32",
             "\u0e24\u0e45\u0e29\u0e35",
             "\u0e26\u0e45\u0e29\u0e35",
             "\u0e40\u0e01\u0e34",
             "\u0e01\u0e49\u0e32\u0e01\u0e32",
             "\u0e01\u0e32"}};

        std::array<int, cases> const tertiary_result = {
            {-1, -1, -1, -1, 0, -1, 0, -1, -1, -1, -1, -1, -1}};

        for (int i = 0; i < cases; ++i) {
            EXPECT_EQ(
                text::collate(
                    text::utf32_range(lhs[i]),
                    text::utf32_range(rhs[i]),
                    table,
                    text::collation_strength::tertiary,
                    text::variable_weighting::non_ignorable),
                tertiary_result[i])
                << "CASE " << i << "\n"
                << text::utf32_range(lhs[i]) << "\n"
                << text::utf32_range(rhs[i]) << "\n";
        }
    }

    {
        int const cases = 26;
        std::array<text::string_view, cases> const lhs = {
            {"\u0E41c\u0301",          "\u0E41\U0001D7CE",
             "\u0E41\U0001D15F",       "\u0E41\U0002F802",
             "\u0E41\u0301",           "\u0E41\u0301\u0316",
             "\u0e24\u0e41",           "\u0e3f\u0e3f\u0e24\u0e41",
             "abc\u0E41c\u0301",       "abc\u0E41\U0001D000",
             "abc\u0E41\U0001D15F",    "abc\u0E41\U0002F802",
             "abc\u0E41\u0301",        "abc\u0E41\u0301\u0316",
             "\u0E41c\u0301abc",       "\u0E41\U0001D000abc",
             "\u0E41\U0001D15Fabc",    "\u0E41\U0002F802abc",
             "\u0E41\u0301abc",        "\u0E41\u0301\u0316abc",
             "abc\u0E41c\u0301abc",    "abc\u0E41\U0001D000abc",
             "abc\u0E41\U0001D15Fabc", "abc\u0E41\U0002F802abc",
             "abc\u0E41\u0301abc",     "abc\u0E41\u0301\u0316abc"}};

        std::array<text::string_view, cases> const rhs = {
            {"\u0E41\u0107",
             "\u0E41\U0001D7CF",
             "\u0E41\U0001D158\U0001D165",
             "\u0E41\u4E41",
             "\u0E41\u0301",
             "\u0E41\u0316\u0301",
             "\u0e41\u0e24",
             "\u0e3f\u0e3f\u0e41\u0e24",
             "abc\u0E41\u0107",
             "abc\u0E41\U0001D001",
             "abc\u0E41\U0001D158\U0001D165",
             "abc\u0E41\u4E41",
             "abc\u0E41\u0301",
             "abc\u0E41\u0316\u0301",
             "\u0E41\u0107abc",
             "\u0E41\U0001D001abc",
             "\u0E41\U0001D158\U0001D165abc",
             "\u0E41\u4E41abc",
             "\u0E41\u0301abc",
             "\u0E41\u0316\u0301abc",
             "abc\u0E41\u0107abc",
             "abc\u0E41\U0001D001abc",
             "abc\u0E41\U0001D158\U0001D165abc",
             "abc\u0E41\u4E41abc",
             "abc\u0E41\u0301abc",
             "abc\u0E41\u0316\u0301abc"}};

        // TODO: Changed cases 5, 13, 19, and 25 from 0 to -1, since they have
        // secondary differences.  Need to understand if this is the right thing
        // to do.
        std::array<int, cases> const secondary_result = {
            {0,  -1, 0,  0, 0, -1, 0,  0, 0,  -1, 0, 0, 0,
             -1, 0,  -1, 0, 0, 0,  -1, 0, -1, 0,  0, 0, -1}};

        for (int i = 0; i < cases; ++i) {
            EXPECT_EQ(
                text::collate(
                    text::utf32_range(lhs[i]),
                    text::utf32_range(rhs[i]),
                    table,
                    text::collation_strength::secondary,
                    text::variable_weighting::non_ignorable),
                secondary_result[i])
                << "CASE " << i << "\n"
                << text::utf32_range(lhs[i]) << "\n"
                << text::utf32_range(rhs[i]) << "\n";
        }
    }

    {
        text::tailored_collation_element_table const custom_table =
            text::make_tailored_collation_element_table(
                "& c < ab",
                "custom-tailoring",
                [](text::string const & s) { std::cout << s; },
                [](text::string const & s) { std::cout << s; });

        EXPECT_EQ(
            text::collate(
                text::string("\u0e41ab"),
                text::string("\u0e41c"),
                custom_table,
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            1);
    }
}

TEST(tailoring, tr)
{
    text::tailored_collation_element_table const table =
        text::make_tailored_collation_element_table(
            text::data::tr::standard_collation_tailoring(),
            "tr::standard_collation_tailoring()",
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; });

    int const cases = 11;
    std::array<container::static_vector<uint32_t, 16>, cases> const lhs = {
        {{0x73, 0x0327},
         {0x76, 0x00e4, 0x74},
         {0x6f, 0x6c, 0x64},
         {0x00fc, 0x6f, 0x69, 0x64},
         {0x68, 0x011e, 0x61, 0x6c, 0x74},
         {0x73, 0x74, 0x72, 0x65, 0x73, 0x015e},
         {0x76, 0x6f, 0x0131, 0x64},
         {0x69, 0x64, 0x65, 0x61},
         {0x00fc, 0x6f, 0x69, 0x64},
         {0x76, 0x6f, 0x0131, 0x64},
         {0x69, 0x64, 0x65, 0x61}}};

    std::array<container::static_vector<uint32_t, 16>, cases> const rhs = {
        {{0x75, 0x0308},
         {0x76, 0x62, 0x74},
         {0x00d6, 0x61, 0x79},
         {0x76, 0x6f, 0x69, 0x64},
         {0x68, 0x61, 0x6c, 0x74},
         {0x015e, 0x74, 0x72, 0x65, 0x015e, 0x73},
         {0x76, 0x6f, 0x69, 0x64},
         {0x49, 0x64, 0x65, 0x61},
         {0x76, 0x6f, 0x69, 0x64},
         {0x76, 0x6f, 0x69, 0x64},
         {0x49, 0x64, 0x65, 0x61}}};

    std::array<int, cases> const tertiary_result = {
        {-1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1}};

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                table,
                text::collation_strength::tertiary,
                text::variable_weighting::non_ignorable),
            tertiary_result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }
    for (int i = 8; i < cases; ++i) {
        EXPECT_EQ(
            text::collate(
                lhs[i],
                rhs[i],
                table,
                text::collation_strength::primary,
                text::variable_weighting::non_ignorable),
            tertiary_result[i])
            << "CASE " << i << "\n"
            << lhs[i] << "\n"
            << rhs[i] << "\n";
    }
}
