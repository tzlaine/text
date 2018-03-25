#include <boost/text/detail/btree.hpp>

#include <gtest/gtest.h>


using namespace boost::text::detail;


inline node_ptr<int>
make_interior_with_leaves(int leaf_size, int leaf_value, int leaves)
{
    interior_node_t<int> * int_node = nullptr;
    node_ptr<int> node(int_node = new_interior_node<int>());
    int_node->children_.push_back(
        make_node(std::vector<int>(leaf_size, leaf_value)));
    int_node->keys_.push_back(size(int_node->children_[0].get()));
    for (int i = 1; i < leaves; ++i) {
        int_node->children_.push_back(
            make_node(std::vector<int>(leaf_size, leaf_value)));
        int_node->keys_.push_back(
            int_node->keys_.back() + size(int_node->children_[i].get()));
    }
    return node;
}

template<int SizeLeft, int SizeCenter, int SizeRight>
node_ptr<int> make_tree_left_center_right()
{
    interior_node_t<int> * int_root = nullptr;
    node_ptr<int> root(int_root = new_interior_node<int>());

    node_ptr<int> left = make_interior_with_leaves(4, 0, SizeLeft);

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    if (SizeCenter != -1) {
        node_ptr<int> center = make_interior_with_leaves(6, 1, SizeCenter);

        int_root->children_.push_back(center);
        int_root->keys_.push_back(int_root->keys_.back() + size(center.get()));
    }

    node_ptr<int> right = make_interior_with_leaves(5, 2, SizeRight);

    int_root->children_.push_back(right);
    int_root->keys_.push_back(int_root->keys_.back() + size(right.get()));

    return root;
}

template<int SizeLeft, int SizeRight>
node_ptr<int> make_tree_left_right()
{
    return make_tree_left_center_right<SizeLeft, -1, SizeRight>();
}

inline node_ptr<int> make_tree_left_max()
{
    return make_tree_left_right<max_children, max_children - 1>();
}

inline node_ptr<int> make_tree_left_min()
{
    return make_tree_left_right<min_children, max_children - 1>();
}


