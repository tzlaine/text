#ifndef BOOST_TEXT_DETAIL_ROPE_HPP
#define BOOST_TEXT_DETAIL_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/text/detail/btree.hpp>


namespace boost { namespace text { namespace detail {

    // TODO: Experiment with collapsing adjacent text_views, references, etc.,
    // when inserting, erasing, etc.

    struct rope_tag;

    template <>
    struct reference<rope_tag>
    {
        reference (node_ptr<rope_tag> const & text_node, text_view ref) noexcept;

        node_ptr<rope_tag> text_;
        text_view ref_;
    };

    constexpr int rope_node_buf_size () noexcept
    {
        return
            max_(alignof(text),
                 max_(alignof(text_view),
                      max_(alignof(repeated_text_view),
                           alignof(reference<rope_tag>))))
            +
            max_(sizeof(text),
                 max_(sizeof(text_view),
                      max_(sizeof(repeated_text_view),
                           sizeof(reference<rope_tag>))))
            ;
    }

    enum class which : char { t, tv, rtv, ref };

    constexpr int text_insert_max = 512;

    static_assert(sizeof(node_ptr<detail::rope_tag>) * 8 <= 64, "");

    template <>
    struct leaf_node_t<rope_tag> : node_t<rope_tag>
    {
        leaf_node_t () noexcept : leaf_node_t (text_view()) {}

        leaf_node_t (text const & t) :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::t)
        {
            auto at = placement_address<text>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text(t);
        }

        leaf_node_t (text && t) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::t)
        {
            auto at = placement_address<text>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text(std::move(t));
        }

        leaf_node_t (text_view tv) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::tv)
        {
            auto at = placement_address<text_view>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text_view(tv);
        }

