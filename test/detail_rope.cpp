#include <boost/text/rope.hpp>

#include <gtest/gtest.h>


using boost::text::text;
using boost::text::text_view;
using boost::text::repeated_text_view;
using namespace boost::text::detail;

TEST(rope_detail, test_node_ptr)
{
    {
        node_ptr<rope_tag> p0(new_interior_node<rope_tag>());
        node_ptr<rope_tag> p1 = p0;

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
        node_ptr<rope_tag> p0(new leaf_node_t<rope_tag>);
        node_ptr<rope_tag> p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_leaf()->refs_, 2);
        EXPECT_EQ(p0.as_leaf()->leaf_, true);
        EXPECT_EQ(p0.as_leaf()->size(), 0);

        EXPECT_EQ(size(p0.get()), 0);
    }
}

TEST(rope_detail, test_make_node)
{
    {
        text t("some text");
        node_ptr<rope_tag> p = make_node(t);

        EXPECT_EQ(size(p.get()), t.size());
        EXPECT_EQ(p.as_leaf()->as_text(), t);
        EXPECT_NE(p.as_leaf()->as_text().begin(), t.begin());
    }

    {
        text t("some text");
        node_ptr<rope_tag> p = make_node(std::move(t));

        EXPECT_EQ(size(p.get()), 9);
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(p.as_leaf()->as_text(), "some text");
    }

    {
        text_view tv("some text");
        node_ptr<rope_tag> p = make_node(tv);

        EXPECT_EQ(size(p.get()), tv.size());
        EXPECT_EQ(p.as_leaf()->as_text(), tv);
    }

    {
        repeated_text_view rtv("abc", 3);
        node_ptr<rope_tag> p = make_node(rtv);

        EXPECT_EQ(size(p.get()), rtv.size());
        EXPECT_EQ(p.as_leaf()->as_repeated_text_view(), rtv);
        EXPECT_EQ(p.as_leaf()->as_repeated_text_view().begin(), rtv.begin());
    }

    {
        text t("some text");
        node_ptr<rope_tag> p_text = make_node(t);

        EXPECT_EQ(size(p_text.get()), t.size());
        EXPECT_EQ(p_text.as_leaf()->as_text(), t);
        EXPECT_NE(p_text.as_leaf()->as_text().begin(), t.begin());

        {
            node_ptr<rope_tag> p_ref0 = make_ref(p_text.as_leaf(), 1, 8);

            EXPECT_EQ(size(p_ref0.get()), 7);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().ref_, "ome tex");
            EXPECT_NE(
                p_ref0.as_leaf()->as_reference().ref_.begin(), t.begin() + 1);

            EXPECT_EQ(p_text->refs_, 2);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().text_->refs_, 2);
            EXPECT_EQ(p_ref0->refs_, 1);

            node_ptr<rope_tag> p_ref1 =
                make_ref(p_ref0.as_leaf()->as_reference(), 1, 6);

            EXPECT_EQ(size(p_ref1.get()), 5);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().ref_, "me te");
            EXPECT_NE(
                p_ref1.as_leaf()->as_reference().ref_.begin(), t.begin() + 2);

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
        node_ptr<rope_tag> p_text = make_node(t);

        auto mut_p_text = p_text.write();

        EXPECT_EQ(p_text->refs_, 1);
        EXPECT_EQ(mut_p_text.as_leaf(), p_text.as_leaf());

        mut_p_text.as_leaf()->as_text() += ".";

        EXPECT_EQ(p_text.as_leaf()->as_text(), "some text.");
        EXPECT_EQ(mut_p_text.as_leaf()->as_text(), "some text.");
    }

    text t("some text");
    node_ptr<rope_tag> p0 = make_node(t);
    node_ptr<rope_tag> p1 = p0;

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

node_ptr<rope_tag> make_tree()
{
    interior_node_t<rope_tag> * int_root = nullptr;
    node_ptr<rope_tag> root(int_root = new_interior_node<rope_tag>());

    interior_node_t<rope_tag> * int_left = nullptr;
    node_ptr<rope_tag> left(int_left = new_interior_node<rope_tag>());
    int_left->children_.push_back(make_node("left left"));
    int_left->keys_.push_back(size(int_left->children_[0].get()));
    int_left->children_.push_back(make_node("left right"));
    int_left->keys_.push_back(
        int_left->keys_[0] + size(int_left->children_[1].get()));

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    interior_node_t<rope_tag> * int_right = nullptr;
    node_ptr<rope_tag> right(int_right = new_interior_node<rope_tag>());
    int_right->children_.push_back(make_node("right left"));
    int_right->keys_.push_back(size(int_right->children_[0].get()));
    int_right->children_.push_back(make_node("right right"));
    int_right->keys_.push_back(
        int_right->keys_[0] + size(int_right->children_[1].get()));

    int_root->children_.push_back(right);
    int_root->keys_.push_back(int_root->keys_[0] + size(right.get()));

    return root;
}

