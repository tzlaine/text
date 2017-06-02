#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/utf8.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <memory>
#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text_view;
    struct repeated_text_view;

    // TODO: text should use the more efficient versions of the
    // constexpr-friendly-but-slower operations that text_view does.
    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () noexcept : data_ (), size_ (0), cap_ (0) {}

        text (text const & t) : data_ (), size_ (0), cap_ (0)
        { insert(0, t); }

        text (text && rhs) noexcept : data_ (), size_ (0), cap_ (0)
        { swap(rhs); }

        explicit text (char const * c_str);

        template <typename CharRange>
        explicit text (
            CharRange const & r,
            detail::rng_alg_ret_t<int *, CharRange> enable = 0
        ) : data_ (), size_ (0), cap_ (0)
        { insert(0, r); }

        inline explicit text (text_view view);
        inline explicit text (repeated_text_view view);

        template <typename Iter>
        text (Iter first, Iter last) : data_ (), size_ (0), cap_ (0)
        { insert(0, first, last); }

        text & operator= (text const & t)
        {
            if (t.size() <= size()) {
                clear();
                insert(0, t);
            } else {
                text tmp(t);
                swap(tmp);
            }
            return *this;
        }

        text & operator= (text && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        template <typename CharRange>
        auto operator= (CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & operator= (text_view view);
        inline text & operator= (repeated_text_view view);

        iterator begin () noexcept
        {
            if (!data_)
                return reinterpret_cast<char *>(&cap_);
            return data_.get();
        }
        iterator end () noexcept
        {
            if (!data_)
                return reinterpret_cast<char *>(&cap_);
            return data_.get() + size_;
        }

        const_iterator begin () const noexcept
        {
            if (!data_)
                return reinterpret_cast<char const *>(&cap_);
            return data_.get();
        }
        const_iterator end () const noexcept
        {
            if (!data_)
                return reinterpret_cast<char const *>(&cap_);
            return data_.get() + size_;
        }

        const_iterator cbegin () const noexcept { return begin(); }
        const_iterator cend () const noexcept { return end(); }

        reverse_iterator rbegin () noexcept { return reverse_iterator(end() - 1); }
        reverse_iterator rend () noexcept { return reverse_iterator(begin() - 1); }

        const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator rend () const noexcept { return const_reverse_iterator(begin()); }

        const_reverse_iterator crbegin () const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator crend () const noexcept { return const_reverse_iterator(begin()); }

        bool empty () const noexcept
        { return size_ == 0; }

        int size () const noexcept
        { return size_; }

        int capacity () const noexcept
        { return cap_ - 1; }

        char operator[] (int i) const noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= 0 && i < size_);
#endif
            return data_[i];
        }

        text_view operator() (int lo, int hi) const noexcept;
        text_view operator() (int lo) const noexcept;

        int max_size () const noexcept
        { return INT_MAX; }

        // TODO: operator<=> () const
        int compare (text_view rhs) const noexcept;

        bool operator== (text_view rhs) const noexcept;
        bool operator!= (text_view rhs) const noexcept;
        bool operator< (text_view rhs) const noexcept;
        bool operator<= (text_view rhs) const noexcept;
        bool operator> (text_view rhs) const noexcept;
        bool operator>= (text_view rhs) const noexcept;

        void clear () noexcept
        {
            size_ = 0;
            if (data_)
                data_[0] = '\0';
        }

        char & operator[] (int i) noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= 0 && i < size_);
