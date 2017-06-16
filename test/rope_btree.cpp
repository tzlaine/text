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

node_ptr make_interior_with_leaves (char const * leaf_name, int leaves)
{
    interior_node_t * int_node = nullptr;
    node_ptr node(int_node = new interior_node_t);
    int_node->children_.push_back(make_node(leaf_name));
    int_node->keys_.push_back(size(int_node->children_[0].get()));
    for (int i = 1; i < leaves; ++i) {
        int_node->children_.push_back(make_node(leaf_name));
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

    node_ptr right = make_interior_with_leaves("right", max_children - 1);

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

    node_ptr right = make_interior_with_leaves("right", max_children - 1);

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
    EXPECT_EQ(keys(root)[0], max_children * 2);
    EXPECT_EQ(keys(root)[1], max_children * 2 * 2);
    EXPECT_EQ(keys(root)[2], max_children * 2 * 2 + (max_children - 1) * 5);

    EXPECT_EQ(num_children(children(root)[0]), min_children);
    EXPECT_EQ(keys(children(root)[0]).size(), min_children);
    EXPECT_EQ(keys(children(root)[0])[0], 4);
    EXPECT_EQ(keys(children(root)[0])[1], 8);
    EXPECT_EQ(keys(children(root)[0])[2], 12);
    EXPECT_EQ(keys(children(root)[0])[3], 16);

    EXPECT_EQ(num_children(children(root)[1]), min_children);
    EXPECT_EQ(keys(children(root)[1]).size(), min_children);
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

        EXPECT_EQ(num_children(left), min_children + 1);

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

    {
        node_ptr root = make_tree_left_min();
        node_ptr left = children(root)[0];

        // Take an extra reference to the child begin split.
        node_ptr left_1 = children(left)[1];

        btree_split_leaf(left, 1, 5);

        EXPECT_EQ(num_children(left), min_children + 1);

        EXPECT_EQ(keys(left)[0], 4);
        EXPECT_EQ(size(children(left)[1].get()), 1);
        EXPECT_EQ(keys(left)[1], 5);
        EXPECT_EQ(size(children(left)[2].get()), 3);
        EXPECT_EQ(keys(left)[2], 8);
        EXPECT_EQ(keys(left)[3], 12);
        EXPECT_EQ(keys(left)[4], 16);

        EXPECT_EQ(size(left_1.get()), 4);

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
    // Insert into half-full interior child, then between existing leaves.
    {
        node_ptr root = make_tree_left_min();

        node_ptr const & left = children(root)[0];
        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, 4, make_node("new node"));

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children + 1);

        EXPECT_EQ(keys(left)[0], 4);
        EXPECT_EQ(size(children(left)[1].get()), 8);
        EXPECT_EQ(keys(left)[1], 12);
        EXPECT_EQ(size(children(left)[2].get()), 4);
        EXPECT_EQ(keys(left)[2], 16);
        EXPECT_EQ(size(children(left)[3].get()), 4);
        EXPECT_EQ(keys(left)[3], 20);
        EXPECT_EQ(keys(left)[4], 24);

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

    // Insert into half-full interior child, then into the middle of an
    // existing leaf.
    {
        node_ptr root = make_tree_left_min();
        node_ptr new_node = make_node("new node");

        node_ptr const & left = children(root)[0];
        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, 5, make_node("new node"));

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children + 2);

        EXPECT_EQ(keys(left)[0], 4);
        EXPECT_EQ(size(children(left)[1].get()), 1);
        EXPECT_EQ(keys(left)[1], 5);
        EXPECT_EQ(size(children(left)[2].get()), 8);
        EXPECT_EQ(keys(left)[2], 13);
        EXPECT_EQ(size(children(left)[3].get()), 3);
        EXPECT_EQ(keys(left)[3], 16);
        EXPECT_EQ(keys(left)[4], 20);
        EXPECT_EQ(keys(left)[5], 24);

        auto const first_left_child = children(left)[0].as_leaf();
        auto const last_left_child = children(left)[5].as_leaf();

        {
            auto child = first_left_child;
            child = child->next_;
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
            child = child->prev_;
            EXPECT_EQ(child, first_left_child);
        }
    }

    // Insert into full interior child, then between existing leaves.
    {
        node_ptr root = make_tree_left_max();

        node_ptr const & left = children(root)[0];
        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, 4, make_node("new node"));

        EXPECT_EQ(num_children(root), 3);
        EXPECT_EQ(num_children(left), min_children + 1);

        EXPECT_EQ(keys(left)[0], 4);
        EXPECT_EQ(size(children(left)[1].get()), 8);
        EXPECT_EQ(keys(left)[1], 12);
        EXPECT_EQ(size(children(left)[2].get()), 4);
        EXPECT_EQ(keys(left)[2], 16);
        EXPECT_EQ(size(children(left)[3].get()), 4);
        EXPECT_EQ(keys(left)[3], 20);
        EXPECT_EQ(keys(left)[4], 24);

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

    // Insert into almost-full interior child, then between existing leaves.
    {
        node_ptr root = make_tree_left_max();

        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, size(root.get()) - 5, make_node("new node"));

        EXPECT_EQ(num_children(root), 3);

        node_ptr const & new_right = children(root)[2];
        EXPECT_EQ(num_children(new_right), min_children);

        EXPECT_EQ(size(children(new_right)[min_children - 2].get()), 8);
        EXPECT_EQ(keys(new_right)[min_children - 2], (min_children - 2) * 5 + 8);
        EXPECT_EQ(size(children(new_right)[min_children - 1].get()), 5);
        EXPECT_EQ(keys(new_right)[min_children - 1], (min_children - 2) * 5 + 8 + 5);

        auto const first_right_child = children(new_right)[0].as_leaf();
        auto const last_right_child = children(new_right)[3].as_leaf();

        {
            auto child = first_right_child;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            EXPECT_EQ(child, last_right_child);
        }

        {
            auto child = last_right_child;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            EXPECT_EQ(child, first_right_child);
        }
    }

    // Insert into almost-full interior child, then into the middle of an
    // existing leaf.
    {
        node_ptr root = make_tree_left_max();

        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, size(root.get()) - 2, make_node("new node"));

        EXPECT_EQ(num_children(root), 3);

        node_ptr const & new_right = children(root)[2];
        EXPECT_EQ(num_children(new_right), min_children + 1);

        EXPECT_EQ(size(children(new_right)[min_children - 2].get()), 3);
        EXPECT_EQ(keys(new_right)[min_children - 2], (min_children - 2) * 5 + 3);
        EXPECT_EQ(size(children(new_right)[min_children - 1].get()), 8);
        EXPECT_EQ(keys(new_right)[min_children - 1], (min_children - 2) * 5 + 3 + 8);
        EXPECT_EQ(size(children(new_right)[min_children].get()), 2);
        EXPECT_EQ(keys(new_right)[min_children], (min_children - 2) * 5 + 3 + 8 + 2);

        auto const first_right_child = children(new_right)[0].as_leaf();
        auto const last_right_child = children(new_right)[4].as_leaf();

        {
            auto child = first_right_child;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            EXPECT_EQ(child, last_right_child);
        }

        {
            auto child = last_right_child;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            EXPECT_EQ(child, first_right_child);
        }
    }


    // Insert into almost-full interior child, then after the last leaf.
    {
        node_ptr root = make_tree_left_max();

        node_ptr const & right = children(root)[1];

        EXPECT_EQ(num_children(right), max_children - 1);

        node_ptr new_root =
            btree_insert_nonfull(root, size(root.get()), make_node("new node"));

        EXPECT_EQ(num_children(root), 3);

        node_ptr const & new_right = children(root)[2];
        EXPECT_EQ(num_children(new_right), min_children);

        EXPECT_EQ(size(children(new_right)[min_children - 1].get()), 8);
        EXPECT_EQ(keys(new_right)[min_children - 1], (min_children - 1) * 5 + 8);

        auto const first_right_child = children(new_right)[0].as_leaf();
        auto const last_right_child = children(new_right)[3].as_leaf();

        {
            auto child = first_right_child;
            child = child->next_;
            child = child->next_;
            child = child->next_;
            EXPECT_EQ(child, last_right_child);
        }

        {
            auto child = last_right_child;
            child = child->prev_;
            child = child->prev_;
            child = child->prev_;
            EXPECT_EQ(child, first_right_child);
        }
    }

    // Copy vs. mutation coverage.

    // No nodes copied.
    {
        node_ptr root = make_tree_left_min();

        node_ptr const & left = children(root)[0];

        EXPECT_EQ(num_children(left), min_children);

        node_ptr new_root =
            btree_insert_nonfull(root, 4, make_node("new node"));

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children + 1);
        EXPECT_EQ(root.as_interior(), new_root.as_interior());
    }

    // Root copied.
    {
        node_ptr root = make_tree_left_min();
        node_ptr root_2 = root;

        node_ptr const & left = children(root)[0];

        EXPECT_EQ(num_children(left), min_children);

        node_ptr new_root =
            btree_insert_nonfull(root, 4, make_node("new node"));

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(root.as_interior(), new_root.as_interior());
        EXPECT_NE(root.as_interior(), root_2.as_interior());
    }

    // Interior node copied.
    {
        node_ptr root = make_tree_left_min();

        node_ptr left = children(root)[0];

        EXPECT_EQ(num_children(left), min_children);

        node_ptr new_root =
            btree_insert_nonfull(root, 4, make_node("new node"));

        node_ptr const & new_left = children(root)[0];

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(new_left), min_children + 1);
    }
}