TEST(rope_detail, test_find)
{
    // find_child

    {
        interior_node_t<rope_tag> parent;
        parent.children_.push_back(make_node(text_view("some")));
        parent.children_.push_back(make_node(text_view(" ")));
        parent.children_.push_back(make_node(text_view("text")));
        parent.keys_.push_back(4);
        parent.keys_.push_back(5);
        parent.keys_.push_back(9);

        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);

        EXPECT_EQ(find_child(&parent, 0), 0);
        EXPECT_EQ(find_child(&parent, 1), 0);
        EXPECT_EQ(find_child(&parent, 2), 0);
        EXPECT_EQ(find_child(&parent, 3), 0);
        EXPECT_EQ(find_child(&parent, 4), 1);
        EXPECT_EQ(find_child(&parent, 5), 2);
        EXPECT_EQ(find_child(&parent, 6), 2);
        EXPECT_EQ(find_child(&parent, 7), 2);
        EXPECT_EQ(find_child(&parent, 8), 2);
        EXPECT_EQ(find_child(&parent, 9), 2);
    }

    // find_leaf

    {
        node_ptr<rope_tag> root = make_node("test");
        found_leaf<rope_tag> found;

        find_leaf(root, 0, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 0);
        EXPECT_TRUE(found.path_.empty());

        find_leaf(root, 2, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 2);
        EXPECT_TRUE(found.path_.empty());

        find_leaf(root, 4, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 4);
        EXPECT_TRUE(found.path_.empty());
    }


    {
        interior_node_t<rope_tag> * int_root = nullptr;
        node_ptr<rope_tag> root(int_root = new_interior_node<rope_tag>());

        interior_node_t<rope_tag> * int_left = nullptr;
        node_ptr<rope_tag> left(int_left = new_interior_node<rope_tag>());
        int_left->children_.push_back(make_node("left left"));
        int_left->keys_.push_back(size(int_left->children_[0].get()));
        int_left->children_.push_back(make_node("left right"));
        int_left->keys_.push_back(
            int_left->keys_[0] + size(int_left->children_[1].get()));

        int_root->children_.push_back(left);
        int_root->keys_.push_back(size(left.get()));

        interior_node_t<rope_tag> * int_right = nullptr;
        node_ptr<rope_tag> right(int_right = new_interior_node<rope_tag>());
        int_right->children_.push_back(make_node("right left"));
        int_right->keys_.push_back(size(int_right->children_[0].get()));
        int_right->children_.push_back(make_node("right right"));
        int_right->keys_.push_back(
            int_right->keys_[0] + size(int_right->children_[1].get()));

        int_root->children_.push_back(right);
        int_root->keys_.push_back(int_root->keys_[0] + size(right.get()));

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 0, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left left");
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 8, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left left");
            EXPECT_EQ(found.offset_, 8);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 9, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left right");
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 10, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left right");
            EXPECT_EQ(found.offset_, 1);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 13, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left right");
            EXPECT_EQ(found.offset_, 4);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 18, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "left right");
            EXPECT_EQ(found.offset_, 9);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 19, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "right left");
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 28, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "right left");
            EXPECT_EQ(found.offset_, 9);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 29, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "right right");
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<rope_tag> found;
            find_leaf(root, 40, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_text(), "right right");
            EXPECT_EQ(found.offset_, 11);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }
    }

    // find_char

    {
        node_ptr<rope_tag> root = make_tree();
        found_char found;

        find_char(root, 0, found);
        EXPECT_EQ(found.c_, 'l');
        find_char(root, 8, found);
        EXPECT_EQ(found.c_, 't');
        find_char(root, 9, found);
        EXPECT_EQ(found.c_, 'l');
        find_char(root, 10, found);
        EXPECT_EQ(found.c_, 'e');
        find_char(root, 13, found);
        EXPECT_EQ(found.c_, ' ');
        find_char(root, 18, found);
        EXPECT_EQ(found.c_, 't');
        find_char(root, 19, found);
        EXPECT_EQ(found.c_, 'r');
        find_char(root, 28, found);
        EXPECT_EQ(found.c_, 't');
        find_char(root, 29, found);
        EXPECT_EQ(found.c_, 'r');
    }
}

