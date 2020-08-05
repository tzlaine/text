// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/text_fwd.hpp>
#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <boost/assert.hpp>

#include <iterator>

#include <climits>


namespace boost { namespace text {

    /** A reference to a constant sequence of graphemes. The underlying
        storage is a string that is UTF-8-encoded and FCC-normalized. */
    template<nf Normalization, typename Char>
#if defined(__cpp_lib_concepts)
    // clang-format off
        requires u8_code_unit<Char> // TODO: Support for UTF-16
#endif
    struct basic_text_view
    // clang-format on
    {
        /** The normalization form used in this basic_text_view. */
        static constexpr nf normalization = Normalization;

        /** The type of code unit used in the underlying storage. */
        using char_type = Char;

        /** The UTF format used in the underlying storage. */
        static constexpr format utf_format = detail::format_of<char_type>();

        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        // TODO: This should change once testing covers UTF-16.
        static_assert(
            utf_format == format::utf8 /* || utf_format == format::utf16*/, "");

        using value_type = utf32_view<utf_8_to_32_iterator<char_type const *>>;
        using size_type = std::size_t;
        using iterator =
            grapheme_iterator<utf_8_to_32_iterator<char_type const *>>;
        using const_iterator = iterator;
        using reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;
        using const_reverse_iterator = reverse_iterator;

        using text_iterator =
            grapheme_iterator<utf_8_to_32_iterator<char_type *>>;
        using const_text_iterator = const_iterator;

        /** Default ctor. */
        basic_text_view() noexcept : first_(), last_() {}

        /** Constructs a basic_text_view from a text. */
#if defined(__cpp_lib_concepts)
        template<typename String>
        // clang-format off
            requires std::is_same_v<std::ranges::range_value_t<String>, Char>
#else
        template<
            typename String,
            typename Enable = std::enable_if_t<
                std::is_same<detail::range_value_t<String>, Char>::value>>
#endif
        basic_text_view(basic_text<Normalization, String> const & t) noexcept;
        // clang-format on

        /** Disallow construction from a temporary text. */
        template<nf Normalization2, typename String>
        basic_text_view(basic_text<Normalization2, String> && t) noexcept =
            delete;

        /** Constructs a basic_text_view from a grapheme_view. */
        template<typename CPIter>
        basic_text_view(grapheme_view<CPIter> range) noexcept;

        /** Constructs a basic_text_view from a pair of const_text_iterators. */
        basic_text_view(
            const_text_iterator first, const_text_iterator last) noexcept :
            first_(first), last_(last)
        {}

        const_iterator begin() const noexcept { return first_; }
        const_iterator end() const noexcept { return last_; }

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

        bool empty() const noexcept { return first_ == last_; }

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        size_type storage_bytes() const noexcept
        {
            return last_.base().base() - first_.base().base();
        }

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        size_type distance() const noexcept
        {
            return std::distance(begin(), end());
        }

        /** Returns the maximum size in bytes a basic_text_view can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Swaps *this with rhs. */
        void swap(basic_text_view & rhs) noexcept
        {
            std::swap(first_, rhs.first_);
            std::swap(last_, rhs.last_);
        }

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, basic_text_view tv)
        {
            if (os.good()) {
                auto const size = tv.distance();
                detail::pad_width_before(os, size);
                if (os.good())
                    os.write(tv.begin().base().base(), tv.storage_bytes());
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator
        make_iter(char_type * first, char_type * it, char_type * last) noexcept
        {
            return iterator{
                utf_8_to_32_iterator<char_type const *>{first, first, last},
                utf_8_to_32_iterator<char_type const *>{first, it, last},
                utf_8_to_32_iterator<char_type const *>{first, last, last}};
        }

        iterator first_;
        iterator last_;

#endif
    };

    template<nf Normalization, typename Char>
    bool operator==(
        basic_text_view<Normalization, Char> lhs,
        basic_text_view<Normalization, Char> rhs) noexcept
    {
        return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
    }

    template<nf Normalization, typename Char>
    bool operator!=(
        basic_text_view<Normalization, Char> lhs,
        basic_text_view<Normalization, Char> rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<nf Normalization, typename Char>
    int operator+(
        basic_text_view<Normalization, Char> const & t,
        Char const * c_str) = delete;

}}

#include <boost/text/text.hpp>

namespace boost { namespace text {

    template<nf Normalization, typename Char>
#if defined(__cpp_lib_concepts)
    template<typename String>
    // clang-format off
        requires std::is_same_v<std::ranges::range_value_t<String>, Char>
#else
    template<typename String, typename Enable>
#endif
    basic_text_view<Normalization, Char>::basic_text_view(
        // clang-format on
        basic_text<Normalization, String> const & t) noexcept :
        first_(t.begin()), last_(t.end())
    {}

    template<nf Normalization, typename Char>
    template<typename CPIter>
    basic_text_view<Normalization, Char>::basic_text_view(
        grapheme_view<CPIter> range) noexcept :
        first_(range.begin()), last_(range.end())
    {}

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<boost::text::nf Normalization, typename Char>
    struct hash<boost::text::basic_text_view<Normalization, Char>>
    {
        using argument_type = boost::text::basic_text_view<Normalization, Char>;
        using result_type = std::size_t;
        result_type operator()(argument_type const & tv) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(tv);
        }
    };
}

#endif

#if defined(__cpp_lib_concepts)

namespace std::ranges {
    template<boost::text::nf Normalization, typename Char>
    inline constexpr bool enable_borrowed_range<
        boost::text::basic_text_view<Normalization, Char>> = true;
}

#endif

#endif
