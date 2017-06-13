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

        EXPECT_EQ(p_text->refs_, 1);
    }
}

TEST(rope_detail, test_mutable_node_ptr)
{
    {
        text t("some text");
        node_ptr p_text = make_node(t);

        auto mut_p_text = p_text.write();

        EXPECT_EQ(p_text->refs_, 1);
        EXPECT_EQ(mut_p_text.as_leaf(), p_text.as_leaf());

        mut_p_text.as_leaf()->as_text() += ".";

        EXPECT_EQ(p_text.as_leaf()->as_text(), "some text.");
        EXPECT_EQ(mut_p_text.as_leaf()->as_text(), "some text.");
    }

    text t("some text");
    node_ptr p0 = make_node(t);
    node_ptr p1 = p0;

    {
        auto mut_p0 = p0.write();

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_NE(mut_p0.as_leaf(), p0.as_leaf());

        mut_p0.as_leaf()->as_text() += " --";

        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(mut_p0.as_leaf()->as_text(), "some text --");
    }

    EXPECT_EQ(p0.as_leaf()->as_text(), "some text --");
}

TEST(rope_detail, test_find)
{
    interior_node_t parent;
    parent.children_.push_back(make_node(text_view("some")));
    parent.children_.push_back(make_node(text_view(" ")));
    parent.children_.push_back(make_node(text_view("text")));
    parent.keys_.push_back(4);
    parent.keys_.push_back(5);
    parent.keys_.push_back(9);

    EXPECT_EQ(parent.offset(0), 0);
    EXPECT_EQ(parent.offset(1), 4);
    EXPECT_EQ(parent.offset(2), 5);

    EXPECT_EQ(find_child(&parent, 0), 0);
    EXPECT_EQ(find_child(&parent, 1), 0);
    EXPECT_EQ(find_child(&parent, 2), 0);
    EXPECT_EQ(find_child(&parent, 3), 0);
    EXPECT_EQ(find_child(&parent, 4), 0);
    EXPECT_EQ(find_child(&parent, 5), 1);
    EXPECT_EQ(find_child(&parent, 6), 2);
    EXPECT_EQ(find_child(&parent, 7), 2);
    EXPECT_EQ(find_child(&parent, 8), 2);
    EXPECT_EQ(find_child(&parent, 9), 2);

    // TODO: find_leaf, find_char
}

void fill_interior_node (interior_node_t & parent)
{
    parent.children_.push_back(make_node(text_view("some")));
    parent.children_.push_back(make_node(text_view(" ")));
    parent.children_.push_back(make_node(text_view("text")));
    parent.keys_.push_back(4);
    parent.keys_.push_back(5);
    parent.keys_.push_back(9);
}

TEST(rope_detail, test_insert_erase_child)
{
    {
        interior_node_t parent;
        fill_interior_node(parent);
        insert_child(&parent, 0, make_node("X"));
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text_view(), "X");
        EXPECT_EQ(parent.keys_[0], 1);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t parent;
        fill_interior_node(parent);
        insert_child(&parent, 2, make_node("X"));
        EXPECT_EQ(parent.children_[2].as_leaf()->as_text_view(), "X");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }


    {
        interior_node_t parent;
        fill_interior_node(parent);
        insert_child(&parent, 3, make_node("X"));
        EXPECT_EQ(parent.children_[3].as_leaf()->as_text_view(), "X");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 9);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t parent;
        fill_interior_node(parent);
        erase_child(&parent, 0, dont_adjust_keys);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text_view(), " ");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text_view(), "text");
        EXPECT_EQ(parent.keys_[0], 5);
        EXPECT_EQ(parent.keys_[1], 9);
    }

    {
        interior_node_t parent;
        fill_interior_node(parent);
        erase_child(&parent, 1);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text_view(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text_view(), "text");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 8);
    }

    {
        interior_node_t parent;
        fill_interior_node(parent);
        erase_child(&parent, 2);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text_view(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text_view(), " ");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
    }
}

