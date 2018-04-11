#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <iterator>
#include <cassert>


namespace boost { namespace text {

    struct text;

    /** A reference to a constant sequence of graphemes. The underlying
        storage is a string that is UTF-8-encoded and FCC-normalized. */
    struct text_view
    {
        using value_type =
            cp_range<utf8::to_utf32_iterator<char const *, char const *>>;
        using size_type = int;
        using iterator = grapheme_iterator<
            utf8::to_utf32_iterator<char const *, char const *>>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<const_iterator>;
        using const_reverse_iterator = reverse_iterator;

        using text_iterator =
            grapheme_iterator<utf8::to_utf32_iterator<char *, char *>>;
        using const_text_iterator = const_iterator;

        /** Default ctor. */
        text_view() noexcept : first_(), last_() {}

        /** Constructs a text_view from a text. */
        text_view(text const & t) noexcept;

        /** Disallow construction from a temporary text. */
        text_view(text && t) noexcept = delete;

        /** Constructs a text_view from a pair of text_iterators. */
        text_view(text_iterator first, text_iterator last) noexcept :
            first_(make_iter(
                first.base().base(), first.base().base(), last.base().base())),
            last_(make_iter(
                first.base().base(), last.base().base(), last.base().base()))
        {}

        /** Constructs a text_view from a pair of const_text_iterators. */
        text_view(const_text_iterator first, const_text_iterator last) noexcept :
            first_(first),
            last_(last)
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
        int storage_bytes() const noexcept
        {
            return last_.base().base() - first_.base().base();
        }

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        int distance() const noexcept { return std::distance(begin(), end()); }

        /** Returns the maximum size a text_view can have. */
        int max_size() const noexcept { return INT_MAX; }

        /** Swaps *this with rhs. */
        void swap(text_view & rhs) noexcept
        {
            std::swap(first_, rhs.first_);
            std::swap(last_, rhs.last_);
        }

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, text_view tv)
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

    private:
        static iterator make_iter(char * first, char * it, char * last) noexcept
        {
            return iterator{utf8::to_utf32_iterator<char const *, char const *>{
                                first, first, last},
                            utf8::to_utf32_iterator<char const *, char const *>{
                                first, it, last},
                            utf8::to_utf32_iterator<char const *, char const *>{
                                first, last, last}};
        }

        iterator first_;
        iterator last_;
    };

    inline bool operator==(text_view lhs, text_view rhs) noexcept
    {
        return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
    }

    inline bool operator!=(text_view lhs, text_view rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline text_view::iterator begin(text_view tv) noexcept
    {
        return tv.begin();
    }
    inline text_view::iterator end(text_view tv) noexcept { return tv.end(); }

    inline text_view::iterator cbegin(text_view tv) noexcept
    {
        return tv.begin();
    }
    inline text_view::iterator cend(text_view tv) noexcept { return tv.end(); }

    inline text_view::reverse_iterator rbegin(text_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline text_view::reverse_iterator rend(text_view tv) noexcept
    {
        return tv.rend();
    }

    inline text_view::reverse_iterator crbegin(text_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline text_view::reverse_iterator crend(text_view tv) noexcept
    {
        return tv.rend();
    }

    inline int operator+(text_view const & t, char const * c_str) = delete;

}}

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline text_view::text_view(text const & t) noexcept :
        first_(t.begin()),
        last_(t.end())
    {}

}}

#endif