void fill_interior_node(interior_node_t<rope_tag> & parent)
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
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        insert_child(&parent, 0, make_node("X"));
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text(), "X");
        EXPECT_EQ(parent.keys_[0], 1);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        insert_child(&parent, 2, make_node("X"));
        EXPECT_EQ(parent.children_[2].as_leaf()->as_text(), "X");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }


    {
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        insert_child(&parent, 3, make_node("X"));
        EXPECT_EQ(parent.children_[3].as_leaf()->as_text(), "X");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 9);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        erase_child(&parent, 0, dont_adjust_keys);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text(), " ");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text(), "text");
        EXPECT_EQ(parent.keys_[0], 5);
        EXPECT_EQ(parent.keys_[1], 9);
    }

    {
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        erase_child(&parent, 1);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text(), "text");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 8);
    }

    {
        interior_node_t<rope_tag> parent;
        fill_interior_node(parent);
        erase_child(&parent, 2);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_text(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_text(), " ");
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
    }
}

TEST(rope_detail, test_slice_leaf)
{
    // text

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, t.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, "some text");
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, t.size(), false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_text(), "some text");
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "ome tex");
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 = p0;
        node_ptr<rope_tag> p2 =
            slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p2.as_leaf()->as_reference().ref_, "ome tex");
    }

    // text_view

    {
        text_view tv("some text");
        node_ptr<rope_tag> p0 = make_node(tv);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, tv.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, "some text");
    }

    {
        text_view tv("some text");
        node_ptr<rope_tag> p0 = make_node(tv);
        slice_leaf(p0, 1, tv.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "ome tex");
    }

    {
        text_view tv("some text");
        node_ptr<rope_tag> p0 = make_node(tv);
        node_ptr<rope_tag> p1 = p0;
        node_ptr<rope_tag> p2 =
            slice_leaf(p0, 1, tv.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(p2.as_leaf()->as_reference().ref_, "ome tex");
    }

    // repeated_text_view

    {
        repeated_text_view rtv("text", 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, rtv.size(), true, check_encoding_breakage);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(p0->refs_, 1);
        EXPECT_EQ(p1->refs_, 1);
    }

    {
        repeated_text_view rtv("text", 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        node_ptr<rope_tag> p1 = slice_leaf(
            p0,
            rtv.view().size(),
            rtv.view().size() * 2,
            false,
            check_encoding_breakage);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "text");
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), "text");
    }

    {
        repeated_text_view rtv("text", 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        node_ptr<rope_tag> p1 = slice_leaf(
            p0,
            rtv.view().size(),
            rtv.view().size() + 1,
            false,
            check_encoding_breakage);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), "texttexttext");
        EXPECT_EQ(text(p1.as_leaf()->as_text()), "t");
    }

    // reference

    {
        text t("some text");
        node_ptr<rope_tag> pt = make_node(t);

        node_ptr<rope_tag> p0 =
            slice_leaf(pt, 0, t.size(), true, check_encoding_breakage);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, t.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "some text");
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, "some text");
    }

    {
        text t("some text");
        node_ptr<rope_tag> pt = make_node(t);

        node_ptr<rope_tag> p0 =
            slice_leaf(pt, 0, t.size(), true, check_encoding_breakage);
        slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "ome tex");
    }

    {
        text t("some text");
        node_ptr<rope_tag> pt = make_node(t);

        node_ptr<rope_tag> p0 =
            slice_leaf(pt, 0, t.size(), true, check_encoding_breakage);
        node_ptr<rope_tag> p1 = p0;
        node_ptr<rope_tag> p2 =
            slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, "some text");
        EXPECT_EQ(p2.as_leaf()->as_reference().ref_, "ome tex");
    }
}

TEST(rope_detail, test_slice_leaf_encoding_checks)
{
    // Unicode 9, 3.9/D90-D92
    // uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char const utf8[] = {0x4d,
                         char(0xd0),
                         char(0xb0),
                         char(0xe4),
                         char(0xba),
                         char(0x8c),
                         char(0xf0),
                         char(0x90),
                         char(0x8c),
                         char(0x82),
                         '\0'};

    // text

    {
        text t(utf8);
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, t.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), utf8);
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, utf8);
    }

