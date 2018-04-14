#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/container/small_vector.hpp>

#include <iterator>


#ifndef BOOST_TEXT_DOXYGEN

#ifdef BOOST_TEXT_TESTING
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()                                  \
    do {                                                                       \
        string str2(str_);                                                     \
        normalize_to_fcc(str2);                                                \
        assert(str_ == str2);                                                  \
    } while (false)
#else
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()
#endif

#endif

namespace boost { namespace text {

    struct string_view;
    struct string;
    struct repeated_string_view;
    struct text_view;
    struct rope_view;

    /** A mutable contiguous null-terminated sequence of graphemes.  The
        underlying storage is a string that is UTF-8-encoded and
        FCC-normalized. */
    struct text
    {
        using value_type = cp_range<utf8::to_utf32_iterator<char *>>;
        using size_type = int;
        using iterator = grapheme_iterator<utf8::to_utf32_iterator<char *>>;
        using const_iterator =
            grapheme_iterator<utf8::to_utf32_iterator<char const *>>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /** Default ctor. */
        text() {}

        /** Constructs a text from a null-terminated string. */
        text(char const * c_str);

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

        /** Constructs a text from a range of graphemes over an underlying
            range of char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit text(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0);

        template<typename CharIter>
        text(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0);

        template<typename GraphemeRange>
        explicit text(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0);

#endif
        /** Assignment from a null-terminated string. */
        text & operator=(char const * c_str);

        /** Assignment from a string. */
        text & operator=(string s);

        /** Assignment from a text_view. */
        text & operator=(text_view tv);

        /** Assignment from a string_view. */
        text & operator=(string_view sv);

        /** Assignment from a repeated_string_view. */
        text & operator=(repeated_string_view rsv);


#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        text & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        text & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<text &, GraphemeRange>;

#endif

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

        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        text & insert(iterator at, char const * c_str);

        /** Inserts the sequence of char from t into *this starting at
            position at. */
        text & insert(iterator at, text const & t);

        /** Inserts the sequence of char from tv into *this starting at
            position at. */
        text & insert(iterator at, text_view tv);

        /** Inserts the sequence of char from sv into *this starting at
            position at. */
        text & insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rsv into *this starting at
            position at. */
        text & insert(iterator at, repeated_string_view rsv);