int height_at (node_ptr const & node, std::ptrdiff_t at)
{
    found_leaf found;
    find_leaf(node, at, found);
    return (int)found.path_.size();
}

void check_leaf_heights (node_ptr const & node)
{
    found_leaf found;
    find_leaf(node, 0, found);
    int const first_leaf_height = (int)found.path_.size();
    leaf_node_t const * leaf = found.leaf_->as_leaf();
    std::ptrdiff_t offset = 0;
    while (leaf) {
        EXPECT_EQ(height_at(node, offset), first_leaf_height);
        offset += leaf->size();
        leaf = leaf->next_;
    }
}

TEST(rope_btree, test_btree_insert)
{
    {
        node_ptr root = make_node("root");
        root = btree_insert(root, 0, make_node("new"));

        EXPECT_FALSE(root->leaf_);
        EXPECT_EQ(num_children(root), 2);

        check_leaf_heights(root);
    }

    {
        node_ptr root = make_node("root");
        root = btree_insert(root, 4, make_node("new"));

        EXPECT_FALSE(root->leaf_);
        EXPECT_EQ(num_children(root), 2);

        check_leaf_heights(root);
    }

    {
        node_ptr root = make_node("root");
        root = btree_insert(root, 2, make_node("new"));

        EXPECT_FALSE(root->leaf_);
        EXPECT_EQ(num_children(root), 3);

        check_leaf_heights(root);
    }

    {
        node_ptr root = make_interior_with_leaves("child", max_children - 1);
        root = btree_insert(root, 2, make_node("new 1"));

        EXPECT_EQ(num_children(root), 2);

        check_leaf_heights(root);
    }

    {
        node_ptr root = make_interior_with_leaves("child", max_children);
        root = btree_insert(root, 2, make_node("new 1"));

        EXPECT_EQ(num_children(root), 2);

        check_leaf_heights(root);
    }

    // Check that many inserts maintains balance.
    {
        node_ptr root = make_node("node");

        int const N = 100000;
        for (int i = 0; i < N; ++i) {
            root = btree_insert(root, 2, make_node("new node"));
        }

        check_leaf_heights(root);

        std::cout << "N=" << N << " leaves gives a tree of height "
                  << height_at(root, 0) << "\n";
    }
}

TEST(rope_btree, test_btree_erase_impl)
{
    // TODO
}

TEST(rope_btree, test_btree_erase)
{
    // TODO
}