TEST(detail_btree, test_btree_erase_entire_node_leaf_children_extra_ref)
{
    {
        node_ptr<int> root = make_interior_with_leaves(4, 7, 3);
        node_ptr<int> extra_ref = root;
        node_ptr<int> extra_ref_2 = root;

        EXPECT_EQ(num_children(root), 3);

        root = btree_erase(root, 0, children(root)[0].as_leaf(), 0);

        EXPECT_EQ(root->refs_, 1);
        EXPECT_EQ(extra_ref->refs_, 2);

        EXPECT_EQ(keys(root)[0], 4);
        EXPECT_EQ(size(children(root)[0].get()), 4);
        EXPECT_EQ(keys(root)[1], 8);
        EXPECT_EQ(size(children(root)[1].get()), 4);
    }

    {
        node_ptr<int> root = make_interior_with_leaves(4, 7, 3);
        node_ptr<int> extra_ref = root;
        node_ptr<int> extra_ref_2 = root;

        EXPECT_EQ(num_children(root), 3);

        root = btree_erase(root, 4, children(root)[1].as_leaf(), 0);

        EXPECT_EQ(root->refs_, 1);
        EXPECT_EQ(extra_ref->refs_, 2);

        EXPECT_EQ(keys(root)[0], 4);
        EXPECT_EQ(size(children(root)[0].get()), 4);
        EXPECT_EQ(keys(root)[1], 8);
        EXPECT_EQ(size(children(root)[1].get()), 4);
    }

    {
        node_ptr<int> root = make_interior_with_leaves(4, 7, 3);
        node_ptr<int> extra_ref = root;
        node_ptr<int> extra_ref_2 = root;

        EXPECT_EQ(num_children(root), 3);

        root = btree_erase(root, 8, children(root)[2].as_leaf(), 0);

        EXPECT_EQ(root->refs_, 1);
        EXPECT_EQ(extra_ref->refs_, 2);

        EXPECT_EQ(keys(root)[0], 4);
        EXPECT_EQ(size(children(root)[0].get()), 4);
        EXPECT_EQ(keys(root)[1], 8);
        EXPECT_EQ(size(children(root)[1].get()), 4);
    }

    {
        node_ptr<int> root = make_interior_with_leaves(4, 7, 3);
        node_ptr<int> extra_ref = root;
        node_ptr<int> extra_ref_2 = root;

        EXPECT_EQ(num_children(root), 3);

        root = btree_erase(root, 12, children(root)[2].as_leaf(), 0);

        EXPECT_EQ(root->refs_, 1);
        EXPECT_EQ(extra_ref->refs_, 2);

        EXPECT_EQ(keys(root)[0], 4);
        EXPECT_EQ(size(children(root)[0].get()), 4);
        EXPECT_EQ(keys(root)[1], 8);
        EXPECT_EQ(size(children(root)[1].get()), 4);
    }

    {
        node_ptr<int> root;
        {
            interior_node_t<int> * int_root = nullptr;
            root = node_ptr<int>(int_root = new_interior_node<int>());
            int_root->children_.push_back(make_node(std::vector<int>(4, 4)));
            int_root->keys_.push_back(size(int_root->children_[0].get()));
            int_root->children_.push_back(make_node(std::vector<int>(5, 5)));
            int_root->keys_.push_back(
                int_root->keys_.back() + size(int_root->children_[1].get()));
        }
        node_ptr<int> extra_ref = root;

        EXPECT_EQ(num_children(root), 2);

        root = btree_erase(root, 0, children(root)[0].as_leaf(), 0);

        EXPECT_NE(root.get(), extra_ref.get());

        EXPECT_TRUE(root->leaf_);
        EXPECT_EQ(size(root.get()), 5);
    }

    {
        node_ptr<int> root;
        {
            interior_node_t<int> * int_root = nullptr;
            root = node_ptr<int>(int_root = new_interior_node<int>());
            int_root->children_.push_back(make_node(std::vector<int>(4, 4)));
            int_root->keys_.push_back(size(int_root->children_[0].get()));
            int_root->children_.push_back(make_node(std::vector<int>(5, 5)));
            int_root->keys_.push_back(
                int_root->keys_.back() + size(int_root->children_[1].get()));
        }
        node_ptr<int> extra_ref = root;

        EXPECT_EQ(num_children(root), 2);

        root = btree_erase(root, 4, children(root)[1].as_leaf(), 0);

        EXPECT_NE(root.get(), extra_ref.get());

        EXPECT_TRUE(root->leaf_);
        EXPECT_EQ(size(root.get()), 4);
    }

    {
        node_ptr<int> root;
        {
            interior_node_t<int> * int_root = nullptr;
            root = node_ptr<int>(int_root = new_interior_node<int>());
            int_root->children_.push_back(make_node(std::vector<int>(4, 4)));
            int_root->keys_.push_back(size(int_root->children_[0].get()));
            int_root->children_.push_back(make_node(std::vector<int>(5, 5)));
            int_root->keys_.push_back(
                int_root->keys_.back() + size(int_root->children_[1].get()));
        }
        node_ptr<int> extra_ref = root;

        EXPECT_EQ(num_children(root), 2);

        root = btree_erase(root, 9, children(root)[1].as_leaf(), 0);

        EXPECT_NE(root.get(), extra_ref.get());

        EXPECT_TRUE(root->leaf_);
        EXPECT_EQ(size(root.get()), 4);
    }
}

