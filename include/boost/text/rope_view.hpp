// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_ROPE_VIEW_HPP
#define BOOST_TEXT_ROPE_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/unencoded_rope_view.hpp>
#include <boost/text/text_fwd.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/detail/rope_iterator.hpp>

#include <iterator>


namespace boost { namespace text {

    struct rope;

    /** A reference to a substring of a rope, text, or text_view. */
    struct rope_view
    {
        using value_type =
            utf32_view<utf_8_to_32_iterator<unencoded_rope_view::const_iterator>>;
        using size_type = std::size_t;
        using iterator = grapheme_iterator<
            utf_8_to_32_iterator<unencoded_rope_view::const_iterator>>;
        using const_iterator = iterator;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

        using rope_iterator = grapheme_iterator<
            utf_8_to_32_iterator<unencoded_rope::const_iterator>>;
        using const_rope_iterator = grapheme_iterator<
            utf_8_to_32_iterator<unencoded_rope::const_iterator>>;

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

        /** Swaps *this with rhs. */
        void swap(rope_view & rhs) noexcept;

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, rope_view rv)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    rv.begin().base(), rv.end().base());
                detail::pad_width_before(os, size);
                if (os.good()) {
                    std::ostream_iterator<char> out(os);
                    std::copy(
                        rv.begin().base().base(), rv.end().base().base(), out);
                }
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs formatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, rope_view rv)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    rv.begin().base(), rv.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf16(rv);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator make_iter(
            unencoded_rope_view::const_iterator first,
            unencoded_rope_view::const_iterator it,
            unencoded_rope_view::const_iterator last) noexcept;

        unencoded_rope_view view_;

#endif
    };

    inline bool operator==(rope_view lhs, rope_view rhs) noexcept
    {
        return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
    }

    inline bool operator!=(rope_view lhs, rope_view rhs) noexcept
    {
        return !(lhs == rhs);
    }

}}

#include <boost/text/text.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>

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
            first.base().base() - first.base().base().vec_->begin();
        auto const hi = last.base().base() - last.base().base().vec_->begin();
        view_ = unencoded_rope_view(first.base().base().vec_, lo, hi);
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

    inline rope_view::iterator rope_view::make_iter(
        unencoded_rope_view::const_iterator first,
        unencoded_rope_view::const_iterator it,
        unencoded_rope_view::const_iterator last) noexcept
    {
        return iterator{utf_8_to_32_iterator<unencoded_rope_view::const_iterator>{
                            first, first, last},
                        utf_8_to_32_iterator<unencoded_rope_view::const_iterator>{
                            first, it, last},
                        utf_8_to_32_iterator<unencoded_rope_view::const_iterator>{
                            first, last, last}};
    }

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::rope_view>
    {
        using argument_type = boost::text::rope_view;
        using result_type = std::size_t;
        result_type operator()(argument_type const & rv) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(rv);
        }
    };
}

#endif

#if defined(__cpp_lib_concepts)

namespace std::ranges {
    template<>
    inline constexpr bool enable_borrowed_range<boost::text::rope_view> = true;
}

#endif

#endif
