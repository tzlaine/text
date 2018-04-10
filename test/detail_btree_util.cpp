#include <boost/text/detail/btree.hpp>

#include <gtest/gtest.h>


using namespace boost::text::detail;

TEST(detail_btree_util, test_node_ptr)
{
    {
        node_ptr<int> p0(new_interior_node<int>());
        node_ptr<int> p1 = p0;

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
        node_ptr<int> p0(new leaf_node_t<int>);
        node_ptr<int> p1 = p0;

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_EQ(p1->refs_, 2);

        EXPECT_EQ(p0.as_leaf()->refs_, 2);
        EXPECT_EQ(p0.as_leaf()->leaf_, true);
        EXPECT_EQ(p0.as_leaf()->size(), 0);

        EXPECT_EQ(size(p0.get()), 0);
    }
}

TEST(detail_btree_util, test_make_node)
{
    {
        std::vector<int> v(9, 3);
        node_ptr<int> p = make_node(v);

        EXPECT_EQ(size(p.get()), v.size());
        EXPECT_EQ(p.as_leaf()->as_vec(), v);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p = make_node(std::move(v));

        EXPECT_EQ(size(p.get()), 9);
        EXPECT_EQ(v.size(), 0);
        EXPECT_EQ(p.as_leaf()->as_vec(), std::vector<int>(9, 3));
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p_text = make_node(v);

        EXPECT_EQ(size(p_text.get()), v.size());
        EXPECT_EQ(p_text.as_leaf()->as_vec(), v);

        {
            node_ptr<int> p_ref0 = make_ref(p_text.as_leaf(), 1, 8);

            EXPECT_EQ(size(p_ref0.get()), 7);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().lo_, 1);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().hi_, 8);

            EXPECT_EQ(p_text->refs_, 2);
            EXPECT_EQ(p_ref0.as_leaf()->as_reference().vec_->refs_, 2);
            EXPECT_EQ(p_ref0->refs_, 1);

            node_ptr<int> p_ref1 =
                make_ref(p_ref0.as_leaf()->as_reference(), 1, 6);

            EXPECT_EQ(size(p_ref1.get()), 5);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().lo_, 2);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().hi_, 7);

            EXPECT_EQ(p_text->refs_, 3);
            EXPECT_EQ(p_ref1.as_leaf()->as_reference().vec_->refs_, 3);
            EXPECT_EQ(p_ref0->refs_, 1);
            EXPECT_EQ(p_ref1->refs_, 1);
        }

        EXPECT_EQ(p_text->refs_, 1);
    }
}

TEST(detail_btree_util_, test_mutable_node_ptr)
{
    {
        std::vector<int> v(9, 3);
        node_ptr<int> p_text = make_node(v);

        auto mut_p_text = p_text.write();

        EXPECT_EQ(p_text->refs_, 1);
        EXPECT_EQ(mut_p_text.as_leaf(), p_text.as_leaf());

        mut_p_text.as_leaf()->as_vec().push_back(3);

        EXPECT_EQ(p_text.as_leaf()->as_vec(), std::vector<int>(10, 3));
        EXPECT_EQ(mut_p_text.as_leaf()->as_vec(), std::vector<int>(10, 3));
    }

    std::vector<int> v(9, 3);
    node_ptr<int> p0 = make_node(v);
    node_ptr<int> p1 = p0;

    {
        auto mut_p0 = p0.write();

        EXPECT_EQ(p0->refs_, 2);
        EXPECT_NE(mut_p0.as_leaf(), p0.as_leaf());

        mut_p0.as_leaf()->as_vec().push_back(3);
        mut_p0.as_leaf()->as_vec().push_back(3);

        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(mut_p0.as_leaf()->as_vec(), std::vector<int>(11, 3));
    }

    EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(11, 3));
}

node_ptr<int> make_tree()
{
    interior_node_t<int> * int_root = nullptr;
    node_ptr<int> root(int_root = new_interior_node<int>());

    interior_node_t<int> * int_left = nullptr;
    node_ptr<int> left(int_left = new_interior_node<int>());
    int_left->children_.push_back(make_node(std::vector<int>(9, 0)));
    int_left->keys_.push_back(size(int_left->children_[0].get()));
    int_left->children_.push_back(make_node(std::vector<int>(10, 1)));
    int_left->keys_.push_back(
        int_left->keys_[0] + size(int_left->children_[1].get()));

    int_root->children_.push_back(left);
    int_root->keys_.push_back(size(left.get()));

    interior_node_t<int> * int_right = nullptr;
    node_ptr<int> right(int_right = new_interior_node<int>());
    int_right->children_.push_back(make_node(std::vector<int>(10, 2)));
    int_right->keys_.push_back(size(int_right->children_[0].get()));
    int_right->children_.push_back(make_node(std::vector<int>(11, 3)));
    int_right->keys_.push_back(
        int_right->keys_[0] + size(int_right->children_[1].get()));

    int_root->children_.push_back(right);
    int_root->keys_.push_back(int_root->keys_[0] + size(right.get()));

    return root;
}

