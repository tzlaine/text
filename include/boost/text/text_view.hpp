// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/text_fwd.hpp>
#include <boost/text/estimated_width.hpp>
#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/utf.hpp>
#include <boost/text/detail/utility.hpp>

#include <boost/assert.hpp>

#include <iterator>

#include <climits>


namespace boost { namespace text {

    template<nf Normalization, typename Char>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires utf8_code_unit<Char> || utf16_code_unit<Char>
#endif
    struct basic_text_view
    // clang-format on
    {
        /** The normalization form used in this `basic_text_view`. */
        static constexpr nf normalization = Normalization;

        /** The type of code unit used in the underlying storage. */
        using char_type = Char;

        /** The UTF format used in the underlying storage. */
        static constexpr format utf_format = detail::format_of<char_type>();

        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        static_assert(
            utf_format == format::utf8 || utf_format == format::utf16, "");

        using value_type =
            utf32_view<detail::text_transcode_iterator_t<char_type const>>;
        using size_type = std::size_t;
        using iterator = grapheme_iterator<
            detail::text_transcode_iterator_t<char_type const>>;
        using const_iterator = iterator;
        using reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;
        using const_reverse_iterator = reverse_iterator;

        using text_iterator =
            grapheme_iterator<detail::text_transcode_iterator_t<char_type>>;
        using const_text_iterator = const_iterator;

        /** Default ctor. */
        basic_text_view() noexcept : first_(), last_() {}

        /** Constructs a `basic_text_view` from a pair of
            `const_text_iterators`.

            \pre boost::text::normalized<normalization>(first.base(),
            last.base()) */
        basic_text_view(
            const_text_iterator first, const_text_iterator last) noexcept :
            first_(first), last_(last)
        {
            BOOST_ASSERT(boost::text::normalized<normalization>(
                first.base(), last.base()));
        }

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

        /** Returns the number of code units referred to by `*this`, not
            including the null terminator. */
        size_type storage_code_units() const noexcept
        {
            return last_.base().base() - first_.base().base();
        }

        /** Returns the number of graphemes in `*this`.  This operation is
            O(n). */
        size_type distance() const noexcept
        {
            return std::distance(begin(), end());
        }

        /** Swaps `*this` with `rhs`. */
        void swap(basic_text_view & rhs) noexcept
        {
            std::swap(first_, rhs.first_);
            std::swap(last_, rhs.last_);
        }

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, basic_text_view tv)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    tv.begin().base(), tv.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os.write(tv.begin().base().base(), tv.storage_code_units());
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs formatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream &
        operator<<(std::wostream & os, basic_text_view tv)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    tv.begin().base(), tv.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf16(tv.str_);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#endif

        friend bool
        operator==(basic_text_view lhs, basic_text_view rhs) noexcept
        {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        friend bool
        operator!=(basic_text_view lhs, basic_text_view rhs) noexcept
        {
            return !(lhs == rhs);
        }

        friend void swap(basic_text_view & lhs, basic_text_view & rhs)
        {
            lhs.swap(rhs);
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator
        make_iter(char_type * first, char_type * it, char_type * last) noexcept
        {
            return iterator{
                detail::text_transcode_iterator_t<char_type const>{
                    first, first, last},
                detail::text_transcode_iterator_t<char_type const>{
                    first, it, last},
                detail::text_transcode_iterator_t<char_type const>{
                    first, last, last}};
        }

        iterator first_;
        iterator last_;

#endif
    };

    template<nf Normalization, typename Char>
    int operator+(
        basic_text_view<Normalization, Char> const & t,
        Char const * c_str) = delete;

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
