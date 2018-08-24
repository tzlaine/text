#ifndef BOOST_TEXT_DETAIL_ROPE_HPP
#define BOOST_TEXT_DETAIL_ROPE_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utf8.hpp>

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

    constexpr int rope_node_buf_size() noexcept
    {
        return max_(
                   alignof(string),
                   max_(
                       alignof(string_view),
                       max_(
                           alignof(repeated_string_view),
                           alignof(reference<rope_tag>)))) +
               max_(
                   sizeof(string),
                   max_(
                       sizeof(string_view),
                       max_(
                           sizeof(repeated_string_view),
                           sizeof(reference<rope_tag>))));
    }

    enum class which : char { t, rtv, ref };

    constexpr int string_insert_max = BOOST_TEXT_STRING_INSERT_MAX;

    static_assert(sizeof(node_ptr<detail::rope_tag>) * 8 <= 64, "");

    template<>
    struct leaf_node_t<rope_tag> : node_t<rope_tag>
    {
        leaf_node_t() noexcept : leaf_node_t(string_view()) {}

        leaf_node_t(string && t) noexcept :
            node_t(true),
            buf_ptr_(nullptr),
            which_(which::t)
        {
            auto at = placement_address<string>(buf_, sizeof(buf_));
            BOOST_ASSERT(at);
            buf_ptr_ = new (at) string(std::move(t));
        }

        leaf_node_t(string_view tv) noexcept :
            node_t(true),
            buf_ptr_(nullptr),
            which_(which::t)
        {
            auto at = placement_address<string_view>(buf_, sizeof(buf_));
            BOOST_ASSERT(at);
            buf_ptr_ = new (at) string(tv);
        }

        leaf_node_t(repeated_string_view rtv) noexcept :
            node_t(true),
            buf_ptr_(nullptr),
            which_(which::rtv)
        {
            auto at =
                placement_address<repeated_string_view>(buf_, sizeof(buf_));
            BOOST_ASSERT(at);
            buf_ptr_ = new (at) repeated_string_view(rtv);
        }

        leaf_node_t(leaf_node_t const & rhs) :
            node_t(true),
            buf_ptr_(rhs.buf_ptr_),
            which_(rhs.which_)
        {
            switch (which_) {
            case which::t: {
                auto at = placement_address<string>(buf_, sizeof(buf_));
                BOOST_ASSERT(at);
                buf_ptr_ = new (at) string(rhs.as_string());
                break;
            }
            case which::rtv: {
                auto at =
                    placement_address<repeated_string_view>(buf_, sizeof(buf_));
                BOOST_ASSERT(at);
                buf_ptr_ = new (at)
                    repeated_string_view(rhs.as_repeated_string_view());
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
            case which::t: as_string().~string(); break;
            case which::rtv:
                as_repeated_string_view().~repeated_string_view();
                break;
            case which::ref: as_reference().~reference(); break;
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
        }

        int size() const noexcept
        {
            switch (which_) {
            case which::t: return as_string().size(); break;
            case which::rtv: return as_repeated_string_view().size(); break;
            case which::ref: return as_reference().ref_.size(); break;
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
            return -(1 << 30); // This should never execute.
        }

        string const & as_string() const noexcept
        {
            BOOST_ASSERT(which_ == which::t);
            return *static_cast<string *>(buf_ptr_);
        }

        repeated_string_view const & as_repeated_string_view() const noexcept
        {
            BOOST_ASSERT(which_ == which::rtv);
            return *static_cast<repeated_string_view *>(buf_ptr_);
        }

        reference<rope_tag> const & as_reference() const noexcept
        {
            BOOST_ASSERT(which_ == which::ref);
            return *static_cast<reference<rope_tag> *>(buf_ptr_);
        }

        string & as_string() noexcept
        {
            BOOST_ASSERT(which_ == which::t);
            return *static_cast<string *>(buf_ptr_);
        }

        repeated_string_view & as_repeated_string_view() noexcept
        {
            BOOST_ASSERT(which_ == which::rtv);
            return *static_cast<repeated_string_view *>(buf_ptr_);
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
        std::ptrdiff_t n,
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
        case which::rtv:
            c = *(
                leaf->as_repeated_string_view().begin() + retval.leaf_.offset_);
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
        string_(string_node),
        ref_(ref)
    {
        BOOST_ASSERT(string_node);
        BOOST_ASSERT(string_node->leaf_);
        BOOST_ASSERT(string_node.as_leaf()->which_ == which::t);
    }

    inline node_ptr<rope_tag> make_node(string const & t)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(t));
    }

    inline node_ptr<rope_tag> make_node(string && t)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(std::move(t)));
    }

    inline node_ptr<rope_tag> make_node(string_view tv)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(tv));
    }

    inline node_ptr<rope_tag> make_node(repeated_string_view rtv)
    {
        return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(rtv));
    }

    inline node_ptr<rope_tag> make_node(char const * c_str)
    {
        return make_node(string_view(c_str));
    }

    inline node_ptr<rope_tag> make_ref(
        leaf_node_t<rope_tag> const * t, std::ptrdiff_t lo, std::ptrdiff_t hi)
    {
        BOOST_ASSERT(t->which_ == which::t);
        string_view const tv =
            string_view(t->as_string().begin() + lo, hi - lo);

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
        reference<rope_tag> const & t, std::ptrdiff_t lo, std::ptrdiff_t hi)
    {
        auto const offset =
            t.ref_.begin() - t.string_.as_leaf()->as_string().begin();
        node_ptr<rope_tag> retval =
            make_ref(t.string_.as_leaf(), lo + offset, hi + offset);
        return retval;
    }

    inline node_ptr<rope_tag> slice_leaf(
        node_ptr<rope_tag> const & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        bool immutable)
    {
        BOOST_ASSERT(node);
        BOOST_ASSERT(0 <= lo && lo <= size(node.get()));
        BOOST_ASSERT(0 <= hi && hi <= size(node.get()));
        BOOST_ASSERT(lo < hi);

        bool const leaf_mutable = !immutable && node->refs_ == 1;

        switch (node.as_leaf()->which_) {
        case which::t:
            BOOST_ASSERT(lo < INT_MAX);
            BOOST_ASSERT(hi < INT_MAX);
            if (!leaf_mutable)
                return make_ref(node.as_leaf(), lo, hi);
            {
                auto mut_node = node.write();
                string & t = mut_node.as_leaf()->as_string();
                t = string_view(t.begin() + lo, hi - lo);
            }
            return node;
        case which::rtv: {
            repeated_string_view const & crtv =
                node.as_leaf()->as_repeated_string_view();
            int const mod_lo = lo % crtv.view().size();
            int const mod_hi = hi % crtv.view().size();
            if (mod_lo != 0 || mod_hi != 0) {
                return make_node(string(crtv.begin() + lo, crtv.begin() + hi));
            } else {
                auto const count = (hi - lo) / crtv.view().size();
                if (!leaf_mutable)
                    return make_node(repeated_string_view(crtv.view(), count));
                auto mut_node = node.write();
                repeated_string_view & rtv =
                    mut_node.as_leaf()->as_repeated_string_view();
                rtv = repeated_string_view(rtv.view(), count);
            }
            return node;
        }
        case which::ref: {
            BOOST_ASSERT(lo < INT_MAX);
            BOOST_ASSERT(hi < INT_MAX);
            if (!leaf_mutable)
                return make_ref(node.as_leaf()->as_reference(), lo, hi);
            {
                auto mut_node = node.write();
                reference<rope_tag> & ref = mut_node.as_leaf()->as_reference();
                ref.ref_ = string_view(ref.ref_.begin() + lo, hi - lo);
            }
            return node;
        }
        default: BOOST_ASSERT(!"unhandled rope node case"); break;
        }
        return node_ptr<rope_tag>(); // This should never execute.
    }

    inline leaf_slices<rope_tag>
    erase_leaf(node_ptr<rope_tag> & node, std::ptrdiff_t lo, std::ptrdiff_t hi)
    {
        BOOST_ASSERT(node);
        BOOST_ASSERT(0 <= lo && lo <= size(node.get()));
        BOOST_ASSERT(0 <= hi && hi <= size(node.get()));
        BOOST_ASSERT(lo < hi);

        bool const leaf_mutable = node.as_leaf()->refs_ == 1;
        auto const leaf_size = size(node.get());

        leaf_slices<rope_tag> retval;

        if (lo == 0 && hi == leaf_size)
            return retval;

        if (leaf_mutable && node.as_leaf()->which_ == which::t) {
            {
                auto mut_node = node.write();
                string & t = mut_node.as_leaf()->as_string();
                t.erase(string_view(t.begin() + lo, hi - lo));
            }
            retval.slice = node;
            return retval;
        }

        if (hi != leaf_size)
            retval.other_slice = slice_leaf(node, hi, leaf_size, true);
        if (lo != 0)
            retval.slice = slice_leaf(node, 0, lo, false);

        if (!retval.slice)
            retval.slice.swap(retval.other_slice);

        return retval;
    }

    inline bool child_immutable(node_ptr<rope_tag> const & node)
    {
        return node.as_leaf()->which_ == which::t;
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

    template<typename Segment>
    bool encoded(Segment const & segment)
    {
        return utf8::encoded(segment.begin(), segment.end());
    }

    inline bool encoded(repeated_string_view rtv)
    {
        return utf8::encoded(rtv.view().begin(), rtv.view().end());
    }

    struct segment_encoding_checker
    {
        template<typename Segment>
        void operator()(Segment const & s) const
        {
            if (!encoded(s))
                throw std::invalid_argument("Invalid UTF-8 encoding");
        }
    };

    struct repeated_range
    {
        repeated_string_view::const_iterator first, last;
        repeated_string_view::const_iterator begin() const { return first; }
        repeated_string_view::const_iterator end() const { return last; }
    };

    inline std::ostream & operator<<(std::ostream & os, repeated_range rr)
    {
        for (char c : rr) {
            os << c;
        }
        return os;
    }

}}}

#endif
