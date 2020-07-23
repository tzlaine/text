// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DETAIL_ROPE_HPP
#define BOOST_TEXT_DETAIL_ROPE_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <boost/text/detail/btree.hpp>


namespace boost { namespace text { namespace detail {

    struct rope_tag;

    template<>
    struct reference<rope_tag>
    {
        reference(
            node_ptr<rope_tag> const & string_node, string_view ref) noexcept;

        node_ptr<rope_tag> string_;
        string_view ref_;
    };

    constexpr std::size_t rope_node_buf_size() noexcept
    {
        return max_(
                   alignof(std::string),
                   max_(alignof(string_view), alignof(reference<rope_tag>))) +
               max_(
                   sizeof(std::string),
                   max_(sizeof(string_view), sizeof(reference<rope_tag>)));
    }

    enum class which : char { t, ref };

    constexpr std::size_t string_insert_max = BOOST_TEXT_STRING_INSERT_MAX;

    static_assert(sizeof(node_ptr<detail::rope_tag>) * 8 <= 64, "");

    template<>
    struct leaf_node_t<rope_tag> : node_t<rope_tag>
    {
        leaf_node_t() noexcept : leaf_node_t(string_view()) {}

        leaf_node_t(std::string && t) noexcept :
            node_t(true), buf_ptr_(nullptr), which_(which::t)
        {
            auto at = placement_address<std::string>(buf_, sizeof(buf_));
            BOOST_ASSERT(at);
            buf_ptr_ = new (at) std::string(std::move(t));
        }

        leaf_node_t(string_view tv) noexcept :
            node_t(true), buf_ptr_(nullptr), which_(which::t)
        {
            auto at = placement_address<string_view>(buf_, sizeof(buf_));
            BOOST_ASSERT(at);
            buf_ptr_ = new (at) std::string(tv);
        }

        leaf_node_t(leaf_node_t const & rhs) :
            node_t(true), buf_ptr_(rhs.buf_ptr_), which_(rhs.which_)
        {
            switch (which_) {
            case which::t: {
                auto at = placement_address<std::string>(buf_, sizeof(buf_));
                BOOST_ASSERT(at);
                buf_ptr_ = new (at) std::string(rhs.as_string());
                break;
            }
            case which::ref: {
                auto at =
                    placement_address<reference<rope_tag>>(buf_, sizeof(buf_));
                BOOST_ASSERT(at);
                buf_ptr_ = new (at) reference<rope_tag>(rhs.as_reference());
                break;
            }
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
        }

        leaf_node_t & operator=(leaf_node_t const &) = delete;
        leaf_node_t(leaf_node_t &&) = delete;
        leaf_node_t & operator=(leaf_node_t &&) = delete;

        ~leaf_node_t() noexcept
        {
            if (!buf_ptr_)
                return;

            switch (which_) {
            case which::t: as_string().~basic_string(); break;
            case which::ref: as_reference().~reference(); break;
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
        }

        std::size_t size() const noexcept
        {
            switch (which_) {
            case which::t: return as_string().size(); break;
            case which::ref: return as_reference().ref_.size(); break;
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
            return -std::size_t(1); // This should never execute.
        }

        std::string const & as_string() const noexcept
        {
            BOOST_ASSERT(which_ == which::t);
            return *static_cast<std::string *>(buf_ptr_);
        }

        reference<rope_tag> const & as_reference() const noexcept
        {
            BOOST_ASSERT(which_ == which::ref);
            return *static_cast<reference<rope_tag> *>(buf_ptr_);
        }

        std::string & as_string() noexcept
        {
            BOOST_ASSERT(which_ == which::t);
            return *static_cast<std::string *>(buf_ptr_);
        }

        reference<rope_tag> & as_reference() noexcept
        {
            BOOST_ASSERT(which_ == which::ref);
            return *static_cast<reference<rope_tag> *>(buf_ptr_);
        }

        char buf_[rope_node_buf_size()];
        void * buf_ptr_;
        which which_;
    };

    struct found_char
    {
        found_leaf<rope_tag> leaf_;
        char c_;
    };

    inline void find_char(
        node_ptr<rope_tag> const & node,
        std::size_t n,
        found_char & retval) noexcept
    {
        BOOST_ASSERT(node);
        find_leaf(node, n, retval.leaf_);

        leaf_node_t<rope_tag> const * leaf =
            retval.leaf_.leaf_->as_leaf(); // Heh.
        char c = '\0';
        switch (leaf->which_) {
        case which::t:
            c = *(leaf->as_string().cbegin() + retval.leaf_.offset_);
            break;
        case which::ref:
            c = *(leaf->as_reference().ref_.begin() + retval.leaf_.offset_);
            break;
        default: BOOST_ASSERT(!"unhandled rope node case"); break;
        }
        retval.c_ = c;
    }

    inline reference<rope_tag>::reference(
        node_ptr<rope_tag> const & string_node, string_view ref) noexcept :
        string_(string_node), ref_(ref)
    {
        BOOST_ASSERT(string_node);
        BOOST_ASSERT(string_node->leaf_);
        BOOST_ASSERT(string_node.as_leaf()->which_ == which::t);
    }

    inline node_ptr<rope_tag> make_node(std::string const & t)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(t));
    }

