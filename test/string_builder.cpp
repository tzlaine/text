// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_builder.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(static_string_builder, test_empty)
{
    {
        text::static_string_builder builder;
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "");
    }
    {
        text::string const result = text::static_string_builder().to_string();
        EXPECT_EQ(result, "");
    }
}

TEST(static_string_builder, test_one)
{
    {
        text::static_string_builder builder;
        auto builder_1 = builder + "text";
        text::string const result = builder_1.to_string();
        EXPECT_EQ(result, "text");
    }
    {
        text::static_string_builder builder;
        text::string const result = (builder + "text").to_string();
        EXPECT_EQ(result, "text");
    }
    {
        text::string const result =
            text::static_string_builder("text").to_string();
        EXPECT_EQ(result, "text");
    }
}

TEST(static_string_builder, test_many)
{
    {
        text::static_string_builder builder;
        auto builder_1 = builder + "some";
        char const * space = " ";
        auto builder_2 = builder_1 + space;
        auto builder_3 = builder_2 + "text";
        text::string const result = builder_3.to_string();
        EXPECT_EQ(result, "some text");
    }
    {
        text::static_string_builder builder;
        text::string const result =
            (builder + "some" + " " + "text").to_string();
        EXPECT_EQ(result, "some text");
    }
    {
        text::string const result =
            (text::static_string_builder("some") + " " + "text").to_string();
        EXPECT_EQ(result, "some text");
    }
}



TEST(static_string_builder, test_empty_constexpr)
{
    {
        constexpr text::static_string_builder builder;
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "");
    }
    {
        text::string const result = text::static_string_builder().to_string();
        EXPECT_EQ(result, "");
    }
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

TEST(static_string_builder, test_one_constexpr)
{
    {
        constexpr text::static_string_builder builder;
        constexpr auto builder_1 = builder + "text";
        text::string const result = builder_1.to_string();
        EXPECT_EQ(result, "text");
    }
    {
        constexpr text::static_string_builder builder;
        text::string const result = (builder + "text").to_string();
        EXPECT_EQ(result, "text");
    }
    {
        text::string const result =
            text::static_string_builder("text").to_string();
        EXPECT_EQ(result, "text");
    }
}

TEST(static_string_builder, test_many_constexpr)
{
    {
        constexpr text::static_string_builder builder;
        constexpr auto builder_1 = builder + "some";
        constexpr char const * space = " ";
        constexpr auto builder_2 = builder_1 + space;
        constexpr auto builder_3 = builder_2 + "text";
        text::string const result = builder_3.to_string();
        EXPECT_EQ(result, "some text");
    }
    {
        constexpr text::static_string_builder builder;
        text::string const result =
            (builder + "some" + " " + "text").to_string();
        EXPECT_EQ(result, "some text");
    }
    {
        text::string const result =
            (text::static_string_builder("some") + " " + "text").to_string();
        EXPECT_EQ(result, "some text");
    }
}

#endif



TEST(string_builder, test_empty)
{
    {
        text::string_builder builder;
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "");
    }
    {
        text::string const result = text::string_builder().to_string();
        EXPECT_EQ(result, "");
    }
}

TEST(string_builder, test_one)
{
    {
        text::string_builder builder;
        builder += "text";
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "text");
    }
    {
        text::string_builder builder;
        text::string const result = (builder += "text").to_string();
        EXPECT_EQ(result, "text");
    }
    {
        text::string const result =
            text::string_builder("text").to_string();
        EXPECT_EQ(result, "text");
    }
}

TEST(string_builder, test_many)
{
    {
        text::string_builder builder;
        builder += "some";
        builder += " ";
        builder += "text";
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "some text");
    }
    {
        text::string_builder builder("some");
        builder += " ";
        builder += "text";
        text::string const result = builder.to_string();
        EXPECT_EQ(result, "some text");
    }
}