        leaf_node_t (repeated_text_view rtv) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::rtv)
        {
            auto at = placement_address<repeated_text_view>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) repeated_text_view(rtv);
        }

        leaf_node_t (leaf_node_t const & rhs) :
            node_t (true),
            buf_ptr_ (rhs.buf_ptr_),
            which_ (rhs.which_)
        {
            switch (which_) {
            case which::t: {
                auto at = placement_address<text>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) text(rhs.as_text());
                break;
            }
            case which::tv: {
                auto at = placement_address<text_view>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) text_view(rhs.as_text_view());
                break;
            }
            case which::rtv: {
                auto at = placement_address<repeated_text_view>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) repeated_text_view(rhs.as_repeated_text_view());
                break;
            }
            case which::ref: {
                auto at = placement_address<reference<rope_tag>>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) reference<rope_tag>(rhs.as_reference());
                break;
            }
            default: assert(!"unhandled rope node case"); break;
            }
        }

        leaf_node_t & operator= (leaf_node_t const &) = delete;
        leaf_node_t (leaf_node_t &&) = delete;
        leaf_node_t & operator= (leaf_node_t &&) = delete;

        ~leaf_node_t () noexcept
        {
            if (!buf_ptr_)
                return;

            switch (which_) {
            case which::t: as_text().~text(); break;
            case which::tv: as_text_view().~text_view (); break;
            case which::rtv: as_repeated_text_view().~repeated_text_view(); break;
            case which::ref: as_reference().~reference(); break;
            default: assert(!"unhandled rope node case"); break;
            }
        }

        int size () const noexcept
        {
            switch (which_) {
            case which::t: return as_text().size(); break;
            case which::tv: return as_text_view().size(); break;
            case which::rtv: return as_repeated_text_view().size(); break;
            case which::ref: return as_reference().ref_.size(); break;
            default: assert(!"unhandled rope node case"); break;
            }
            return -(1 << 30); // This should never execute.
        }

        text const & as_text () const noexcept
        {
            assert(which_ == which::t);
            return *static_cast<text *>(buf_ptr_);
        }

        text_view const & as_text_view () const noexcept
        {
            assert(which_ == which::tv);
            return *static_cast<text_view *>(buf_ptr_);
        }

        repeated_text_view const & as_repeated_text_view () const noexcept
        {
            assert(which_ == which::rtv);
            return *static_cast<repeated_text_view *>(buf_ptr_);
        }

        reference<rope_tag> const & as_reference () const noexcept
        {
            assert(which_ == which::ref);
            return *static_cast<reference<rope_tag> *>(buf_ptr_);
        }

        text & as_text () noexcept
        {
            assert(which_ == which::t);
            return *static_cast<text *>(buf_ptr_);
        }

        text_view & as_text_view () noexcept
        {
            assert(which_ == which::tv);
            return *static_cast<text_view *>(buf_ptr_);
        }

        repeated_text_view & as_repeated_text_view () noexcept
        {
            assert(which_ == which::rtv);
            return *static_cast<repeated_text_view *>(buf_ptr_);
        }

        reference<rope_tag> & as_reference () noexcept
        {
            assert(which_ == which::ref);
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

    inline void find_char (node_ptr<rope_tag> const & node, std::ptrdiff_t n, found_char & retval) noexcept
    {
        assert(node);
        find_leaf(node, n, retval.leaf_);

        leaf_node_t<rope_tag> const * leaf = retval.leaf_.leaf_->as_leaf(); // Heh.
        char c = '\0';
        switch (leaf->which_) {
        case which::t:
            c = *(leaf->as_text().cbegin() + retval.leaf_.offset_);
            break;
        case which::tv:
            c = *(leaf->as_text_view().begin() + retval.leaf_.offset_);
            break;
        case which::rtv:
            c = *(leaf->as_repeated_text_view().begin() + retval.leaf_.offset_);
            break;
        case which::ref:
            c = *(leaf->as_reference().ref_.begin() + retval.leaf_.offset_);
            break;
        default: assert(!"unhandled rope node case"); break;
        }
        retval.c_ = c;
    }

    inline reference<rope_tag>::reference (node_ptr<rope_tag> const & text_node, text_view ref) noexcept :
        text_ (text_node),
        ref_ (ref)
    {
        assert(text_node);
        assert(text_node->leaf_);
        assert(text_node.as_leaf()->which_ == which::t);
    }

    enum encoding_note_t { check_encoding_breakage, encoding_breakage_ok };

    inline node_ptr<rope_tag> make_node (text const & t)
    { return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(t)); }

    inline node_ptr<rope_tag> make_node (text && t)
    { return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(std::move(t))); }

    inline node_ptr<rope_tag> make_node (text_view tv)
    { return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(tv)); }

    inline node_ptr<rope_tag> make_node (repeated_text_view rtv)
    { return node_ptr<rope_tag>(new leaf_node_t<rope_tag>(rtv)); }

    inline node_ptr<rope_tag> make_ref (
        leaf_node_t<rope_tag> const * t,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        assert(t->which_ == which::t);
        text_view const tv =
            encoding_note == encoding_breakage_ok ?
            text_view(t->as_text().begin() + lo, hi - lo, utf8::unchecked) :
            t->as_text()(lo, hi);

        leaf_node_t<rope_tag> * leaf = nullptr;
        node_ptr<rope_tag> retval(leaf = new leaf_node_t<rope_tag>);
        leaf->which_ = which::ref;
        auto at = placement_address<reference<rope_tag>>(leaf->buf_, sizeof(leaf->buf_));
        assert(at);
        leaf->buf_ptr_ = new (at) reference<rope_tag>(node_ptr<rope_tag>(t), tv);
        return retval;
    }

    inline node_ptr<rope_tag> make_ref (
        reference<rope_tag> const & t,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        auto const offset = t.ref_.begin() - t.text_.as_leaf()->as_text().begin();
        node_ptr<rope_tag> retval = make_ref(t.text_.as_leaf(), lo + offset, hi + offset, encoding_note);
        return retval;
    }

    inline node_ptr<rope_tag> slice_leaf (
        node_ptr<rope_tag> const & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        bool immutable,
        encoding_note_t encoding_note
    ) {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = !immutable && node->refs_ == 1;

        switch (node.as_leaf()->which_) {
        case which::t:
            if (!leaf_mutable)
                return make_ref(node.as_leaf(), lo, hi, encoding_note);
            {
                auto mut_node = node.write();
                text & t = mut_node.as_leaf()->as_text();
                if (encoding_note == encoding_breakage_ok)
                    t = text_view(t.begin() + lo, hi - lo, utf8::unchecked);
                else
                    t = t(lo, hi);
            }
            return node;
        case which::tv: {
            text_view const old_tv = node.as_leaf()->as_text_view();
            text_view const new_tv =
                encoding_note == encoding_breakage_ok ?
                text_view(old_tv.begin() + lo, hi - lo, utf8::unchecked) :
                old_tv(lo, hi);

            if (!leaf_mutable)
                return make_node(new_tv);

            {
                auto mut_node = node.write();
                text_view & tv = mut_node.as_leaf()->as_text_view();
                tv = new_tv;
            }
            return node;
        }
        case which::rtv: {
            repeated_text_view const & crtv = node.as_leaf()->as_repeated_text_view();
            int const mod_lo = lo % crtv.view().size();
            int const mod_hi = hi % crtv.view().size();
            if (mod_lo != 0 || mod_hi != 0) {
                if (encoding_note == check_encoding_breakage) {
                    text_view const tv = crtv.view()(
                        (std::min)(mod_lo, mod_hi),
                        (std::max)(mod_lo, mod_hi)
                    );
                    (void)tv;
                }
                return make_node(text(crtv.begin() + lo, crtv.begin() + hi));
            } else {
                auto const count = (hi - lo) / crtv.view().size();
                if (!leaf_mutable)
                    return make_node(repeated_text_view(crtv.view(), count));
                auto mut_node = node.write();
                repeated_text_view & rtv = mut_node.as_leaf()->as_repeated_text_view();
                rtv = repeated_text_view(rtv.view(), count);
            }
            return node;
        }
        case which::ref: {
            if (!leaf_mutable)
                return make_ref(node.as_leaf()->as_reference(), lo, hi, encoding_note);
            {
                auto mut_node = node.write();
                reference<rope_tag> & ref = mut_node.as_leaf()->as_reference();
                ref.ref_ =
                    encoding_note == encoding_breakage_ok ?
                    text_view(ref.ref_.begin() + lo, hi - lo, utf8::unchecked) :
                    ref.ref_(lo, hi);
            }
            return node;
        }
        default: assert(!"unhandled rope node case"); break;
        }
        return node_ptr<rope_tag>(); // This should never execute.
    }

    inline leaf_slices<rope_tag> erase_leaf (
        node_ptr<rope_tag> & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note
    ) {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = node.as_leaf()->refs_ == 1;
        auto const leaf_size = size(node.get());

        leaf_slices<rope_tag> retval;

        if (lo == 0 && hi == leaf_size)
            return retval;

        if (leaf_mutable && node.as_leaf()->which_ == which::t) {
            {
                auto mut_node = node.write();
                text & t = mut_node.as_leaf()->as_text();
                if (encoding_note == encoding_breakage_ok)
                    t.erase(text_view(t.begin() + lo, hi - lo, utf8::unchecked));
                else
                    t.erase(t(lo, hi));
            }
            retval.slice = node;
            return retval;
        }

        if (hi != leaf_size)
            retval.other_slice = slice_leaf(node, hi, leaf_size, true, encoding_note);
        if (lo != 0)
            retval.slice = slice_leaf(node, 0, lo, false, encoding_note);

        if (!retval.slice)
            retval.slice.swap(retval.other_slice);

        return retval;
    }

    inline bool child_immutable (node_ptr<rope_tag> const & node)
    { return node.as_leaf()->which_ == which::t; }

    struct segment_inserter
    {
        template <typename Segment>
        void operator() (Segment const & s) const
        {
            if (os_.good())
                os_ << s;
        }

        std::ostream & os_;
    };

    template <typename Segment>
    bool encoded (Segment const & segment)
    { return utf8::encoded(segment.begin(), segment.end()); }

    inline bool encoded (repeated_text_view rtv)
    { return utf8::encoded(rtv.view().begin(), rtv.view().end()); }

    struct segment_encoding_checker
    {
        template <typename Segment>
        void operator() (Segment const & s) const
        {
            if (!encoded(s))
                throw std::invalid_argument("Invalid UTF-8 encoding");
        }
    };

    struct repeated_range
    {
        repeated_text_view::const_iterator first, last;
        repeated_text_view::const_iterator begin () const { return first; }
        repeated_text_view::const_iterator end () const { return last; }
    };

    inline std::ostream & operator<< (std::ostream & os, repeated_range rr)
    {
        for (char c : rr) {
            os << c;
        }
        return os;
    }

} } }

#endif