TEST(detail_btree, test_btree_erase_entire_node_interior_children)
{
    // Last interior node has more than min children.
    {
        node_ptr<int> root = make_tree_left_min();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        root = btree_erase(
            root, min_children * 4, children(right)[0].as_leaf(), 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 2);

        EXPECT_EQ(keys(left).back(), min_children * 4);
        EXPECT_EQ(keys(right).back(), (max_children - 2) * 5);
    }

    {
        node_ptr<int> root = make_tree_left_min();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        root = btree_erase(
            root,
            min_children * 4 + (max_children - 1) * 5,
            children(right).back().as_leaf(),
            0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 2);

        EXPECT_EQ(keys(left).back(), min_children * 4);
        EXPECT_EQ(keys(right).back(), (max_children - 2) * 5);
    }


    // Last interior node min children, left has min children.
    {
        node_ptr<int> root = make_tree_left_right<min_children, max_children>();

        auto const root_initial_size = size(root.get());

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children);

        root = btree_erase(root, 0, children(left).front().as_leaf(), 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        EXPECT_EQ(keys(root)[0], (min_children - 1) * 4 + 5);
        EXPECT_EQ(keys(root)[1], root_initial_size - 4);
        EXPECT_EQ(keys(left).back(), (min_children - 1) * 4 + 5);
        EXPECT_EQ(keys(right).back(), (max_children - 1) * 5);
    }

    {
        node_ptr<int> root = make_tree_left_right<min_children, max_children>();

        auto const root_initial_size = size(root.get());

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children);

        root = btree_erase(
            root, min_children * 4 - 1, children(left).back().as_leaf(), 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        EXPECT_EQ(keys(root)[0], (min_children - 1) * 4 + 5);
        EXPECT_EQ(keys(root)[1], root_initial_size - 4);
        EXPECT_EQ(keys(left).back(), (min_children - 1) * 4 + 5);
        EXPECT_EQ(keys(right).back(), (max_children - 1) * 5);
    }


    // Last interior node min children, right has min children.
    {
        node_ptr<int> root = make_tree_left_right<max_children, min_children>();

        auto const root_initial_size = size(root.get());

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), min_children);

        root = btree_erase(
            root, size(root.get()), children(right).back().as_leaf(), 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), max_children - 1);
        EXPECT_EQ(num_children(right), min_children);

        EXPECT_EQ(keys(root)[0], (max_children - 1) * 4);
        EXPECT_EQ(keys(root)[1], root_initial_size - 5);
        EXPECT_EQ(keys(left).back(), (max_children - 1) * 4);
        EXPECT_EQ(keys(right).back(), 4 + (min_children - 1) * 5);
    }

    {
        node_ptr<int> root = make_tree_left_right<max_children, min_children>();

        auto const root_initial_size = size(root.get());

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), min_children);

        root = btree_erase(
            root, max_children * 4, children(right).front().as_leaf(), 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), max_children - 1);
        EXPECT_EQ(num_children(right), min_children);

        EXPECT_EQ(keys(root)[0], (max_children - 1) * 4);
        EXPECT_EQ(keys(root)[1], root_initial_size - 5);
        EXPECT_EQ(keys(left).back(), (max_children - 1) * 4);
        EXPECT_EQ(keys(right).back(), 4 + (min_children - 1) * 5);
    }


    // Last interior node min children, both sides have min children.
    {
        node_ptr<int> root = make_tree_left_right<min_children, min_children>();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), min_children);

        root = btree_erase(
            root, size(root.get()), children(right).back().as_leaf(), 0);

        EXPECT_EQ(num_children(root), max_children - 1);

        int i = 0;
        std::ptrdiff_t sz = 0;
        for (; i < min_children - 1; ++i) {
            sz += size(children(root)[i].get());
            EXPECT_EQ(keys(root)[i], sz) << "i=" << i;
        }
        for (; i < max_children - 1; ++i) {
            sz += size(children(root)[i].get());
            EXPECT_EQ(keys(root)[i], sz) << "i=" << i;
        }
    }

    {
        node_ptr<int> root = make_tree_left_right<min_children, min_children>();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), min_children);

        root = btree_erase(root, 0, children(left).front().as_leaf(), 0);

        EXPECT_EQ(num_children(root), max_children - 1);

        int i = 0;
        std::ptrdiff_t sz = 0;
        for (; i < min_children; ++i) {
            sz += size(children(root)[i].get());
            EXPECT_EQ(keys(root)[i], sz) << "i=" << i;
        }
        for (; i < max_children - 1; ++i) {
            sz += size(children(root)[i].get());
            EXPECT_EQ(keys(root)[i], sz) << "i=" << i;
        }
    }


    // Last interior node min children, all three children have min children.
    {
        node_ptr<int> root = make_tree_left_center_right<
            min_children,
            min_children,
            min_children>();

        auto const root_initial_size = size(root.get());

        {
            node_ptr<int> const & left = children(root)[0];
            node_ptr<int> const & center = children(root)[1];
            node_ptr<int> const & right = children(root)[2];

            EXPECT_EQ(num_children(left), min_children);
            EXPECT_EQ(num_children(center), min_children);
            EXPECT_EQ(num_children(right), min_children);

            root = btree_erase(root, 0, children(left).front().as_leaf(), 0);
        }

        EXPECT_EQ(num_children(root), 2);

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children - 1);
        EXPECT_EQ(num_children(right), min_children);
        EXPECT_EQ(keys(root)[0], (min_children - 1) * 4 + min_children * 6);
        EXPECT_EQ(keys(root)[1], root_initial_size - 4);
        EXPECT_EQ(keys(left).back(), (min_children - 1) * 4 + min_children * 6);
        EXPECT_EQ(keys(right).back(), min_children * 5);
    }

    {
        node_ptr<int> root = make_tree_left_center_right<
            min_children,
            min_children,
            min_children>();

        auto const root_initial_size = size(root.get());

        {
            node_ptr<int> const & left = children(root)[0];
            node_ptr<int> const & center = children(root)[1];
            node_ptr<int> const & right = children(root)[2];

            EXPECT_EQ(num_children(left), min_children);
            EXPECT_EQ(num_children(center), min_children);
            EXPECT_EQ(num_children(right), min_children);

            root = btree_erase(
                root, min_children * 4, children(center).front().as_leaf(), 0);
        }

        EXPECT_EQ(num_children(root), 2);

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children - 1);
        EXPECT_EQ(num_children(right), min_children);
        EXPECT_EQ(keys(root)[0], min_children * 4 + (min_children - 1) * 6);
        EXPECT_EQ(keys(root)[1], root_initial_size - 6);
        EXPECT_EQ(keys(left).back(), min_children * 4 + (min_children - 1) * 6);
        EXPECT_EQ(keys(right).back(), min_children * 5);
    }

    {
        node_ptr<int> root = make_tree_left_center_right<
            min_children,
            min_children,
            min_children>();

        auto const root_initial_size = size(root.get());

        {
            node_ptr<int> const & left = children(root)[0];
            node_ptr<int> const & center = children(root)[1];
            node_ptr<int> const & right = children(root)[2];

            EXPECT_EQ(num_children(left), min_children);
            EXPECT_EQ(num_children(center), min_children);
            EXPECT_EQ(num_children(right), min_children);

            root = btree_erase(
                root, size(root.get()), children(right).back().as_leaf(), 0);
        }

        EXPECT_EQ(num_children(root), 2);

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), min_children);
        EXPECT_EQ(num_children(right), max_children - 1);
        EXPECT_EQ(keys(root)[0], min_children * 4);
        EXPECT_EQ(keys(root)[1], root_initial_size - 5);
        EXPECT_EQ(keys(left).back(), min_children * 4);
        EXPECT_EQ(
            keys(right).back(), min_children * 6 + (min_children - 1) * 5);
    }
}