#endif
            return data_[i];
        }

        template <typename CharRange>
        auto insert (int at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & insert (int at, text_view view);
        inline text & insert (int at, repeated_text_view rv);

        template <typename Iter>
        text & insert (int at, Iter first, Iter last)
        {
            assert(0 <= at && at <= size_);

            if (first == last)
                return *this;

            if (!utf8::starts_encoded(cbegin() + at, cend()))
                throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

            std::unique_ptr<char []> initial_data;
            int const initial_size = size_;
            int const initial_cap = cap_;
            try {
                while (first != last) {
                    push_char(*first, initial_data);
                    ++first;
                }
            } catch (std::bad_alloc const &) {
                data_.swap(initial_data);
                size_ = initial_size;
                cap_ = initial_cap;
                throw;
            }

            std::rotate(begin() + at, begin() + initial_size, end());
            data_[size_] = '\0';

            return *this;
        }

        inline text & erase (text_view view) noexcept;

        template <typename CharRange>
        auto replace (text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & replace (text_view old_substr, text_view new_substr);
        inline text & replace (text_view old_substr, repeated_text_view new_substr);

        template <typename Iter>
        text & replace (text_view old_substr, Iter first, Iter last);

        void resize (int new_size, char c)
        {
            assert(0 <= new_size);

            if (c & 0x80)
                throw std::invalid_argument("Given character is not a valid UTF-8 1-character code point");

            int const prev_size = size_;
            int const delta = new_size - prev_size;
            if (!delta)
                return;

            int const available = cap_ - 1 - size_;
            if (available < delta) {
                std::unique_ptr<char []> new_data = get_new_data(delta - available);
                std::copy(begin(), begin() + prev_size, new_data.get());
                new_data.swap(data_);
            } else {
                if (!utf8::ends_encoded(cbegin(), cbegin() + new_size))
                    throw std::invalid_argument("Resizing to the given size breaks UTF-8 encoding.");
            }

            size_ = new_size;

            if (0 < delta)
                std::fill(begin() + prev_size, end(), c);

            data_[size_] = '\0';
        }

        void reserve (int new_size)
        {
            assert(0 <= new_size);
            int const new_cap = new_size + 1;
            if (new_cap <= cap_)
                return;
            std::unique_ptr<char []> new_data(new char[new_cap]);
            *std::copy(cbegin(), cend(), new_data.get()) = '\0';
            data_.swap(new_data);
            cap_ = new_cap;
        }

        void shrink_to_fit ()
        {
            if (cap_ == 0 || cap_ == size_ + 1)
                return;
            std::unique_ptr<char []> new_data(new char[size_ + 1]);
            *std::copy(cbegin(), cend(), new_data.get()) = '\0';
            data_.swap(new_data);
            cap_ = size_ + 1;
        }

        void swap (text & rhs) noexcept
        {
            data_.swap(rhs.data_);
            std::swap(size_, rhs.size_);
            std::swap(cap_, rhs.cap_);
        }

        friend iterator begin (text & t) noexcept
        { return t.begin(); }
        friend iterator end (text & t) noexcept
        { return t.end(); }
        friend const_iterator begin (text const & t) noexcept
        { return t.begin(); }
        friend const_iterator end (text const & t) noexcept
        { return t.end(); }
        friend const_iterator cbegin (text const & t) noexcept
        { return t.cbegin(); }
        friend const_iterator cend (text const & t) noexcept
        { return t.cend(); }

        friend reverse_iterator rbegin (text & t) noexcept
        { return t.rbegin(); }
        friend reverse_iterator rend (text & t) noexcept
        { return t.rend(); }
        friend const_reverse_iterator rbegin (text const & t) noexcept
        { return t.rbegin(); }
        friend const_reverse_iterator rend (text const & t) noexcept
        { return t.rend(); }
        friend const_reverse_iterator crbegin (text const & t) noexcept
        { return t.crbegin(); }
        friend const_reverse_iterator crend (text const & t) noexcept
        { return t.crend(); }

        friend std::ostream & operator<< (std::ostream & os, text const & t)
        { return os.write(t.begin(), t.size()); }

    private:
        int grow_cap (int min_new_cap) const
        {
            assert(0 < min_new_cap);
            int retval = (std::max)(8, cap_);
            while (retval < min_new_cap) {
                retval = retval / 2 * 3;
            }
            // Leave very short strings very short (8 bytes -- even though
            // this is not portably achievable), but have the rest end on a
            // 16-byte bundary.
            if (8 < retval) {
                int const rem = (retval + 16) % 16;
                retval += 16 - rem;
            }
            return retval;
        }

        std::unique_ptr<char []> get_new_data (int resize_amount)
        {
            assert(0 < resize_amount);
            int const new_cap = grow_cap(cap_ + resize_amount);
            std::unique_ptr<char []> retval(new char [new_cap]);
            cap_ = new_cap;
            return retval;
        }

        void push_char (char c, std::unique_ptr<char []> & initial_data)
        {
            int const available = cap_ - 1 - size_;
            if (available < 1) {
                std::unique_ptr<char []> new_data = get_new_data(1 - available);
                std::copy(cbegin(), cend(), new_data.get());
                if (!initial_data) {
                    initial_data = std::move(data_);
                    data_ = std::move(new_data);
                } else {
                    new_data.swap(data_);
                }
            }
            data_[size_] = c;
            ++size_;
        }

        std::unique_ptr<char []> data_;
        int size_;
        int cap_;
    };

    inline text const & checked_encoding (text const & t)
    {
        if (!utf8::encoded(t.begin(), t.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return t;
    }

    inline text && checked_encoding (text && t)
    {
        if (!utf8::encoded(t.begin(), t.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return std::move(t);
    }

} }

#include <boost/text/text_view.hpp>

namespace boost { namespace text {

    namespace literals {

        inline text operator"" _t (char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            return text(text_view(str, len));
        }

        inline text operator"" _t (char16_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 2);
            return text(
                utf8::from_utf16_iterator<char16_t const *>(str),
                utf8::from_utf16_iterator<char16_t const *>(str + len)
            );
        }

        inline text operator"" _t (char32_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 4);
            return text(
                utf8::from_utf32_iterator<char32_t const *>(str),
                utf8::from_utf32_iterator<char32_t const *>(str + len)
            );
        }

    }

    inline text::text (char const * c_str) : data_ (), size_ (0), cap_ (0)
    { insert(0, c_str); }

    inline text::text (text_view view) : data_ (), size_ (0), cap_ (0)
    { insert(0, view); }

    inline text::text (repeated_text_view rv) : data_ (), size_ (0), cap_ (0)
    { insert(0, rv); }

    template <typename CharRange>
    auto text::operator= (CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return *this = text_view(&*r.begin(), r.end() - r.begin()); }

    inline text & text::operator= (text_view view)
    {
        assert(0 <= view.size());
        if (view.size() <= size()) {
            clear();
            insert(0, view);
        } else {
            text tmp(view);
            swap(tmp);
        }
        return *this;
    }

    inline text & text::operator= (repeated_text_view rv)
    {
        assert(0 <= rv.size());
        if (rv.size() <= size()) {
            clear();
            insert(0, rv);
        } else {
            text tmp(rv);
            swap(tmp);
        }
        return *this;
    }

    inline text_view text::operator() (int lo, int hi) const noexcept
    { return text_view(*this)(lo, hi); }

    inline text_view text::operator() (int lo) const noexcept
    { return text_view(*this)(lo); }

    inline int text::compare (text_view rhs) const noexcept
    { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

    inline bool text::operator== (text_view rhs) const noexcept
    { return compare(rhs) == 0; }

    inline bool text::operator!= (text_view rhs) const noexcept
    { return compare(rhs) != 0; }

    inline bool text::operator< (text_view rhs) const noexcept
    { return compare(rhs) < 0; }

    inline bool text::operator<= (text_view rhs) const noexcept
    { return compare(rhs) <= 0; }

    inline bool text::operator> (text_view rhs) const noexcept
    { return compare(rhs) > 0; }

    inline bool text::operator>= (text_view rhs) const noexcept
    { return compare(rhs) >= 0; }

    template <typename CharRange>
    auto text::insert (int at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::insert (int at, text_view view)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= view.size());

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const view_null_terminated = !view.empty() && view.end()[-1] == '\0';
        if (view_null_terminated)
            view = view(0, -1);

        int const delta = view.size();
        if (!delta)
            return *this;

        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            buf = std::copy(view.begin(), view.end(), buf);
            buf = std::copy(cbegin() + at, cend(), buf);
            new_data.swap(data_);
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            std::copy(view.begin(), view.end(), buf);
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::insert (int at, repeated_text_view rv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= rv.size());

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const view_null_terminated = !rv.view().empty() && rv.view().end()[-1] == '\0';
        if (view_null_terminated)
            rv = repeat(rv.view()(0, -1), rv.count());

        int const delta = rv.size();
        if (!delta)
            return *this;

        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            for (int i = 0; i < rv.count(); ++i) {
                buf = std::copy(rv.view().begin(), rv.view().end(), buf);
            }
            std::copy(cbegin() + at, cend(), buf);
            new_data.swap(data_);
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            for (int i = 0; i < rv.count(); ++i) {
                buf = std::copy(rv.view().begin(), rv.view().end(), buf);
            }
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::erase (text_view view) noexcept
    {
        assert(0 <= view.size());

        bool const view_null_terminated = !view.empty() && view.end()[-1] == '\0';
        if (view_null_terminated)
            view = view(0, -1);

        assert(begin() <= view.begin() && view.end() <= end());

        std::copy(
            view.end(), cend(),
            const_cast<char *>(view.begin())
        );
        size_ -= view.size();
        data_[size_] = '\0';

        return *this;
    }

    template <typename CharRange>
    auto text::replace (text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return replace(old_substr, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::replace (text_view old_substr, text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        bool const new_substr_null_terminated =
            !new_substr.empty() && new_substr.end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = new_substr(0, -1);

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        int const delta = new_substr.size() - old_substr.size();
        if (!delta)
            return *this;

        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            buf = std::copy(new_substr.begin(), new_substr.end(), buf);
            std::copy(old_substr.end(), cend(), buf);
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                );
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                );
            }
            char * buf = const_cast<char *>(old_substr.begin());
            std::copy(new_substr.begin(), new_substr.end(), buf);
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::replace (text_view old_substr, repeated_text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        bool const new_substr_null_terminated =
            !new_substr.view().empty() && new_substr.view().end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = repeat(new_substr.view()(0, -1), new_substr.count());

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        int const delta = new_substr.size() - old_substr.size();
        if (!delta)
            return *this;

        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
            std::copy(old_substr.end(), cend(), buf);
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                );
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                );
            }
            char * buf = const_cast<char *>(old_substr.begin());
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    // TODO: Constrain Iter, here and elsewhere.
    template <typename Iter>
    text & text::replace (text_view old_substr, Iter first, Iter last)
    {
        assert(0 <= old_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        std::unique_ptr<char []> initial_data;
        int const initial_size = size_;
        int const initial_cap = cap_;
        int chars_pushed = 0;
        try {
            while (first != last) {
                push_char(*first, initial_data);
                ++first;
                ++chars_pushed;
            }
        } catch (std::bad_alloc const &) {
            data_.swap(initial_data);
            size_ = initial_size;
            cap_ = initial_cap;
            throw;
        }

        int const delta = chars_pushed - old_substr.size();
        if (0 < delta) {
            std::rotate(const_cast<char *>(old_substr.end()), begin() + initial_size, end());
            std::copy(old_substr.end(), cend(), const_cast<char *>(old_substr.begin()));
        } else {
            if (delta != 0) {
                std::copy(
                    old_substr.end(), cbegin() + initial_size,
                    const_cast<char *>(old_substr.end()) + delta
                );
            }
            std::copy(
                cbegin() + initial_size, cend(),
                begin() + initial_size + delta
            );
        }

        data_[size_] = '\0';

        return *this;
    }



    inline bool operator== (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) == 0; }

    inline bool operator!= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) != 0; }

    inline bool operator< (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) < 0; }

    inline bool operator<= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) <= 0; }

    inline bool operator> (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) > 0; }

    inline bool operator>= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) >= 0; }



    inline text & operator+= (text & t, text_view view)
    { return t.insert(t.size(), view); }

    inline text & operator+= (text & t, repeated_text_view rv)
    {
        assert(0 <= rv.size());
        t.reserve(t.size() + rv.size());
        for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
            t.insert(t.size(), rv.view());
        }
        return t;
    }

    template <typename CharRange>
    auto operator+= (text & t, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return t.insert(t.size(), text_view(&*r.begin(), r.end() - r.begin())); }



    inline text & operator+= (text && t, text_view view)
    { return t.insert(t.size(), view); }

    inline text & operator+= (text && t, repeated_text_view rv)
    {
        assert(0 <= rv.size());
        t.reserve(t.size() + rv.size());
        for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
            t.insert(t.size(), rv.view());
        }
        return t;
    }

    template <typename CharRange>
    auto operator+= (text && t, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return t.insert(t.size(), text_view(&*r.begin(), r.end() - r.begin())); }



    inline text operator+ (text t, text_view view)
    { return t += view; }

    inline text operator+ (text t, repeated_text_view rv)
    { return t += rv; }

    template <typename CharRange>
    auto operator+ (text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    { return t += r; }

    inline text operator+ (text_view view, text const & t)
    { return (text() += view) += t; }

    inline text operator+ (repeated_text_view rv, text const & t)
    { return (text() += rv) += t; }

    template <typename CharRange>
    auto operator+ (CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    { return (text() += r) += t; }

} }

#endif