TEST(rope_detail, test_slice_leaf)
{
    // text

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = slice_leaf(p0, 0, t.size(), true);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, "some text");
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = slice_leaf(p0, 0, t.size(), false);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_text(), "some text");
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        slice_leaf(p0, 1, t.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_text(), "ome tex");
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = p0;
        node_ptr p2 = slice_leaf(p0, 1, t.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p2.as_leaf()->as_reference().ref_, "ome tex");
    }

    // text_view

    {
        text_view tv("some text");
        node_ptr p0 = make_node(tv);
        node_ptr p1 = slice_leaf(p0, 0, tv.size(), true);
        EXPECT_EQ(p0.as_leaf()->as_text_view(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_text_view(), "some text");
    }

    {
        text_view tv("some text");
        node_ptr p0 = make_node(tv);
        slice_leaf(p0, 1, tv.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_text_view(), "ome tex");
    }

    {
        text_view tv("some text");
        node_ptr p0 = make_node(tv);
        node_ptr p1 = p0;
        node_ptr p2 = slice_leaf(p0, 1, tv.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_text_view(), "some text");
        EXPECT_EQ(p2.as_leaf()->as_text_view(), "ome tex");
    }

    // repeated_text_view

    {
        repeated_text_view rtv("text", 3);
        node_ptr p0 = make_node(rtv);
        node_ptr p1 = slice_leaf(p0, 0, rtv.size(), true);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(p0->refs_, 1);
        EXPECT_EQ(p1->refs_, 1);
    }

    {
        repeated_text_view rtv("text", 3);
        node_ptr p0 = make_node(rtv);
        node_ptr p1 = slice_leaf(p0, rtv.view().size(), rtv.view().size() * 2, false);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "text");
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), "text");
    }

    {
        repeated_text_view rtv("text", 3);
        node_ptr p0 = make_node(rtv);
        node_ptr p1 = slice_leaf(p0, rtv.view().size(), rtv.view().size() + 1, false);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(text(p1.as_leaf()->as_text()), "t");
    }

    // reference

    {
        text t("some text");
        node_ptr pt = make_node(t);

        node_ptr p0 = slice_leaf(pt, 0, t.size(), true);
        node_ptr p1 = slice_leaf(p0, 0, t.size(), true);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "some text");
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, "some text");
    }

    {
        text t("some text");
        node_ptr pt = make_node(t);

        node_ptr p0 = slice_leaf(pt, 0, t.size(), true);
        slice_leaf(p0, 1, t.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "ome tex");
    }

    {
        text t("some text");
        node_ptr pt = make_node(t);

        node_ptr p0 = slice_leaf(pt, 0, t.size(), true);
        node_ptr p1 = p0;
        node_ptr p2 = slice_leaf(p0, 1, t.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "some text");
        EXPECT_EQ(p2.as_leaf()->as_reference().ref_, "ome tex");
    }
}

TEST(rope_detail, test_erase_leaf)
{
    // text

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        leaf_slices slices = erase_leaf(p0, 0, 9);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.get(), nullptr);
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        leaf_slices slices = erase_leaf(p0, 1, 9);
        EXPECT_EQ(p0.as_leaf()->as_text(), "s");
        EXPECT_EQ(slices.slice.as_leaf()->as_text(), "s");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = p0;
        leaf_slices slices = erase_leaf(p0, 1, 9);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "s");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = p0;
        leaf_slices slices = erase_leaf(p0, 0, 8);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "t");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = p0;
        leaf_slices slices = erase_leaf(p0, 1, 8);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "s");
        EXPECT_EQ(slices.other_slice.as_leaf()->as_reference().ref_, "t");
    }


    // text_view

    {
        text_view t("some text");
        node_ptr p0 = make_node(t);
        leaf_slices slices = erase_leaf(p0, 1, 8);
        EXPECT_EQ(p0.as_leaf()->as_text_view(), "s");
        EXPECT_EQ(slices.slice.as_leaf()->as_text_view(), "s");
        EXPECT_EQ(slices.other_slice.as_leaf()->as_text_view(), "t");
    }

    {
        text_view t("some text");
        node_ptr p0 = make_node(t);
        node_ptr p1 = p0;
        leaf_slices slices = erase_leaf(p0, 1, 8);
        EXPECT_EQ(p0.as_leaf()->as_text_view(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_text_view(), "s");
        EXPECT_EQ(slices.other_slice.as_leaf()->as_text_view(), "t");
    }
}
