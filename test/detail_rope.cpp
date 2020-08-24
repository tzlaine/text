// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/unencoded_rope.hpp>

#include <gtest/gtest.h>


using boost::text::string_view;
using namespace boost::text::detail;

TEST(rope_detail, test_node_ptr)
{
    {
        node_ptr<char, std::string> p0(new_interior_node<char, std::string>());
        node_ptr<char, std::string> p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_interior()->refs_, 2);
        EXPECT_EQ(p0.as_interior()->leaf_, false);
        EXPECT_EQ(p0.as_interior()->keys_.size(), 0u);
        EXPECT_EQ(p0.as_interior()->children_.size(), 0u);

        EXPECT_EQ(size(p0.get()), 0u);

        (void)children(p0);
        (void)keys(p0);

        EXPECT_EQ(num_children(p0), 0u);
        EXPECT_EQ(num_keys(p0), 0u);
    }

    {
        node_ptr<char, std::string> p0(new leaf_node_t<char, std::string>);
        node_ptr<char, std::string> p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_leaf()->refs_, 2);
        EXPECT_EQ(p0.as_leaf()->leaf_, true);
        EXPECT_EQ(p0.as_leaf()->size(), 0u);

        EXPECT_EQ(size(p0.get()), 0u);
    }
}

TEST(rope_detail, test_make_node)
{
    {
        std::string t("some text");
        node_ptr<char, std::string> p = make_node(t);

        EXPECT_EQ(size(p.get()), t.size());
        EXPECT_EQ(p.as_leaf()->as_seg(), t);
        EXPECT_NE(&*p.as_leaf()->as_seg().begin(), &*t.begin());
    }

    {
        std::string t("some text");
        node_ptr<char, std::string> p = make_node(std::move(t));

        EXPECT_EQ(size(p.get()), 9u);
        EXPECT_EQ(t.size(), 0u);
        EXPECT_EQ(p.as_leaf()->as_seg(), "some text");
    }

    {
        string_view tv("some text");
        node_ptr<char, string_view> p = make_node(tv);

        EXPECT_EQ(size(p.get()), tv.size());
        EXPECT_EQ(p.as_leaf()->as_seg(), tv);
    }
}