#if 0 // TODO
    {
        text t(utf8);
        node_ptr<rope_tag> p0 = make_node(t);
        EXPECT_THROW(
            slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage),
            std::logic_error);
        EXPECT_NO_THROW(
            slice_leaf(p0, 1, t.size() - 1, false, encoding_breakage_ok));
    }
#endif

    // text_view

    {
        text_view tv(utf8);
        node_ptr<rope_tag> p0 = make_node(tv);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, tv.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), utf8);
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, utf8);
    }

#if 0 // TODO
    {
        text_view tv(utf8);
        node_ptr<rope_tag> p0 = make_node(tv);
        EXPECT_THROW(
            slice_leaf(p0, 1, tv.size() - 1, false, check_encoding_breakage),
            std::logic_error);
        EXPECT_NO_THROW(
            slice_leaf(p0, 1, tv.size() - 1, false, encoding_breakage_ok));
    }
#endif

    // repeated_text_view

    {
        repeated_text_view rtv(utf8, 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, rtv.size(), true, check_encoding_breakage);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), rtv);
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), rtv);
        EXPECT_EQ(p0->refs_, 1);
        EXPECT_EQ(p1->refs_, 1);
    }

    {
        repeated_text_view rtv(utf8, 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        node_ptr<rope_tag> p1 = slice_leaf(
            p0,
            rtv.view().size(),
            rtv.view().size() * 2,
            false,
            check_encoding_breakage);
        EXPECT_EQ(text(p0.as_leaf()->as_repeated_text_view()), utf8);
        EXPECT_EQ(text(p1.as_leaf()->as_repeated_text_view()), utf8);
    }

#if 0 // TODO
    {
        repeated_text_view rtv(utf8, 3);
        node_ptr<rope_tag> p0 = make_node(rtv);
        EXPECT_THROW(
            slice_leaf(
                p0,
                rtv.view().size() - 1,
                rtv.view().size(),
                false,
                check_encoding_breakage),
            std::logic_error);
        EXPECT_NO_THROW(slice_leaf(
            p0,
            rtv.view().size() - 1,
            rtv.view().size(),
            false,
            encoding_breakage_ok));
    }
#endif

    // reference

    {
        text t(utf8);
        node_ptr<rope_tag> pt = make_node(t);

        node_ptr<rope_tag> p0 =
            slice_leaf(pt, 0, t.size(), true, check_encoding_breakage);
        node_ptr<rope_tag> p1 =
            slice_leaf(p0, 0, t.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, utf8);
        EXPECT_EQ(p1.as_leaf()->as_reference().ref_, utf8);
    }

    {
        text t(utf8);
        node_ptr<rope_tag> pt = make_node(t);

        node_ptr<rope_tag> p0 =
            slice_leaf(pt, 0, t.size(), true, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_reference().ref_, utf8);

#if 0 // TODO
        EXPECT_THROW(
            slice_leaf(p0, 1, t.size() - 1, false, check_encoding_breakage),
            std::logic_error);
        EXPECT_NO_THROW(
            slice_leaf(p0, 1, t.size() - 1, false, encoding_breakage_ok));
#endif
    }
}

TEST(rope_detail, test_erase_leaf)
{
    // text

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 0, 9, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.get(), nullptr);
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 1, 9, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "s");
        EXPECT_EQ(slices.slice.as_leaf()->as_text(), "s");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 = p0;
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 1, 9, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "s");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 = p0;
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 0, 8, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "t");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 = p0;
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 1, 8, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "s");
        EXPECT_EQ(slices.other_slice.as_leaf()->as_reference().ref_, "t");
    }


    // text_view

    {
        text_view t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 1, 8, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "st");
        EXPECT_EQ(slices.slice.as_leaf()->as_text(), "st");
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        text_view t("some text");
        node_ptr<rope_tag> p0 = make_node(t);
        node_ptr<rope_tag> p1 = p0;
        leaf_slices<rope_tag> slices =
            erase_leaf(p0, 1, 8, check_encoding_breakage);
        EXPECT_EQ(p0.as_leaf()->as_text(), "some text");
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().ref_, "s");
        EXPECT_EQ(slices.other_slice.as_leaf()->as_reference().ref_, "t");
    }
}
