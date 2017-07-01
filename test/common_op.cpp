#include <boost/text/rope.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(common_operations, test_operator_plus)
{
    text::text_view const tv("tv");
    text::repeated_text_view const rtv(tv, 3);
    text::text const t("t");
    text::rope const r("r");
    text::rope_view const rv(r);

    text::rope result;

    EXPECT_EQ((result = tv + tv), "tvtv");
    EXPECT_EQ((result = tv + rtv), "tvtvtvtv");
    EXPECT_EQ((result = tv + t), "tvt");
    EXPECT_EQ((result = tv + std::move(t)), "tvt");
    EXPECT_EQ((result = tv + r), "tvr");
    EXPECT_EQ((result = tv + std::move(r)), "tvr");
    EXPECT_EQ((result = tv + rv), "tvr");

    EXPECT_EQ((result = rtv + tv), "tvtvtvtv");
    EXPECT_EQ((result = rtv + rtv), "tvtvtvtvtvtv");
    EXPECT_EQ((result = rtv + t), "tvtvtvt");
    EXPECT_EQ((result = rtv + std::move(t)), "tvtvtvt");
    EXPECT_EQ((result = rtv + r), "tvtvtvr");
    EXPECT_EQ((result = rtv + std::move(r)), "tvtvtvr");
    EXPECT_EQ((result = rtv + rv), "tvtvtvr");

    EXPECT_EQ((result = t + tv), "ttv");
    EXPECT_EQ((result = t + rtv), "ttvtvtv");
    EXPECT_EQ((result = t + t), "tt");
    EXPECT_EQ((result = t + std::move(t)), "tt");
    EXPECT_EQ((result = t + r), "tr");
    EXPECT_EQ((result = t + std::move(r)), "tr");
    EXPECT_EQ((result = t + rv), "tr");

    EXPECT_EQ((result = std::move(t) + tv), "ttv");
    EXPECT_EQ((result = std::move(t) + rtv), "ttvtvtv");
    EXPECT_EQ((result = std::move(t) + t), "tt");
    EXPECT_EQ((result = std::move(t) + std::move(t)), "tt");
    EXPECT_EQ((result = std::move(t) + r), "tr");
    EXPECT_EQ((result = std::move(t) + std::move(r)), "tr");
    EXPECT_EQ((result = std::move(t) + rv), "tr");

    EXPECT_EQ((result = r + tv), "rtv");
    EXPECT_EQ((result = r + rtv), "rtvtvtv");
    EXPECT_EQ((result = r + t), "rt");
    EXPECT_EQ((result = r + std::move(t)), "rt");
    EXPECT_EQ((result = r + r), "rr");
    EXPECT_EQ((result = r + std::move(r)), "rr");
    EXPECT_EQ((result = r + rv), "rr");

    EXPECT_EQ((result = std::move(r) + tv), "rtv");
    EXPECT_EQ((result = std::move(r) + rtv), "rtvtvtv");
    EXPECT_EQ((result = std::move(r) + t), "rt");
    EXPECT_EQ((result = std::move(r) + std::move(t)), "rt");
    EXPECT_EQ((result = std::move(r) + r), "rr");
    EXPECT_EQ((result = std::move(r) + std::move(r)), "rr");
    EXPECT_EQ((result = std::move(r) + rv), "rr");

    EXPECT_EQ((result = rv + tv), "rtv");
    EXPECT_EQ((result = rv + rtv), "rtvtvtv");
    EXPECT_EQ((result = rv + t), "rt");
    EXPECT_EQ((result = rv + std::move(t)), "rt");
    EXPECT_EQ((result = rv + r), "rr");
    EXPECT_EQ((result = rv + std::move(r)), "rr");
    EXPECT_EQ((result = rv + rv), "rr");
}