        /** Inserts the sequence of char from rv into *this starting at
            position at. */
        text & insert(iterator at, rope_view rv);

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
            -> detail::char_iter_ret_t<text &, CharIter>;

#endif

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin().base().base(), begin().base().base()) &&
            !std::less(end().base().base(), tv.end().base().base()) */
        text & erase(text_view tv) noexcept;

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        text & erase(iterator first, iterator last) noexcept;

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, char const * new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, text const & new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, text_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, repeated_string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, rope_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharRange>
        text & replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
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

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, text const & t)
        {
            if (os.good()) {
                auto const size = t.distance();
                detail::pad_width_before(os, size);
                os << t.str_;
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator make_iter(char * first, char * it, char * last) noexcept
        {
            return iterator{utf8::to_utf32_iterator<char *>{first, first, last},
                            utf8::to_utf32_iterator<char *>{first, it, last},
                            utf8::to_utf32_iterator<char *>{first, last, last}};
        }

        static const_iterator make_iter(
            char const * first, char const * it, char const * last) noexcept
        {
            return const_iterator{
                utf8::to_utf32_iterator<char const *>{first, first, last},
                utf8::to_utf32_iterator<char const *>{first, it, last},
                utf8::to_utf32_iterator<char const *>{first, last, last}};
        }

        template<typename Iter>
        static std::reverse_iterator<Iter> make_reverse_iter(Iter it) noexcept
        {
            return std::reverse_iterator<Iter>{it};
        }

        using mutable_utf32_iter = utf8::to_utf32_iterator<char *>;

        mutable_utf32_iter prev_stable_cp(mutable_utf32_iter last) noexcept;
        mutable_utf32_iter next_stable_cp(mutable_utf32_iter first) noexcept;

        // https://www.unicode.org/reports/tr15/#Concatenation
        void normalize_subrange(int from_near_offset, int to_near_offset);

        template<typename CharIter>
        text & insert_impl(
            iterator at,
            CharIter first,
            CharIter last,
            bool first_last_normalized);
        text & insert_impl(iterator at, string_view sv, bool sv_normalized);

        template<typename CharIter>
        text & replace_impl(
            text_view old_substr,
            CharIter first,
            CharIter last,
            bool first_last_normalized);
        text & replace_impl(
            text_view old_substr,
            string_view new_substr,
            bool new_substr_normalized);

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
#include <boost/text/rope.hpp>
#include <boost/text/normalize.hpp>

namespace boost { namespace text {

    namespace literals {

        /** Creates a text from a char string literal. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            assert(len < INT_MAX / 2);
            return text(str, str + len);
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline text::text(char const * c_str) : str_(c_str)
    {
        normalize_to_fcc(str_);
    }

    inline text::text(string s) : str_(std::move(s))
    {
        normalize_to_fcc(str_);
    }

    inline text::text(text_view tv) : str_()
    {
        str_.insert(str_.begin(), string_view(tv));
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
    }

    inline text::text(string_view sv) : str_(sv)
    {
        normalize_to_fcc(str_);
    }

    inline text::text(repeated_string_view rsv) : str_(rsv)
    {
        normalize_to_fcc(str_);
    }

    template<typename CharRange>
    text::text(CharRange const & r, detail::rng_alg_ret_t<int *, CharRange>) :
        str_(r)
    {
        normalize_to_fcc(str_);
    }

    template<typename CharIter>
    text::text(
        CharIter first,
        CharIter last,
        detail::char_iter_ret_t<void *, CharIter>) :
        str_(first, last)
    {
        normalize_to_fcc(str_);
    }

    template<typename GraphemeRange>
    text::text(
        GraphemeRange const & r,
        detail::graph_rng_alg_ret_t<int *, GraphemeRange>) :
        str_(r)
    {
        normalize_to_fcc(str_);
    }

    inline text & text::operator=(char const * c_str)
    {
        str_ = string_view(c_str);
        normalize_to_fcc(str_);
        return *this;
    }

    inline text & text::operator=(string s)
    {
        str_ = std::move(s);
        normalize_to_fcc(str_);
        return *this;
    }

    inline text & text::operator=(text_view tv)
    {
        str_ = string_view(tv);
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
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
    auto text::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        str_ = r;
        normalize_to_fcc(str_);
        return *this;
    }

    template<typename GraphemeRange>
    auto text::operator=(GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<text &, GraphemeRange>
    {
        str_ = r;
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    template<typename CharRange>
    auto text::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        using std::begin;
        using std::end;
        return insert(at, begin(r), end(r));
    }

    template<typename CharIter>
    auto text::insert(iterator at, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<text &, CharIter>
    {
        return insert_impl(at, first, last, false);
    }

    inline text & text::insert(iterator at, char const * c_str)
    {
        return insert(at, string_view(c_str));
    }

    inline text & text::insert(iterator at, text const & t)
    {
        return insert_impl(at, string_view(t), true);
    }

    inline text & text::insert(iterator at, text_view tv)
    {
        return insert_impl(at, string_view(tv), true);
    }

    inline text & text::insert(iterator at, string_view sv)
    {
        return insert_impl(at, sv, false);
    }

    inline text & text::insert(iterator at, repeated_string_view rsv)
    {
        bool const rsv_null_terminated =
            !rsv.view().empty() && rsv.view().end()[-1] == '\0';
        if (rsv_null_terminated)
            rsv = repeat(rsv.view()(0, -1), rsv.count());
        return insert_impl(at, rsv.begin(), rsv.end(), false);
    }

    inline text & text::erase(text_view tv) noexcept
    {
        int const at = tv.begin().base().base() - str_.begin();
        str_.erase(string_view(tv));
        normalize_subrange(at, at);
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text & text::erase(iterator first, iterator last) noexcept
    {
        return erase(text_view(first, last));
    }

    inline text & text::replace(text_view old_substr, char const * new_substr)
    {
        return replace_impl(old_substr, string_view(new_substr), false);
    }

    inline text & text::replace(text_view old_substr, text const & new_substr)
    {
        return replace_impl(old_substr, string_view(new_substr), true);
    }

    inline text & text::replace(text_view old_substr, text_view new_substr)
    {
        return replace_impl(old_substr, string_view(new_substr), true);
    }

    inline text & text::replace(text_view old_substr, string_view new_substr)
    {
        return replace_impl(old_substr, new_substr, false);
    }

    inline text &
    text::replace(text_view old_substr, repeated_string_view new_substr)
    {
        bool const new_substr_null_terminated =
            !new_substr.view().empty() && new_substr.view().end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = repeat(new_substr.view()(0, -1), new_substr.count());
        return replace_impl(
            old_substr, new_substr.begin(), new_substr.end(), false);
    }

    template<typename CharRange>
    auto text::replace(text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        using std::begin;
        using std::end;
        return replace(old_substr, begin(r), end(r));
    }

    template<typename CharIter>
    auto text::replace(text_view old_substr, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<text &, CharIter>
    {
        return replace_impl(old_substr, first, last, false);
    }

    inline text & text::operator+=(char const * c_str)
    {
        return operator+=(string_view(c_str));
    }

    inline text & text::operator+=(string_view sv) { return insert(end(), sv); }

    inline text & text::operator+=(repeated_string_view rsv)
    {
        return insert(end(), rsv);
    }

    template<typename CharRange>
    auto text::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        using std::begin;
        using std::end;
        return insert(this->end(), begin(r), end(r));
    }

    inline text::mutable_utf32_iter
    text::prev_stable_cp(mutable_utf32_iter last) noexcept
    {
        auto const first =
            mutable_utf32_iter(str_.begin(), str_.begin(), str_.end());
        auto const it =
            find_if_backward(first, last, detail::stable_fcc_code_point);
        if (it == last)
            return first;
        return it;
    }

    inline text::mutable_utf32_iter
    text::next_stable_cp(mutable_utf32_iter first) noexcept
    {
        auto const last =
            mutable_utf32_iter(str_.begin(), str_.end(), str_.end());
        auto const it = find_if(first, last, detail::stable_fcc_code_point);
        return it;
    }

    inline void
    text::normalize_subrange(int from_near_offset, int to_near_offset)
    {
        mutable_utf32_iter first(
            str_.begin(), str_.begin() + from_near_offset, str_.end());
        mutable_utf32_iter last(
            str_.begin(), str_.begin() + to_near_offset, str_.end());
        first = prev_stable_cp(first);
        last = next_stable_cp(last);

        if (first == last)
            return;

        container::small_vector<char, 1024> buf;
        normalize_to_fcc(first, last, utf8::from_utf32_back_inserter(buf));

        str_.replace(
            string_view(first.base(), last.base() - first.base()),
            string_view(&buf[0], buf.size()));
    }

    template<typename CharIter>
    text & text::insert_impl(
        iterator at, CharIter first, CharIter last, bool first_last_normalized)
    {
        int const lo = at.base().base() - str_.begin();
        auto const insertion_it = str_.insert(at.base().base(), first, last);
        int const hi = insertion_it - str_.begin();
        if (first_last_normalized) {
            if (lo)
                normalize_subrange(lo, lo);
            if (hi < str_.size())
                normalize_subrange(hi, hi);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text &
    text::insert_impl(iterator at, string_view sv, bool sv_normalized)
    {
        int const lo = at.base().base() - str_.begin();
        auto const insertion_it = str_.insert(at.base().base(), sv);
        int const hi = insertion_it - str_.begin();
        if (sv_normalized) {
            if (lo)
                normalize_subrange(lo, lo);
            if (hi < str_.size())
                normalize_subrange(hi, hi);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    template<typename CharIter>
    text & text::replace_impl(
        text_view old_substr, CharIter first, CharIter last, bool first_last_normalized)
    {
        int const lo = old_substr.begin().base().base() - str_.begin();
        int const old_size = storage_bytes();
        int const old_substr_size = old_substr.storage_bytes();
        str_.replace(string_view(old_substr), first, last);
        int const new_size = storage_bytes();
        int const hi = lo + old_substr_size + (new_size - old_size);
        if (first_last_normalized) {
            if (lo)
                normalize_subrange(lo, lo);
            if (hi < str_.size())
                normalize_subrange(hi, hi);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text & text::replace_impl(
        text_view old_substr,
        string_view new_substr,
        bool new_substr_normalized)
    {
        bool const new_substr_null_terminated =
            !new_substr.empty() && new_substr.end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = new_substr(0, -1);

        int const lo = old_substr.begin().base().base() - str_.begin();
        int const hi = lo + old_substr.storage_bytes() +
                       (new_substr.size() - old_substr.storage_bytes());
        str_.replace(string_view(old_substr), new_substr);
        if (new_substr_normalized) {
            if (lo)
                normalize_subrange(lo, lo);
            if (hi < str_.size())
                normalize_subrange(hi, hi);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

#endif // Doxygen

    inline bool
    operator==(text const & lhs, char const * rhs) noexcept = delete;
    inline bool
    operator==(char const * lhs, text const & rhs) noexcept = delete;

    inline bool
    operator!=(text const & lhs, char const * rhs) noexcept = delete;
    inline bool
    operator!=(char const * lhs, text const & rhs) noexcept = delete;

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

    inline bool operator==(text const & lhs, text const & rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    inline bool operator!=(text const & lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Creates a new text object that is the concatenation of t and t2. */
    inline text operator+(text t, text const & t2) { return t += t2; }

    /** Creates a new text object that is the concatenation of t and c_str. */
    inline text operator+(text t, char const * c_str) { return t += c_str; }

    /** Creates a new text object that is the concatenation of c_str and t. */
    inline text operator+(char const * c_str, text const & t)
    {
        return text(c_str) + t;
    }

    /** Creates a new text object that is the concatenation of t and tv. */
    inline text operator+(text t, text_view tv) { return t += tv; }

    /** Creates a new text object that is the concatenation of tv and t. */
    inline text operator+(text_view tv, text const & t) { return text(tv) + t; }

    /** Creates a new text object that is the concatenation of t and rtv. */
    inline text operator+(text t, repeated_string_view rtv) { return t += rtv; }

    /** Creates a new text object that is the concatenation of rtv and t. */
    inline text operator+(repeated_string_view rtv, text const & t)
    {
        return text(rtv) + t;
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
        return text(r) + t;
    }

#endif

}}

#endif
