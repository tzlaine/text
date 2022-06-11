// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/data/all.hpp>
#include <boost/text/detail/parser.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <map>


bool g_shifted = false;

namespace boost { namespace text {

    std::ostream & operator<<(std::ostream & os, collation_strength strength)
    {
        if (g_shifted)
            return os << "collation_strength::quaternary";

        switch (strength) {
        case collation_strength::primary:
            os << "collation_strength::primary";
            break;
        case collation_strength::secondary:
            os << "collation_strength::secondary";
            break;
        case collation_strength::tertiary:
            os << "collation_strength::tertiary";
            break;
        case collation_strength::quaternary:
            os << "collation_strength::quaternary";
            break;
        case collation_strength::identical:
            // identical is overloaded to mean "=", which must compare
            // quaternary-equal.
            os << "collation_strength::quaternary";
            break;
        }
        return os;
    }

    collation_strength prev(collation_strength strength)
    {
        assert(strength != collation_strength::primary);
        return collation_strength(static_cast<int>(strength) - 1);
    }

}}

using namespace boost::text;

int g_count = 0;
int g_test_subfile_count = 0;
int g_test_subtest_count = 0;
std::string g_this_test;

detail::cp_seq_t g_reset;
detail::cp_seq_t g_curr_reset;
bool g_before = false;
bool g_just_after_reset = false;

std::string g_curr_file;
std::string g_curr_tailoring;
std::string g_tailoring;
std::ofstream g_ofs;

std::map<int, int> const g_logical_positions{
    {detail::first_tertiary_ignorable,
     detail::initial_first_tertiary_ignorable},
    {detail::last_tertiary_ignorable, detail::initial_last_tertiary_ignorable},
    {detail::first_primary_ignorable, detail::initial_first_primary_ignorable},
    {detail::last_primary_ignorable, detail::initial_last_primary_ignorable},
    {detail::first_variable, detail::initial_first_variable},
    {detail::last_variable, detail::initial_last_variable},
    {detail::first_regular, detail::initial_first_regular},
    {detail::last_regular, detail::initial_last_regular},
    {detail::first_implicit, detail::initial_first_implicit},
};

void write_file_prefix(std::string const & this_test)
{
    string_view const header = R"(
// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Warning! This file is autogenerated.
#include <boost/text/collation_table.hpp>
#include <boost/text/collate.hpp>
#include <boost/text/data/all.hpp>

#ifndef LIMIT_TESTING_FOR_CI
#include <boost/text/save_load_table.hpp>

#include <boost/filesystem.hpp>
#endif

#include <gtest/gtest.h>

using namespace boost::text;

auto const error = [](std::string const & s) { std::cout << s; };
auto const warning = [](std::string const & s) {};
)";

    std::string const this_tailoring =
        std::string(g_curr_file) + "::" + g_curr_tailoring;
    std::string const table_name =
        std::string(g_curr_file) + "_" + g_curr_tailoring;

    g_ofs << header << R"Q(
collation_table make_save_load_table()
{
#ifdef LIMIT_TESTING_FOR_CI
    std::string const table_str(data::)Q" << this_tailoring << R"Q(_collation_tailoring());
    return tailored_collation_table(
        table_str,
        ")Q"
      << this_tailoring << R"Q(_collation_tailoring()", error, warning);
#else
    if (!exists(boost::filesystem::path(")Q" << table_name << R"Q(.table"))) {
        std::string const table_str(data::)Q" << this_tailoring << R"Q(_collation_tailoring());
        collation_table table = tailored_collation_table(
            table_str,
            ")Q"
          << this_tailoring << R"Q(_collation_tailoring()", error, warning);
        save_table(table, ")Q" << table_name << ".table." << g_test_subfile_count << R"Q(");
        boost::filesystem::rename(")Q" << table_name << ".table." << g_test_subfile_count << "\", \"" << table_name << ".table" << R"Q(");
    }
    return load_table(")Q" << table_name << R"Q(.table");
#endif
}
collation_table const & table()
{
    static collation_table retval = make_save_load_table();
    return retval;
}
)Q";
}

