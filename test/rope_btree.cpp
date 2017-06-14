#include <boost/text/rope.hpp>

#include <gtest/gtest.h>


using boost::text::text;
using boost::text::text_view;
using boost::text::repeated_text_view;
using namespace boost::text::detail;


leaf_node_t * link_leafs (leaf_node_t * prev, interior_node_t const * node)
{
    for (auto c : const_cast<interior_node_t *>(node)->children_) {
        auto curr = const_cast<leaf_node_t *>(c.as_leaf());
        if (prev)
            prev->next_ = curr;
        curr->prev_ = prev;
        prev = curr;
    }
    return prev;
}

node_ptr make_interior_with_leaves (char const * name, int leaves)
{
    interior_node_t * int_node = nullptr;
    node_ptr node(int_node = new interior_node_t);
    int_node->children_.push_back(make_node(name));
    int_node->keys_.push_back(size(int_node->children_[0].get()));
    for (int i = 1; i < leaves; ++i) {
        int_node->children_.push_back(make_node(name));
        int_node->keys_.push_back(int_node->keys_.back() + size(int_node->children_[i].get()));
    }
    return node;
}

node_ptr make_tree_left_max ()
{
    interior_node_t * int_root = nullptr;
    node_ptr root(int_root = new interior_node_t);

    node_ptr left = make_interior_with_leaves("left", max_children);

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    node_ptr right = make_interior_with_leaves("right", max_children);

    int_root->children_.push_back(right);
    int_root->keys_.push_back(int_root->keys_.back() + size(right.get()));

    leaf_node_t * prev = link_leafs(nullptr, left.as_interior());
    prev = link_leafs(prev, right.as_interior());
    prev->next_ = nullptr;

    return root;
}

node_ptr make_tree_left_min ()
{
    interior_node_t * int_root = nullptr;
    node_ptr root(int_root = new interior_node_t);

    node_ptr left = make_interior_with_leaves("left", min_children);

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    node_ptr right = make_interior_with_leaves("right", max_children);

    int_root->children_.push_back(right);
    int_root->keys_.push_back(int_root->keys_.back() + size(right.get()));

    leaf_node_t * prev = link_leafs(nullptr, left.as_interior());
    prev = link_leafs(prev, right.as_interior());
    prev->next_ = nullptr;

    return root;
}


TEST(rope_btree, test_btree_split_child)
{
    node_ptr root = make_tree_left_max();
    node_ptr root_2 = btree_split_child(root, 0);

    EXPECT_EQ(root->refs_, 2);
    EXPECT_EQ(root_2->refs_, 2);

    EXPECT_EQ(children(root).size(), 3);
    EXPECT_EQ(keys(root)[0], 16);
    EXPECT_EQ(keys(root)[1], 32);
    EXPECT_EQ(keys(root)[2], 72);

    EXPECT_EQ(children(children(root)[0]).size(), 4);
    EXPECT_EQ(keys(children(root)[0]).size(), 4);
    EXPECT_EQ(keys(children(root)[0])[0], 4);
    EXPECT_EQ(keys(children(root)[0])[1], 8);
    EXPECT_EQ(keys(children(root)[0])[2], 12);
    EXPECT_EQ(keys(children(root)[0])[3], 16);

    EXPECT_EQ(children(children(root)[1]).size(), 4);
    EXPECT_EQ(keys(children(root)[1]).size(), 4);
    EXPECT_EQ(keys(children(root)[1])[0], 4);
    EXPECT_EQ(keys(children(root)[1])[1], 8);
    EXPECT_EQ(keys(children(root)[1])[2], 12);
    EXPECT_EQ(keys(children(root)[1])[3], 16);
}

TEST(rope_btree, test_btree_split_leaf)
{
    {
        node_ptr root = make_tree_left_min();
        node_ptr left = children(root)[0];
        btree_split_leaf(left, 1, 4);
        EXPECT_EQ(size(children(left)[1].get()), 4);
    }

    {
        node_ptr root = make_tree_left_min();
        node_ptr left = children(root)[0];
        btree_split_leaf(left, 1, 5);

        EXPECT_EQ(num_children(left), 5);

        EXPECT_EQ(keys(left)[0], 4);
        EXPECT_EQ(size(children(left)[1].get()), 1);
        EXPECT_EQ(keys(left)[1], 5);
        EXPECT_EQ(size(children(left)[2].get()), 3);
        EXPECT_EQ(keys(left)[2], 8);
        EXPECT_EQ(keys(left)[3], 12);
        EXPECT_EQ(keys(left)[4], 16);

        auto const first_left_child = children(left)[0].as_leaf();
        auto const last_left_child = children(left)[4].as_leaf();

        {
            auto child = first_left_child;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            EXPECT_EQ(child, last_left_child);
        }

        {
            auto child = last_left_child;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            EXPECT_EQ(child, first_left_child);
        }
    }
}

TEST(rope_btree, test_btree_insert_nonfull)
{
}

TEST(rope_btree, test_btree_insert)
{
}

TEST(rope_btree, test_btree_erase_impl)
{
}

TEST(rope_btree, test_btree_erase)
{
}