TEST(detail_btree_util, test_find)
{
    // find_child

    {
        interior_node_t<int> parent;
        parent.children_.push_back(make_node(std::vector<int>({0, 1, 2, 3})));
        parent.children_.push_back(make_node(std::vector<int>(1, 4)));
        parent.children_.push_back(make_node(std::vector<int>({5, 6, 7, 8})));
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
        node_ptr<int> root = make_node(std::vector<int>({0, 1, 2, 3}));
        found_leaf<int> found;

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
        interior_node_t<int> * int_root = nullptr;
        node_ptr<int> root(int_root = new_interior_node<int>());

        interior_node_t<int> * int_left = nullptr;
        node_ptr<int> left(int_left = new_interior_node<int>());
        int_left->children_.push_back(make_node(std::vector<int>(9, 0)));
        int_left->keys_.push_back(size(int_left->children_[0].get()));
        int_left->children_.push_back(make_node(std::vector<int>(10, 1)));
        int_left->keys_.push_back(
            int_left->keys_[0] + size(int_left->children_[1].get()));

        int_root->children_.push_back(left);
        int_root->keys_.push_back(size(left.get()));

        interior_node_t<int> * int_right = nullptr;
        node_ptr<int> right(int_right = new_interior_node<int>());
        int_right->children_.push_back(make_node(std::vector<int>(10, 2)));
        int_right->keys_.push_back(size(int_right->children_[0].get()));
        int_right->children_.push_back(make_node(std::vector<int>(11, 3)));
        int_right->keys_.push_back(
            int_right->keys_[0] + size(int_right->children_[1].get()));

        int_root->children_.push_back(right);
        int_root->keys_.push_back(int_root->keys_[0] + size(right.get()));

        {
            found_leaf<int> found;
            find_leaf(root, 0, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_vec(), std::vector<int>(9, 0));
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 8, found);
            EXPECT_EQ(found.leaf_->as_leaf()->as_vec(), std::vector<int>(9, 0));
            EXPECT_EQ(found.offset_, 8);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 9, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 1));
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 10, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 1));
            EXPECT_EQ(found.offset_, 1);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 13, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 1));
            EXPECT_EQ(found.offset_, 4);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 18, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 1));
            EXPECT_EQ(found.offset_, 9);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_left);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 19, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 2));
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 28, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(10, 2));
            EXPECT_EQ(found.offset_, 9);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 29, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(11, 3));
            EXPECT_EQ(found.offset_, 0);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }

        {
            found_leaf<int> found;
            find_leaf(root, 40, found);
            EXPECT_EQ(
                found.leaf_->as_leaf()->as_vec(), std::vector<int>(11, 3));
            EXPECT_EQ(found.offset_, 11);
            EXPECT_EQ(found.path_.size(), 2);
            EXPECT_EQ(found.path_[0], int_root);
            EXPECT_EQ(found.path_[1], int_right);
        }
    }

    // find_element

    {
        node_ptr<int> root = make_tree();
        found_element<int> found;

        find_element(root, 0, found);
        EXPECT_EQ(*found.element_, 0);
        find_element(root, 8, found);
        EXPECT_EQ(*found.element_, 0);
        find_element(root, 9, found);
        EXPECT_EQ(*found.element_, 1);
        find_element(root, 10, found);
        EXPECT_EQ(*found.element_, 1);
        find_element(root, 13, found);
        EXPECT_EQ(*found.element_, 1);
        find_element(root, 18, found);
        EXPECT_EQ(*found.element_, 1);
        find_element(root, 19, found);
        EXPECT_EQ(*found.element_, 2);
        find_element(root, 28, found);
        EXPECT_EQ(*found.element_, 2);
        find_element(root, 29, found);
        EXPECT_EQ(*found.element_, 3);
    }
}

void fill_interior_node(interior_node_t<int> & parent)
{
    parent.children_.push_back(make_node(std::vector<int>({0, 1, 2, 3})));
    parent.children_.push_back(make_node(std::vector<int>(1, 4)));
    parent.children_.push_back(make_node(std::vector<int>({5, 6, 7, 8})));
    parent.keys_.push_back(4);
    parent.keys_.push_back(5);
    parent.keys_.push_back(9);
}