std::string vector_of(detail::cp_seq_t cps)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << "std::vector<uint32_t>";
    if (cps.size() == 1) {
        auto cp = cps[0];
        auto const it = g_logical_positions.find(cp);
        if (it != g_logical_positions.end())
            cp = it->second;
        ss << "(1, 0x" << std::setw(4) << cp << ")";
    } else {
        ss << "{";
        bool first = true;
        for (auto cp : cps){
            if (!first)
                ss << ", ";
            ss << "0x" << std::setw(4) << cp;
            first = false;
        }
        ss << "}";
    }
    return std::string(ss.str().c_str());
}

void print_rule_test(
    detail::cp_seq_t relation,
    collation_strength strength,
    detail::optional_cp_seq_t prefix,
    detail::optional_cp_seq_t extension)
{
    auto symbolic_ignorable = [](uint32_t cp) {
        return cp == detail::first_secondary_ignorable ||
               cp == detail::last_secondary_ignorable ||
               cp == detail::first_primary_ignorable;
    };
    if (std::any_of(
            g_curr_reset.begin(), g_curr_reset.end(), symbolic_ignorable) ||
        std::any_of(relation.begin(), relation.end(), symbolic_ignorable))
        return;

    auto curr_reset = g_curr_reset;
    if (prefix) {
        curr_reset.insert(curr_reset.begin(), relation.begin(), relation.end());
        relation.insert(relation.end(), prefix->begin(), prefix->end());
    }

    {
        if (extension) {
            curr_reset.insert(
                curr_reset.end(), extension->begin(), extension->end());
        }
        g_ofs << "    {\n"
              << "    // greater than (or equal to, for =) preceeding cps\n"
              << "    auto const res = " << vector_of(curr_reset) << ";\n"
              << "    auto const rel = " << vector_of(relation) << ";\n"
              << "    std::string const res_str = to_string(res);\n"
              << "    std::string const rel_str = to_string(rel);\n"
              << "    auto const res_view = as_utf32(res);\n"
              << "    auto const rel_view = as_utf32(rel);\n"
              << "    EXPECT_EQ(collate(\n"
              << "        res.begin(), res.end(),\n"
              << "        rel.begin(), rel.end(),\n"
              << "        table(), " << strength << "),\n"
              << "        "
              << (strength == collation_strength::identical ? 0 : -1) << ");\n"
              << "    EXPECT_EQ(collate(\n"
              << "        res_view.begin(), res_view.end(),\n"
              << "        rel_view.begin(), rel_view.end(),\n"
              << "        table(), " << strength << "),\n"
              << "        "
              << (strength == collation_strength::identical ? 0 : -1) << ");\n";
        if (collation_strength::primary < strength &&
            strength <= collation_strength::quaternary && !g_shifted) {
            g_ofs << "    // equal to preceeding cps at next-lower strength\n"
                  << "    EXPECT_EQ(collate(\n"
                  << "        res.begin(), res.end(),\n"
                  << "        rel.begin(), rel.end(),\n"
                  << "        table(), " << prev(strength) << "),\n"
                  << "        0);\n"
                  << "    // equal to preceeding cps at next-lower strength\n"
                  << "    EXPECT_EQ(collate(\n"
                  << "        res_view.begin(), res_view.end(),\n"
                  << "        rel_view.begin(), rel_view.end(),\n"
                  << "        table(), " << prev(strength) << "),\n"
                  << "        0);\n";
        }
        g_ofs << "    }\n";
    }

    if (g_before &&
        !std::any_of(g_reset.begin(), g_reset.end(), symbolic_ignorable)) {
        auto reset = g_reset;
        if (extension) {
            reset.insert(reset.end(), extension->begin(), extension->end());
        }
        g_ofs << "    {\n"
              << "    // before initial reset cps\n"
              << "    auto const res = " << vector_of(reset) << ";\n"
              << "    auto const rel = " << vector_of(relation) << ";\n"
              << "    std::string const res_str = to_string(res);\n"
              << "    std::string const rel_str = to_string(rel);\n"
              << "    auto const res_view = as_utf32(res);\n"
              << "    auto const rel_view = as_utf32(rel);\n"
              << "    EXPECT_EQ(collate(\n"
              << "        res.begin(), res.end(),\n"
              << "        rel.begin(), rel.end(),\n"
              << "        table(), collation_strength::quaternary),\n"
              << "        1);\n"
              << "    }\n"
              << "    EXPECT_EQ(collate(\n"
              << "        res_view.begin(), res_view.end(),\n"
              << "        rel_view.begin(), rel_view.end(),\n"
              << "        table(), collation_strength::quaternary),\n"
              << "        1);\n"
              << "    }\n";
    }
}

