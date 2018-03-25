#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <iterator>


namespace boost { namespace text {

    struct string_view;
    struct string;
    struct repeated_string_view;
    struct text_view;

    /* TODO: Make sure we support operations on:
        char const *          checked     (=)string_view
        string_view           checked     (=)string_view
        repeated_string_view  checked
        string                checked     (=)string_view, except sink params
        unencoded_rope        checked     (=)unencoded_rope_view
        unencoded_rope_view   checked     (=)unencoded_rope_view
        text_view             unchecked   (=)text_view
        text                  unchecked   (=)text_view
        rope                  unchecked
    */

    /** A mutable contiguous null-terminated sequence of graphemes.  The
        underlying storage is a string that is UTF-8-encoded and
        FCC-normalized. */
    struct text
    {
        using iterator =
            grapheme_iterator<utf8::to_utf32_iterator<char *, char *>>;
        using const_iterator = grapheme_iterator<
            utf8::to_utf32_iterator<char const *, char const *>>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /** Default ctor. */
        text() {}

        /** Constructs a text from a string. */
        explicit text(string s);

        /** Constructs a text from a text_view. */
        explicit text(text_view tv);

        /** Constructs a text from a string_view. */
        explicit text(string_view sv);

        /** Constructs a text from a repeated_string_view. */
        explicit text(repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a text from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        explicit text(CharRange const & r);

        /** Constructs a text from a sequence of char.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        text(CharIter first, Iter Charlast);

#else

        template<typename CharRange>
        explicit text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            str_()
        {
            str_.insert(0, r);
        }

        template<typename CharIter>
        text(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0) :
            str_()
        {
            str_.insert(0, first, last);
        }

#endif

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        text & operator=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        /** Assignment from a string. */
        text & operator=(string const & s);

        /** Assignment from a string. */
        text & operator=(string && s);

        /** Assignment from a text_view. */
        text & operator=(text_view tv);

        /** Assignment from a string_view. */
        text & operator=(string_view sv);

        /** Assignment from a repeated_string_view. */
        text & operator=(repeated_string_view rsv);

        iterator begin() noexcept
        {
            return make_iter(str_.begin(), str_.begin(), str_.end());
        }
        iterator end() noexcept
        {
            return make_iter(str_.begin(), str_.end(), str_.end());
        }

        const_iterator begin() const noexcept
        {
            return make_iter(str_.begin(), str_.begin(), str_.end());
        }
        const_iterator end() const noexcept
        {
            return make_iter(str_.begin(), str_.end(), str_.end());
        }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept { return make_reverse_iter(end()); }
        reverse_iterator rend() noexcept { return make_reverse_iter(begin()); }

        const_reverse_iterator rbegin() const noexcept
        {
            return make_reverse_iter(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return make_reverse_iter(begin());
        }

        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        /** Returns true if size() == 0, false otherwise. */
        bool empty() const noexcept { return str_.empty(); }

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        int storage_bytes() const noexcept { return str_.size(); }

        /** Returns the number of bytes of storage currently in use by
         *this. */
        int capacity() const noexcept { return str_.capacity(); }

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        int distance() const noexcept { return std::distance(begin(), end()); }

        /** Returns the maximum size a text can have. */
        int max_size() const noexcept { return INT_MAX / 2; }

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit an
            empty string */
        void clear() noexcept { str_.clear(); }

        /** Inserts the sequence of char from tv into *this starting at position
            at. */
        text & insert(iterator at, text_view tv);

        /** Inserts the sequence of char from sv into *this starting at position
            at. */
        text & insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rsv into *this starting at
            position at. */
        text & insert(iterator at, repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        text & insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        text & insert(iterator at, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename CharIter>
        auto insert(iterator at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<text &, CharIter>
        {
            return str_.insert(at.base().base(), first, last);
        }

#endif

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin(), begin()) && !std::less(end(),
            tv.end()) */
        text & erase(text_view tv) noexcept;

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text & replace(text_view old_substr, text_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text & replace(text_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text & replace(text_view old_substr, repeated_string_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharRange>
        text & replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharIter>
        text & replace(text_view old_substr, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto replace(text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename CharIter>
        auto replace(text_view old_substr, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<text &, CharIter>;

#endif

        /** Reserves storage enough for a string of at least new_size
            bytes.

            \post capacity() >= new_size + 1 */
        void reserve(int new_size) { str_.reserve(new_size); }

        /** Reduces storage used by *this to just the amount necessary to
            contain size() chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit() { str_.shrink_to_fit(); }

        /** Swaps *this with rhs. */
        void swap(text & rhs) noexcept { str_.swap(rhs.str_); }

        /** Removes and returns the underlying string from *this. */
        string extract() && noexcept { return std::move(str_); }

        /** Replaces the underlying string in *this. */
        void replace(string && s) noexcept { str_ = std::move(s); }

        /** Appends c_str to *this. */
        text & operator+=(char const * c_str);

        /** Appends tv to *this. */
        text & operator+=(text_view tv);

        /** Appends tv to *this. */
        text & operator+=(string_view sv);

        /** Appends rtv to *this. */
        text & operator+=(repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        text & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        // TODO: Do formatted output.
        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<<(std::ostream & os, text const & t)
        {
            return os.write(t.str_.begin(), t.str_.size());
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator make_iter(char * first, char * it, char * last)
        {
            return iterator{
                utf8::to_utf32_iterator<char *, char *>{first, first, last},
                utf8::to_utf32_iterator<char *, char *>{first, it, last},
                utf8::to_utf32_iterator<char *, char *>{first, last, last}};
        }

        static const_iterator
        make_iter(char const * first, char const * it, char const * last)
        {
            return const_iterator{
                utf8::to_utf32_iterator<char const *, char const *>{
                    first, first, last},
                utf8::to_utf32_iterator<char const *, char const *>{
                    first, it, last},
                utf8::to_utf32_iterator<char const *, char const *>{
                    first, last, last}};
        }

        template<typename Iter>
        static std::reverse_iterator<Iter> make_reverse_iter(Iter it)
        {
            return std::reverse_iterator<Iter>{it};
        }

        string str_;

#endif // Doxygen
    };

    inline text::iterator begin(text & t) noexcept { return t.begin(); }
    inline text::iterator end(text & t) noexcept { return t.end(); }
    inline text::const_iterator begin(text const & t) noexcept
    {
        return t.begin();
    }
    inline text::const_iterator end(text const & t) noexcept { return t.end(); }
    inline text::const_iterator cbegin(text const & t) noexcept
    {
        return t.cbegin();
    }
    inline text::const_iterator cend(text const & t) noexcept
    {
        return t.cend();
    }

    inline text::reverse_iterator rbegin(text & t) noexcept
    {
        return t.rbegin();
    }
    inline text::reverse_iterator rend(text & t) noexcept { return t.rend(); }
    inline text::const_reverse_iterator rbegin(text const & t) noexcept
    {
        return t.rbegin();
    }
    inline text::const_reverse_iterator rend(text const & t) noexcept
    {
        return t.rend();
    }
    inline text::const_reverse_iterator crbegin(text const & t) noexcept
    {
        return t.crbegin();
    }
    inline text::const_reverse_iterator crend(text const & t) noexcept
    {
        return t.crend();
    }

}}

#include <boost/text/string.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/normalize.hpp>

namespace boost { namespace text {

    namespace literals {

        /** Creates a text from a char string literal. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            assert(utf8::encoded(str, str + len));
            return text(str, str + len);
        }

        /** Creates a text from a char16_t string literal. */
        inline text operator"" _t(char16_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 2);
            return text(
                utf8::from_utf16_iterator<char16_t const *>(str),
                utf8::from_utf16_iterator<char16_t const *>(str + len));
        }

        /** Creates a text from a char32_t string literal. */
        inline text operator"" _t(char32_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 4);
            return text(
                utf8::from_utf32_iterator<char32_t const *>(str),
                utf8::from_utf32_iterator<char32_t const *>(str + len));
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline text::text(text_view tv) : str_()
    {
        str_.insert(
            str_.begin(), tv.begin().base().base(), tv.end().base().base());
    }

    inline text::text(string_view sv) : str_()
    {
        str_.insert(str_.begin(), sv.begin(), sv.end());
        normalize_to_fcc(str_);
    }

    inline text::text(repeated_string_view rsv) : str_()
    {
        str_.insert(str_.begin(), rsv.begin(), rsv.end());
        normalize_to_fcc(str_);
    }

    template<typename CharRange>
    auto text::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        str_ = r;
        normalize_to_fcc(str_);
        return *this;
    }

    inline text & text::operator=(text_view tv)
    {
        clear();
        str_.insert(
            str_.begin(), tv.begin().base().base(), tv.end().base().base());
        return *this;
    }

    inline text & text::operator=(string_view sv)
    {
        str_ = sv;
        normalize_to_fcc(str_);
        return *this;
    }

    inline text & text::operator=(repeated_string_view rsv)
    {
        str_ = rsv;
        normalize_to_fcc(str_);
        return *this;
    }

    template<typename CharRange>
    auto text::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return insert(at, string_view(r));
    }

    inline text & text::insert(iterator at, text_view tv)
    {
        str_.insert(
            at.base().base(), tv.begin().base().base(), tv.end().base().base());
        // TODO: Partial-normalize at either end of inserted tv.
        return *this;
    }

    inline text & text::insert(iterator at, string_view sv)
    {
        str_.insert(at.base().base() - str_.begin(), sv);
        // TODO: Partial-normalize entirety of inserted sv.
        return *this;
    }

    inline text & text::insert(iterator at, repeated_string_view rsv)
    {
        str_.insert(at.base().base() - str_.begin(), rsv);
        // TODO: Partial-normalize entirety of inserted sv.
        return *this;
    }

    inline text & text::erase(text_view tv) noexcept
    {
        auto const first = tv.begin().base().base();
        auto const last = tv.end().base().base();
        str_.erase(string_view(first, last - first));
        // TODO: Partial-normalize around first.
        return *this;
    }

    template<typename CharRange>
    auto text::replace(text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        // TODO
        return *this;
    }

    inline text & text::replace(text_view old_substr, text_view new_substr)
    {
        // TODO
        return *this;
    }

    inline text & text::replace(text_view old_substr, string_view new_substr)
    {
        // TODO
        return *this;
    }

    inline text &
    text::replace(text_view old_substr, repeated_string_view new_substr)
    {
        // TODO
        return *this;
    }

    template<typename CharIter>
    auto text::replace(text_view old_substr, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<text &, CharIter>
    {
        // TODO
        return *this;
    }

    inline text & text::operator+=(char const * c_str)
    {
        str_ += c_str;
        // TODO: Partial-normalize entirety of inserted c_str.
        return *this;
    }

    inline text & text::operator+=(text_view tv)
    {
        auto const first = tv.begin().base().base();
        auto const last = tv.end().base().base();
        str_ += string_view(first, last - first);
        // TODO: Partial-normalize around old end().
        return *this;
    }

    inline text & text::operator+=(string_view sv)
    {
        str_ += sv;
        // TODO: Partial-normalize around old end() to new end().
        return *this;
    }

    inline text & text::operator+=(repeated_string_view rsv)
    {
        str_ += rsv;
        // TODO: Partial-normalize around old end() to new end().
        return *this;
    }

    template<typename CharRange>
    auto text::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        str_.insert(str_.end(), r);
        // TODO: Partial-normalize around old end() to new end().
        return *this;
    }

#endif // Doxygen


    inline bool operator==(text const & lhs, text_view rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    inline bool operator==(text_view lhs, text const & rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(text const & lhs, text_view rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator!=(text_view lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs,
                   lhs + strlen(lhs),
                   rhs.begin().base().base(),
                   rhs.end().base().base()) == 0;
    }

    inline bool operator!=(char const * lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(text const & lhs, char const * rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(text const & lhs, char const * rhs) noexcept
    {
        return !(lhs == rhs);
    }


    /** Creates a new text object that is the concatenation of t and t2. */
    inline text operator+(text t, text const & t2) { return t += t2; }

    /** Creates a new text object that is the concatenation of t and tv. */
    inline text operator+(text t, text_view tv) { return t += tv; }

    /** Creates a new text object that is the concatenation of tv and t. */
    inline text operator+(text_view tv, text const & t)
    {
        return (text() += tv) += t;
    }

    /** Creates a new text object that is the concatenation of t and rtv. */
    inline text operator+(text t, repeated_string_view rtv) { return t += rtv; }

    /** Creates a new text object that is the concatenation of rtv and t. */
    inline text operator+(repeated_string_view rtv, text const & t)
    {
        return (text() += rtv) += t;
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new text object that is the concatenation of t and r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(text t, CharRange const & r);

    /** Creates a new text object that is the concatenation of r and t.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(CharRange const & r, text const & t);

#else

    template<typename CharRange>
    auto operator+(text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return t += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return (text() += r) += t;
    }

#endif

}}

#endif