TEST(common_operations, test_operator_plus_assign)
{
    {
        text::text_view const tv("tv");
        text::repeated_text_view const rtv(tv, 3);
        text::text const t("t");
        text::rope const r("r");
        text::rope_view const rv(r);

        text::text result_t;

        EXPECT_EQ((result_t += tv), "tv");
        EXPECT_EQ((result_t += rtv), "tvtvtvtv");
        EXPECT_EQ((result_t += t), "tvtvtvtvt");
        EXPECT_EQ((result_t += std::move(t)), "tvtvtvtvtt");
        EXPECT_EQ((result_t += r), "tvtvtvtvttr");
        EXPECT_EQ((result_t += std::move(r)), "tvtvtvtvttrr");
        EXPECT_EQ((result_t += rv), "tvtvtvtvttrrr");
    }

    {
        text::text_view const tv("tv");
        text::repeated_text_view const rtv(tv, 3);
        text::text const t("t");
        text::rope const r("r");
        text::rope_view const rv(r);

        text::rope result_r;

        EXPECT_EQ((result_r += tv), "tv");
        EXPECT_EQ((result_r += rtv), "tvtvtvtv");
        EXPECT_EQ((result_r += t), "tvtvtvtvt");
        EXPECT_EQ((result_r += std::move(t)), "tvtvtvtvtt");
        EXPECT_EQ((result_r += r), "tvtvtvtvttr");
        EXPECT_EQ((result_r += std::move(r)), "tvtvtvtvttrr");
        EXPECT_EQ((result_r += rv), "tvtvtvtvttrrr");
    }
}

TEST(common_operations, copy_assign_broken_encoding_no_throw)
{
    // Unicode 9, 3.9/D90-D92
    // uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char const utf8[] = {0x4d, char(0xd0), char(0xb0), char(0xe4), char(0xba), char(0x8c), char(0xf0), char(0x90), char(0x8c), char(0x82)};

    text::text_view const tv(utf8, sizeof(utf8) - 1, text::utf8::unchecked);
    text::repeated_text_view const rtv(tv, 3);
    text::text const t(tv);
    text::rope const r(tv);

    EXPECT_NO_THROW(text::rope_view(tv));
    EXPECT_NO_THROW(text::rope_view(rtv));
    EXPECT_NO_THROW(text::rope_view(t));

    text::rope_view const rv(r);

    EXPECT_NO_THROW(text::text_view(tv));
    EXPECT_NO_THROW(text::text_view(t));
    EXPECT_NO_THROW(text::text_view(text::text(t)));

    EXPECT_NO_THROW(text::repeated_text_view(tv, 2));
    EXPECT_NO_THROW(text::repeated_text_view(rtv));

    EXPECT_NO_THROW(text::text(tv));
    EXPECT_NO_THROW(text::text(rtv));
    EXPECT_NO_THROW(text::text(t));
    EXPECT_NO_THROW(text::text(text::text(t)));

    EXPECT_NO_THROW(text::rope(tv));
    EXPECT_NO_THROW(text::rope(rtv));
    EXPECT_NO_THROW(text::rope(t));
    EXPECT_NO_THROW(text::rope(text::text(t)));
    EXPECT_NO_THROW(text::rope(r));
    EXPECT_NO_THROW(text::rope(text::rope(r)));
    EXPECT_NO_THROW(text::rope(rv));

    EXPECT_NO_THROW(text::rope_view(tv));
    EXPECT_NO_THROW(text::rope_view(rtv));
    EXPECT_NO_THROW(text::rope_view(t));
    EXPECT_NO_THROW(text::rope_view(text::text(t)));
    EXPECT_NO_THROW(text::rope_view(r));
    EXPECT_NO_THROW(text::rope_view(rv));

    EXPECT_NO_THROW(text::text_view() = tv);
    EXPECT_NO_THROW(text::text_view() = t);
    EXPECT_NO_THROW(text::text_view() = text::text(t));

    EXPECT_NO_THROW(text::repeated_text_view() = rtv);

    EXPECT_NO_THROW(text::text() = tv);
    EXPECT_NO_THROW(text::text() = rtv);
    EXPECT_NO_THROW(text::text() = t);
    EXPECT_NO_THROW(text::text() = text::text(t));

    EXPECT_NO_THROW(text::rope() = tv);
    EXPECT_NO_THROW(text::rope() = rtv);
    EXPECT_NO_THROW(text::rope() = t);
    EXPECT_NO_THROW(text::rope() = text::text(t));
    EXPECT_NO_THROW(text::rope() = r);
    EXPECT_NO_THROW(text::rope() = text::rope(r));
    EXPECT_NO_THROW(text::rope() = rv);

    EXPECT_NO_THROW(text::rope_view() = tv);
    EXPECT_NO_THROW(text::rope_view() = rtv);
    EXPECT_NO_THROW(text::rope_view() = t);
    EXPECT_NO_THROW(text::rope_view() = text::text(t));
    EXPECT_NO_THROW(text::rope_view() = r);
    EXPECT_NO_THROW(text::rope_view() = rv);
}