std::string new_test(bool first_test = false)
{
    std::string this_test = g_this_test;
    if (!first_test)
        g_ofs << "}\n\n";
    std::string const filename_prefix = "tailoring_rule_test_" + this_test;
    if (++g_test_subtest_count == 20) {
        g_test_subtest_count = 0;
        ++g_test_subfile_count;
        g_ofs.close();
        std::stringstream ss;
        ss << '_' << std::setfill('0') << std::setw(3) << g_test_subfile_count;
        g_this_test =
            std::string(g_curr_file) + "_" + g_curr_tailoring + ss.str().c_str();
        std::string filename = "tailoring_rule_test_" + g_this_test + ".cpp";
        g_ofs.open(filename.c_str());
        write_file_prefix(g_this_test);
    }
    std::stringstream ss;
    ss << '_' << std::setfill('0') << std::setw(3) << g_test_subtest_count;
    this_test += ss.str().c_str();
    g_ofs << "TEST(tailoring, " << this_test << ")\n{\n";
    return this_test;
}

detail::collation_tailoring_interface g_callbacks = {
    [](detail::cp_seq_t const & reset, bool before) {
        g_curr_reset = g_reset = reset;
        g_before = before;
        g_just_after_reset = true;
        ++g_count;
    },
    [](detail::relation_t const & rel) {
        if (50 < g_count) {
            g_count = 0;
            new_test();
        }
        if (!g_before) {
            print_rule_test(
                rel.cps_,
                static_cast<collation_strength>(rel.op_),
                rel.prefix_and_extension_.prefix_,
                rel.prefix_and_extension_.extension_);
        }
        g_curr_reset = rel.prefix_and_extension_.prefix_
                           ? *rel.prefix_and_extension_.prefix_
                           : rel.cps_;
        g_just_after_reset = false;
        ++g_count;
    },
    [](collation_strength strength) {},
    [](variable_weighting weighting) {
        g_shifted = weighting == variable_weighting::shifted;
    },
    [](l2_weight_order order) {},
    [](case_level) {},
    [](case_first) {},
    [](detail::cp_seq_t const & suppressions) {},
    [](std::vector<detail::reorder_group> const & reorder_groups) {},
    [](std::string const & s) { std::cout << s; },
    [](std::string const & s) {}};

void make_test()
{
    g_count = 0;
    g_test_subtest_count = 0;
    g_test_subfile_count = 0;
    g_this_test = std::string(g_curr_file) + "_" + g_curr_tailoring + "_000";
    g_shifted = false;
    std::string filename = "tailoring_rule_test_" + g_this_test + ".cpp";
    g_ofs.open(filename.c_str());
    write_file_prefix(g_this_test);
    new_test(true);
    detail::parse(
        g_tailoring.data(),
        g_tailoring.data() + g_tailoring.size(),
        g_callbacks,
        g_this_test);
    g_ofs << "}\n";
    g_ofs.close();
}

#include "tailoring_rules_tests_main.cpp"