    inline node_ptr<rope_tag> make_node(std::string && t)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(std::move(t)));
    }

    inline node_ptr<rope_tag> make_node(string_view tv)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(tv));
    }

    inline node_ptr<rope_tag> make_node(char const * c_str)
    {
        return make_node(string_view(c_str));
    }

    inline node_ptr<rope_tag> make_ref(
        leaf_node_t<rope_tag> const * t, std::size_t lo, std::size_t hi)
    {
        BOOST_ASSERT(t->which_ == which::t);
        string_view const tv =
            string_view(t->as_string().c_str() + lo, hi - lo);

        leaf_node_t<rope_tag> * leaf = nullptr;
        node_ptr<rope_tag> retval(leaf = new leaf_node_t<rope_tag>);
        leaf->which_ = which::ref;
        auto at = placement_address<reference<rope_tag>>(
            leaf->buf_, sizeof(leaf->buf_));
        BOOST_ASSERT(at);
        leaf->buf_ptr_ =
            new (at) reference<rope_tag>(node_ptr<rope_tag>(t), tv);
        return retval;
    }

    inline node_ptr<rope_tag> make_ref(
        reference<rope_tag> const & t, std::size_t lo, std::size_t hi)
    {
        auto const offset =
            t.ref_.begin() - t.string_.as_leaf()->as_string().c_str();
        node_ptr<rope_tag> retval =
            make_ref(t.string_.as_leaf(), lo + offset, hi + offset);
        return retval;
    }

    inline node_ptr<rope_tag> slice_leaf(
        node_ptr<rope_tag> const & node, std::size_t lo, std::size_t hi)
    {
        BOOST_ASSERT(node);
        BOOST_ASSERT(0 <= lo && lo <= size(node.get()));
        BOOST_ASSERT(0 <= hi && hi <= size(node.get()));
        BOOST_ASSERT(lo < hi);

        node_ptr<rope_tag> retval;

        switch (node.as_leaf()->which_) {
        case which::t:
            BOOST_ASSERT(lo < INT_MAX);
            BOOST_ASSERT(hi < INT_MAX);
            return make_ref(node.as_leaf(), lo, hi);
        case which::ref: {
            BOOST_ASSERT(lo < INT_MAX);
            BOOST_ASSERT(hi < INT_MAX);
            return make_ref(node.as_leaf()->as_reference(), lo, hi);
        }
        default: BOOST_ASSERT(!"unhandled rope node case"); break;
        }
        return retval; // This should never execute.
    }

    inline leaf_slices<rope_tag> erase_leaf(
        node_ptr<rope_tag> const & node, std::size_t lo, std::size_t hi)
    {
        BOOST_ASSERT(node);
        BOOST_ASSERT(0 <= lo && lo <= size(node.get()));
        BOOST_ASSERT(0 <= hi && hi <= size(node.get()));
        BOOST_ASSERT(lo < hi);

        auto const leaf_size = size(node.get());

        leaf_slices<rope_tag> retval;

        if (lo == 0 && hi == leaf_size)
            return retval;

        if (hi != leaf_size)
            retval.other_slice = slice_leaf(node, hi, leaf_size);
        if (lo != 0)
            retval.slice = slice_leaf(node, 0, lo);

        if (!retval.slice)
            retval.slice.swap(retval.other_slice);

        return retval;
    }

    struct segment_inserter
    {
        template<typename Segment>
        void operator()(Segment const & s) const
        {
            if (os_.good())
                os_ << s;
        }

        std::ostream & os_;
    };

}}}

#endif