TEST(detail_btree_util_, test_insert_erase_child)
{
    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        insert_child(&parent, 0, make_node(std::vector<int>(1, 10)));
        EXPECT_EQ(
            parent.children_[0].as_leaf()->as_vec(), std::vector<int>(1, 10));
        EXPECT_EQ(parent.keys_[0], 1);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        insert_child(&parent, 2, make_node(std::vector<int>(1, 10)));
        EXPECT_EQ(
            parent.children_[2].as_leaf()->as_vec(), std::vector<int>(1, 10));
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 6);
        EXPECT_EQ(parent.keys_[3], 10);
    }


    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        insert_child(&parent, 3, make_node(std::vector<int>(1, 10)));
        EXPECT_EQ(
            parent.children_[3].as_leaf()->as_vec(), std::vector<int>(1, 10));
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
        EXPECT_EQ(parent.keys_[2], 9);
        EXPECT_EQ(parent.keys_[3], 10);
    }

    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        erase_child(&parent, 0, dont_adjust_keys);
        EXPECT_EQ(
            parent.children_[0].as_leaf()->as_vec(), std::vector<int>(1, 4));
        EXPECT_EQ(
            parent.children_[1].as_leaf()->as_vec(),
            std::vector<int>({5, 6, 7, 8}));
        EXPECT_EQ(parent.keys_[0], 5);
        EXPECT_EQ(parent.keys_[1], 9);
    }

    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        erase_child(&parent, 1);
        EXPECT_EQ(
            parent.children_[0].as_leaf()->as_vec(),
            std::vector<int>({0, 1, 2, 3}));
        EXPECT_EQ(
            parent.children_[1].as_leaf()->as_vec(),
            std::vector<int>({5, 6, 7, 8}));
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 8);
    }

    {
        interior_node_t<int> parent;
        fill_interior_node(parent);
        erase_child(&parent, 2);
        EXPECT_EQ(
            parent.children_[0].as_leaf()->as_vec(),
            std::vector<int>({0, 1, 2, 3}));
        EXPECT_EQ(
            parent.children_[1].as_leaf()->as_vec(), std::vector<int>(1, 4));
        EXPECT_EQ(parent.keys_[0], 4);
        EXPECT_EQ(parent.keys_[1], 5);
    }
}

TEST(detail_btree_util_0, test_slice_leaf)
{
    // text

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = slice_leaf(p0, 0, v.size(), true);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(p1.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(p1.as_leaf()->as_reference().hi_, 9);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = slice_leaf(p0, 0, v.size(), false);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(p1.as_leaf()->as_vec(), std::vector<int>(9, 3));
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        slice_leaf(p0, 1, v.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(7, 3));
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = p0;
        node_ptr<int> p2 = slice_leaf(p0, 1, v.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(p2.as_leaf()->as_reference().lo_, 1);
        EXPECT_EQ(p2.as_leaf()->as_reference().hi_, 8);
    }

    // reference

    {
        std::vector<int> v(9, 3);
        node_ptr<int> pt = make_node(v);

        node_ptr<int> p0 = slice_leaf(pt, 0, v.size(), true);
        node_ptr<int> p1 = slice_leaf(p0, 0, v.size(), true);
        EXPECT_EQ(p0.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(p0.as_leaf()->as_reference().hi_, 9);
        EXPECT_EQ(p1.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(p1.as_leaf()->as_reference().hi_, 9);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> pt = make_node(v);

        node_ptr<int> p0 = slice_leaf(pt, 0, v.size(), true);
        slice_leaf(p0, 1, v.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_reference().lo_, 1);
        EXPECT_EQ(p0.as_leaf()->as_reference().hi_, 8);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> pt = make_node(v);

        node_ptr<int> p0 = slice_leaf(pt, 0, v.size(), true);
        node_ptr<int> p1 = p0;
        node_ptr<int> p2 = slice_leaf(p0, 1, v.size() - 1, false);
        EXPECT_EQ(p0.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(p0.as_leaf()->as_reference().hi_, 9);
        EXPECT_EQ(p2.as_leaf()->as_reference().lo_, 1);
        EXPECT_EQ(p2.as_leaf()->as_reference().hi_, 8);
    }
}

TEST(detail_btree_util_, test_erase_leaf)
{
    // text

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        leaf_slices<int> slices = erase_leaf(p0, 0, 9);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(slices.slice.get(), nullptr);
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        leaf_slices<int> slices = erase_leaf(p0, 1, 9);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(1, 3));
        EXPECT_EQ(slices.slice.as_leaf()->as_vec(), std::vector<int>(1, 3));
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = p0;
        leaf_slices<int> slices = erase_leaf(p0, 1, 9);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().hi_, 1);
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = p0;
        leaf_slices<int> slices = erase_leaf(p0, 0, 8);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().lo_, 8);
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().hi_, 9);
        EXPECT_EQ(slices.other_slice.get(), nullptr);
    }

    {
        std::vector<int> v(9, 3);
        node_ptr<int> p0 = make_node(v);
        node_ptr<int> p1 = p0;
        leaf_slices<int> slices = erase_leaf(p0, 1, 8);
        EXPECT_EQ(p0.as_leaf()->as_vec(), std::vector<int>(9, 3));
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().lo_, 0);
        EXPECT_EQ(slices.slice.as_leaf()->as_reference().hi_, 1);
        EXPECT_EQ(slices.other_slice.as_leaf()->as_reference().lo_, 8);
        EXPECT_EQ(slices.other_slice.as_leaf()->as_reference().hi_, 9);
    }
}
