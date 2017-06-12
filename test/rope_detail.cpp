#include <boost/text/rope.hpp>

#include <gtest/gtest.h>


using boost::text::text;
using boost::text::text_view;
using boost::text::repeated_text_view;
using namespace boost::text::detail;

TEST(rope_detail, test_node_ptr)
{
    {
        node_ptr p0(new interior_node_t);
        node_ptr p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_interior()->refs_, 2);
        EXPECT_EQ(p0.as_interior()->leaf_, false);
        EXPECT_EQ(p0.as_interior()->keys_.size(), 0u);
        EXPECT_EQ(p0.as_interior()->children_.size(), 0u);

        EXPECT_EQ(size(p0.get()), 0);

        (void)children(p0);
        (void)keys(p0);

        EXPECT_EQ(num_children(p0), 0);
        EXPECT_EQ(num_keys(p0), 0);
    }

    {
        node_ptr p0(new leaf_node_t);
        node_ptr p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_leaf()->refs_, 2);
        EXPECT_EQ(p0.as_leaf()->leaf_, true);
        EXPECT_EQ(p0.as_leaf()->prev_, nullptr);
        EXPECT_EQ(p0.as_leaf()->next_, nullptr);
        EXPECT_EQ(p0.as_leaf()->size(), 0);

        EXPECT_EQ(size(p0.get()), 0);
    }
}

TEST(rope_detail, test_make_node)
{
    {
        text t("some text");
        node_ptr p = make_node(t);

        EXPECT_EQ(size(p.get()), t.size());
        EXPECT_EQ(p.as_leaf()->as_text(), t);
        EXPECT_NE(p.as_leaf()->as_text().begin(), t.begin());
    }

    {
        text t("some text");
        node_ptr p = make_node(std::move(t));

        EXPECT_EQ(size(p.get()), 9);
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(p.as_leaf()->as_text(), "some text");
    }

    {
        text_view tv("some text");
        node_ptr p = make_node(tv);

        EXPECT_EQ(size(p.get()), tv.size());
        EXPECT_EQ(p.as_leaf()->as_text_view(), tv);
        EXPECT_EQ(p.as_leaf()->as_text_view().begin(), tv.begin());
    }

    {
        repeated_text_view rtv("abc", 3);
        node_ptr p = make_node(rtv);

        EXPECT_EQ(size(p.get()), rtv.size());
        EXPECT_EQ(p.as_leaf()->as_repeated_text_view(), rtv);
        EXPECT_EQ(p.as_leaf()->as_repeated_text_view().begin(), rtv.begin());
    }

    {
        text t("some text");
        node_ptr p_text = make_node(t);

        EXPECT_EQ(size(p_text.get()), t.size());
        EXPECT_EQ(p_text.as_leaf()->as_text(), t);
        EXPECT_NE(p_text.as_leaf()->as_text().begin(), t.begin());

        {
            node_ptr p_ref0 = make_ref(p_text.as_leaf(), 1, 8);

            EXPECT_EQ(size(p_ref0.get()), 7);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().ref_, "ome tex");
            EXPECT_NE(p_ref0.as_leaf()->as_reference().ref_.begin(), t.begin() + 1);

            EXPECT_EQ(p_text->refs_, 2);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().text_->refs_, 2);
            EXPECT_EQ(p_ref0->refs_, 1);

            node_ptr p_ref1 = make_ref(p_ref0.as_leaf()->as_reference(), 1, 6);

            EXPECT_EQ(size(p_ref1.get()), 5);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().ref_, "me te");
            EXPECT_NE(p_ref1.as_leaf()->as_reference().ref_.begin(), t.begin() + 2);

            EXPECT_EQ(p_text->refs_, 3);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().text_->refs_, 3);
            EXPECT_EQ(p_ref0->refs_, 1);
            EXPECT_EQ(p_ref1->refs_, 1);
        }
    }
}

// TODO: mutable_node_ptr
// TODO: find_child, find_leaf, find_char
// TODO: insert_child, erase_child
// TODO: slice_leaf
// TODO: erase_leaf