TEST(detail_btree, test_btree_erase)
{
    // Erasure from a leaf node
    {
        node_ptr<int> root = make_node(std::vector<int>(9, 9));

        root = btree_erase(root, 0, 9, 0);

        EXPECT_EQ(root.get(), nullptr);
    }

    {
        node_ptr<int> root = make_node(std::vector<int>(9, 9));

        root = btree_erase(root, 0, 8, 0);

        EXPECT_TRUE(root->leaf_);
        EXPECT_EQ(size(root.get()), 1);
    }

#if 0 // This test expects to split the node, but that doesn't really work
      // with std::vector.
    {
        node_ptr<int> root = make_node(std::vector<int>(9, 9));

        root = btree_erase(root, 1, 8, 0);

        EXPECT_EQ(size(root.get()), 2);
        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(keys(root)[0], 1);
        EXPECT_EQ(size(children(root)[1].get()), 1);
        EXPECT_EQ(keys(root)[1], 2);
        EXPECT_EQ(size(children(root)[1].get()), 1);
    }
#endif

    // Erasure from non-leaf nodes, entire segments only

    {
        node_ptr<int> root = make_tree_left_right<max_children, max_children>();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), max_children);

        root = btree_erase(root, (max_children - 1) * 4, max_children * 4, 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), max_children - 1);
        EXPECT_EQ(num_children(right), max_children);

        EXPECT_EQ(keys(root)[0], (max_children - 1) * 4);
        EXPECT_EQ(keys(root)[1], (max_children - 1) * 4 + max_children * 5);
        EXPECT_EQ(keys(left).back(), (max_children - 1) * 4);
        EXPECT_EQ(keys(right).back(), max_children * 5);
    }

    {
        node_ptr<int> root = make_tree_left_right<max_children, max_children>();

        node_ptr<int> const & left = children(root)[0];
        node_ptr<int> const & right = children(root)[1];

        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), max_children);

        root = btree_erase(root, max_children * 4, max_children * 4 + 5, 0);

        EXPECT_EQ(num_children(root), 2);
        EXPECT_EQ(num_children(left), max_children);
        EXPECT_EQ(num_children(right), max_children - 1);

        EXPECT_EQ(keys(root)[0], max_children * 4);
        EXPECT_EQ(keys(root)[1], max_children * 4 + (max_children - 1) * 5);
        EXPECT_EQ(keys(left).back(), max_children * 4);
        EXPECT_EQ(keys(right).back(), (max_children - 1) * 5);
    }
}
