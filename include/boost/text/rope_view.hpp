#ifndef BOOST_TEXT_ROPE_VIEW_HPP
#define BOOST_TEXT_ROPE_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <iterator>


namespace boost { namespace text {

    namespace detail {
        struct rope_iterator;
        struct const_rope_iterator;
    }

    struct text;
    struct text_view;
    struct rope;

    /** A reference to a substring of a rope, text, or text_view. */
    struct rope_view
    {
        using value_type =
            cp_range<utf8::to_utf32_iterator<detail::const_rope_view_iterator>>;
        using size_type = std::ptrdiff_t;
        using iterator = grapheme_iterator<
            utf8::to_utf32_iterator<detail::const_rope_view_iterator>>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

        using rope_iterator =
            grapheme_iterator<utf8::to_utf32_iterator<detail::rope_iterator>>;
        using const_rope_iterator = grapheme_iterator<
            utf8::to_utf32_iterator<detail::const_rope_iterator>>;

        /** Default ctor. */
        rope_view() noexcept {}

        /** Constructs a rope_view from a text. */
        rope_view(text const & t) noexcept;

        /** Disable construction from a temporary text. */
        rope_view(text && t) noexcept = delete;

        /** Constructs a rope_view from a text_view. */
        rope_view(text_view tv) noexcept;

        /** Constructs a rope_view from a rope. */
        rope_view(rope const & r) noexcept;

        /** Disable construction from a temporary rope. */
        rope_view(rope && r) noexcept = delete;

        /** Constructs a rope_view from a pair of const_rope_iterators. */
        rope_view(const_rope_iterator first, const_rope_iterator last) noexcept;

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        const_reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        bool empty() const noexcept { return begin() == end(); }

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        size_type storage_bytes() const noexcept;

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        size_type distance() const noexcept
        {
            return std::distance(begin(), end());
        }

        /** Returns the maximum size in bytes a rope_view can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Visits each segment s of the underlying unencoded_rope and calls
            f(s).  Each segment is a value whose type models a CharIter
            iterator-range.  Depending of the operation performed on each
            segment, this may be more efficient than iterating over [begin(),
            end()).

            \pre Fn is an Invocable accepting a single argument whose begin
            and end model CharIter. */
        template<typename Fn>
        void foreach_segment(Fn && f) const;

        /** Swaps *this with rhs. */
        void swap(rope_view & rhs) noexcept;

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, rope_view rv)
        {
            if (os.good()) {
                auto const size = rv.distance();
                detail::pad_width_before(os, size);
                if (os.good())
                    rv.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

    private:
        static iterator make_iter(
            detail::const_rope_view_iterator first,
            detail::const_rope_view_iterator it,
            detail::const_rope_view_iterator last) noexcept;

        unencoded_rope_view view_;
    };

    inline bool operator==(rope_view lhs, rope_view rhs) noexcept
    {
        return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
    }

    inline bool operator!=(rope_view lhs, rope_view rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline rope_view::iterator begin(rope_view tv) noexcept
    {
        return tv.begin();
    }
    inline rope_view::iterator end(rope_view tv) noexcept { return tv.end(); }

    inline rope_view::iterator cbegin(rope_view tv) noexcept
    {
        return tv.begin();
    }
    inline rope_view::iterator cend(rope_view tv) noexcept { return tv.end(); }

    inline rope_view::reverse_iterator rbegin(rope_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline rope_view::reverse_iterator rend(rope_view tv) noexcept
    {
        return tv.rend();
    }

    inline rope_view::reverse_iterator crbegin(rope_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline rope_view::reverse_iterator crend(rope_view tv) noexcept
    {
        return tv.rend();
    }

}}

#include <boost/text/text.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>
#include <boost/text/detail/rope_iterator.hpp>

namespace boost { namespace text {

    inline rope_view::rope_view(text const & t) noexcept :
        view_(string_view(t.begin().base().base(), t.storage_bytes()))
    {}

    inline rope_view::rope_view(text_view tv) noexcept :
        view_(string_view(tv.begin().base().base(), tv.storage_bytes()))
    {}

    inline rope_view::rope_view(rope const & r) noexcept : view_(r.rope_) {}

    inline rope_view::rope_view(
        const_rope_iterator first, const_rope_iterator last) noexcept
    {
        auto const lo =
            first.base().base() - first.base().base().rope_->begin();
        auto const hi = last.base().base() - last.base().base().rope_->begin();
        view_ = unencoded_rope_view(*first.base().base().rope_, lo, hi);
    }

    inline rope_view::const_iterator rope_view::begin() const noexcept
    {
        return make_iter(view_.begin(), view_.begin(), view_.end());
    }
    inline rope_view::const_iterator rope_view::end() const noexcept
    {
        return make_iter(view_.begin(), view_.end(), view_.end());
    }

    inline rope_view::size_type rope_view::storage_bytes() const noexcept
    {
        return view_.size();
    }

    template<typename Fn>
    void rope_view::foreach_segment(Fn && f) const
    {
        view_.foreach_segment(static_cast<Fn &&>(f));
    }

    inline void rope_view::swap(rope_view & rhs) noexcept
    {
        view_.swap(rhs.view_);
    }

    inline rope_view::iterator rope_view::make_iter(
        detail::const_rope_view_iterator first,
        detail::const_rope_view_iterator it,
        detail::const_rope_view_iterator last) noexcept
    {
        return iterator{
            utf8::to_utf32_iterator<detail::const_rope_view_iterator>{
                first, first, last},
            utf8::to_utf32_iterator<detail::const_rope_view_iterator>{
                first, it, last},
            utf8::to_utf32_iterator<detail::const_rope_view_iterator>{
                first, last, last}};
    }

}}

#endif