node_ptr<char, std::string> make_tree()
{
    interior_node_t<char, std::string> * int_root = nullptr;
    node_ptr<char, std::string> root(int_root = new_interior_node<char, std::string>());

    interior_node_t<char, std::string> * int_left = nullptr;
    node_ptr<char, std::string> left(int_left = new_interior_node<char, std::string>());
    int_left->children_.push_back(make_node<std::string>("left left"));
    int_left->keys_.push_back(size(int_left->children_[0].get()));
    int_left->children_.push_back(make_node<std::string>("left right"));
    int_left->keys_.push_back(
        int_left->keys_[0] + size(int_left->children_[1].get()));

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    interior_node_t<char, std::string> * int_right = nullptr;
    node_ptr<char, std::string> right(int_right = new_interior_node<char, std::string>());
    int_right->children_.push_back(make_node<std::string>("right left"));
    int_right->keys_.push_back(size(int_right->children_[0].get()));
    int_right->children_.push_back(make_node<std::string>("right right"));
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
        interior_node_t<char, string_view> parent;
        parent.children_.push_back(make_node(string_view("some")));
        parent.children_.push_back(make_node(string_view(" ")));
        parent.children_.push_back(make_node(string_view("text")));
        parent.keys_.push_back(4);
        parent.keys_.push_back(5);
        parent.keys_.push_back(9);

        EXPECT_EQ(parent.keys_[0], 4u);
        EXPECT_EQ(parent.keys_[1], 5u);

        EXPECT_EQ(find_child(&parent, 0), 0u);
        EXPECT_EQ(find_child(&parent, 1), 0u);
        EXPECT_EQ(find_child(&parent, 2), 0u);
        EXPECT_EQ(find_child(&parent, 3), 0u);
        EXPECT_EQ(find_child(&parent, 4), 1u);
        EXPECT_EQ(find_child(&parent, 5), 2u);
        EXPECT_EQ(find_child(&parent, 6), 2u);
        EXPECT_EQ(find_child(&parent, 7), 2u);
        EXPECT_EQ(find_child(&parent, 8), 2u);
        EXPECT_EQ(find_child(&parent, 9), 2u);
    }

    // find_leaf

    {
        node_ptr<char, std::string> root = make_node<std::string>("test");
        found_leaf<char, std::string> found;

        find_leaf(root, 0, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 0u);
        EXPECT_TRUE(found.path_.empty());

        find_leaf(root, 2, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 2u);
        EXPECT_TRUE(found.path_.empty());

        find_leaf(root, 4, found);
        EXPECT_EQ(found.leaf_, &root);
        EXPECT_EQ(found.offset_, 4u);
        EXPECT_TRUE(found.path_.empty());
    }


    {
        interior_node_t<char, std::string> * int_root = nullptr;
        node_ptr<char, std::string> root(int_root = new_interior_node<char, std::string>());

        interior_node_t<char, std::string> * int_left = nullptr;
        node_ptr<char, std::string> left(int_left = new_interior_node<char, std::string>());
        int_left->children_.push_back(make_node<std::string>("left left"));
        int_left->keys_.push_back(size(int_left->children_[0].get()));
        int_left->children_.push_back(make_node<std::string>("left right"));
        int_left->keys_.push_back(
            int_left->keys_[0] + size(int_left->children_[1].get()));

        int_root->children_.push_back(left);
        int_root->keys_.push_back(size(left.get()));

        interior_node_t<char, std::string> * int_right = nullptr;
        node_ptr<char, std::string> right(int_right = new_interior_node<char, std::string>());
        int_right->children_.push_back(make_node<std::string>("right left"));
        int_right->keys_.push_back(size(int_right->children_[0].get()));
        int_right->children_.push_back(make_node<std::string>("right right"));
        int_right->keys_.push_back(
            int_right->keys_[0] + size(int_right->children_[1].get()));

        int_root->children_.push_back(right);
        int_root->keys_.push_back(int_root->keys_[0] + size(right.get()));

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 0, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left left");
            EXPECT_EQ(found.offset_, 0u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 8, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left left");
            EXPECT_EQ(found.offset_, 8u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 9, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left right");
            EXPECT_EQ(found.offset_, 0u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 10, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left right");
            EXPECT_EQ(found.offset_, 1u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 13, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left right");
            EXPECT_EQ(found.offset_, 4u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 18, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "left right");
            EXPECT_EQ(found.offset_, 9u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 19, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "right left");
            EXPECT_EQ(found.offset_, 0u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 28, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "right left");
            EXPECT_EQ(found.offset_, 9u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 29, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "right right");
            EXPECT_EQ(found.offset_, 0u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<char, std::string> found;
            find_leaf(root, 40, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_seg(), "right right");
            EXPECT_EQ(found.offset_, 11u);
            EXPECT_EQ(found.path_.size(), 2u);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }
    }
}

void fill_interior_node(interior_node_t<char, std::string> & parent)
{
    parent.children_.push_back(make_node<std::string>("some"));
    parent.children_.push_back(make_node<std::string>(" "));
    parent.children_.push_back(make_node<std::string>("text"));
    parent.keys_.push_back(4);
    parent.keys_.push_back(5);
    parent.keys_.push_back(9);
}

TEST(rope_detail, test_insert_erase_child)
{
    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        insert_child(&parent, 0, make_node<std::string>("X"));
        EXPECT_EQ(parent.children_[0].as_leaf()->as_seg(), "X");
        EXPECT_EQ(parent.keys_[0], 1u);
        EXPECT_EQ(parent.keys_[1], 5u);
        EXPECT_EQ(parent.keys_[2], 6u);
        EXPECT_EQ(parent.keys_[3], 10u);
    }

    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        insert_child(&parent, 2, make_node<std::string>("X"));
        EXPECT_EQ(parent.children_[2].as_leaf()->as_seg(), "X");
        EXPECT_EQ(parent.keys_[0], 4u);
        EXPECT_EQ(parent.keys_[1], 5u);
        EXPECT_EQ(parent.keys_[2], 6u);
        EXPECT_EQ(parent.keys_[3], 10u);
    }


    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        insert_child(&parent, 3, make_node<std::string>("X"));
        EXPECT_EQ(parent.children_[3].as_leaf()->as_seg(), "X");
        EXPECT_EQ(parent.keys_[0], 4u);
        EXPECT_EQ(parent.keys_[1], 5u);
        EXPECT_EQ(parent.keys_[2], 9u);
        EXPECT_EQ(parent.keys_[3], 10u);
    }

    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        erase_child(&parent, 0, dont_adjust_keys);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_seg(), " ");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_seg(), "text");
        EXPECT_EQ(parent.keys_[0], 5u);
        EXPECT_EQ(parent.keys_[1], 9u);
    }

    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        erase_child(&parent, 1);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_seg(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_seg(), "text");
        EXPECT_EQ(parent.keys_[0], 4u);
        EXPECT_EQ(parent.keys_[1], 8u);
    }

    {
        interior_node_t<char, std::string> parent;
        fill_interior_node(parent);
        erase_child(&parent, 2);
        EXPECT_EQ(parent.children_[0].as_leaf()->as_seg(), "some");
        EXPECT_EQ(parent.children_[1].as_leaf()->as_seg(), " ");
        EXPECT_EQ(parent.keys_[0], 4u);
        EXPECT_EQ(parent.keys_[1], 5u);
    }
}
