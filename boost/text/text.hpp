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

    namespace detail {

        template <typename T>
        struct static_const
        { static constexpr T value {}; };

        template <typename T>
        constexpr T static_const<T>::value;

        struct get_empty_str_fn
        {
            constexpr get_empty_str_fn () : c_ ('\0') {}

            constexpr char const * operator() () const noexcept
            { return &c_; }

            char c_;
        };

    }

    // TODO: inline const c = '\0';
    inline namespace {
        constexpr auto & get_empty_str = detail::static_const<detail::get_empty_str_fn>::value;
    }

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () noexcept : data_ (nullptr), size_ (0), cap_ (0) {}

        text (text const & t) : data_ (), size_ (0), cap_ (0)
        { insert(0, t); }

        text (text && rhs) noexcept : data_ (), size_ (0), cap_ (0)
        { swap(rhs); }

        text (char const * c_str);

        template <typename CharRange>
        explicit text (
            CharRange const & r,
            detail::rng_alg_ret_t<int *, CharRange> enable = 0
        )
        { insert(0, r); }

        inline explicit text (text_view view);
        inline explicit text (repeated_text_view view);

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
                return const_cast<char *>(get_empty_str());
            return data_.get();
        }
        iterator end () noexcept
        {
            if (!data_)
                return const_cast<char *>(get_empty_str());
            return data_.get() + size_;
        }

        const_iterator begin () const noexcept
        {
            if (!data_)
                return get_empty_str();
            return data_.get();
        }
        const_iterator end () const noexcept
        {
            if (!data_)
                return get_empty_str();
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
        { return cap_; }

        char operator[] (int i) const noexcept
        {
            assert(0 <= 0 && i < size_);
            return data_[i];
        }

        text_view operator() (int lo, int hi) const noexcept;
        text_view operator() (int lo) const noexcept;

        int max_size () const noexcept
        { return INT_MAX; }

        operator text_view () const noexcept;

        // TODO: operator<=> () const
        int compare (text const & rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        bool operator== (text const & rhs) const noexcept
        { return compare(rhs) == 0; }

        bool operator!= (text const & rhs) const noexcept
        { return compare(rhs) != 0; }

        bool operator< (text const & rhs) const noexcept
        { return compare(rhs) < 0; }

        bool operator<= (text const & rhs) const noexcept
        { return compare(rhs) <= 0; }

        bool operator> (text const & rhs) const noexcept
        { return compare(rhs) > 0; }

        bool operator>= (text const & rhs) const noexcept
        { return compare(rhs) >= 0; }

        void clear ()
        {
            size_ = 0;
            data_[0] = '\0';
        }

        // TODO: Update the char-range contraints to require random access iterators.

        char & operator[] (int i) noexcept
        {
            assert(0 <= i && i < size_);
            return data_[i];
        }

        text_view operator() (int lo, int hi) noexcept;
        text_view operator() (int lo) noexcept;

        template <typename CharRange>
        auto insert (int at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & insert (int at, text_view view);
        inline text & insert (int at, repeated_text_view rv);

        inline text & erase (text_view view);

        template <typename CharRange>
        auto replace (text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & replace (text_view old_substr, text_view new_substr);
        inline text & replace (text_view old_substr, repeated_text_view new_substr);

        void resize (int new_size, char c)
        {
            assert(0 <= new_size);

            if (c & 0x80)
                throw std::invalid_argument("Given character is not a valid UTF-8 code point");

            int const prev_size = size_;
            int const delta = new_size - prev_size;
            int const available = cap_ - 1 - size_;
            if (available < delta) {
                std::unique_ptr<char []> new_data = get_new_data(delta - available);
                std::copy(begin(), begin() + prev_size, new_data.get());
                new_data.swap(data_);
            } else {
                if (!utf8::ends_encoded(cbegin(), cbegin() + new_size))
                    throw std::invalid_argument("Given character is not a valid UTF-8 code point");
                size_ = new_size;
            }

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
        int grow_size (int min_new_size) const
        {
            assert(0 < min_new_size);
            int retval = (std::max)(8, size_);
            while (retval < min_new_size) {
                retval = retval / 2 * 3;
            }
            return retval;
        }

        std::unique_ptr<char []> get_new_data (int resize_amount)
        {
            assert(0 < resize_amount);
            int const new_size = grow_size(size_ + resize_amount);
            std::unique_ptr<char []> retval(new char [new_size + 1]);
            size_ = new_size;
            cap_ = size_ + 1;
            return retval;
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
#if 0
                utf8::from_utf16_iterator<char16_t const *>(str),
                utf8::from_utf16_iterator<char16_t const *>(str + len)
#endif
            );
        }

        inline text operator"" _t (char32_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 4);
            return text(
#if 0
                utf8::from_utf16_iterator<char32_t const *>(str),
                utf8::from_utf16_iterator<char32_t const *>(str + len)
#endif
            );
        }

        namespace detail {

            template <typename Char, int CharSize = sizeof(Char)>
            struct wchar_to_utf8_iterator;

            template <typename Char>
            struct wchar_to_utf8_iterator<Char, 2>
            { using type = utf8::from_utf16_iterator<Char const *>; };

            template <typename Char>
            struct wchar_to_utf8_iterator<Char, 4>
            { using type = utf8::from_utf32_iterator<Char const *>; };

        }

        inline text operator"" _t (wchar_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / sizeof(wchar_t));
            using iterator = typename detail::wchar_to_utf8_iterator<wchar_t>::type;
            return text(
#if 0
                iterator<wchar_t const *>(str),
                iterator<wchar_t const *>(str + len)
#endif
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

    inline text::operator text_view () const noexcept
    {
        if (!data_)
            return text_view(get_empty_str(), 0);
        return text_view(data_.get(), size_);
    }

    inline text_view text::operator() (int lo, int hi) noexcept
    { return const_cast<text const &>(*this)(lo, hi); }

    inline text_view text::operator() (int lo) noexcept
    { return const_cast<text const &>(*this)(lo); }

    template <typename CharRange>
    auto text::insert (int at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::insert (int at, text_view view)
    {
        assert(0 <= at && at <= size_);

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const view_null_terminated = !view.empty() && view.end()[-1] == '\0';
        if (view_null_terminated)
            view = view(0, -1);

        int const delta = view.size();
        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            buf = std::copy(view.begin(), view.end(), buf);
            buf = std::copy(cbegin() + at, cend(), buf);
            *buf = '\0';
            new_data.swap(data_);
        } else {
            if (0 < delta)
                std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            buf = std::copy(view.begin(), view.end(), buf);
            data_[size_] = '\0';
        }

        return *this;
    }

    inline text & text::insert (int at, repeated_text_view rv)
    {
        assert(0 <= at && at <= size_);

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const view_null_terminated = !rv.view().empty() && rv.view().end()[-1] == '\0';
        if (view_null_terminated)
            rv = repeat(rv.view()(0, -1), rv.count());

        int const delta = rv.size();
        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            for (int i = 0; i < rv.count(); ++i) {
                buf = std::copy(rv.view().begin(), rv.view().end(), buf);
            }
            buf = std::copy(cbegin() + at, cend(), buf);
            *buf = '\0';
            new_data.swap(data_);
        } else {
            if (0 < delta)
                std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            for (int i = 0; i < rv.count(); ++i) {
                buf = std::copy(rv.view().begin(), rv.view().end(), buf);
            }
            data_[size_] = '\0';
        }

        return *this;
    }

    inline text & text::erase (text_view view)
    {
        bool const view_null_terminated = !view.empty() && view.end()[-1] == '\0';
        if (view_null_terminated)
            view = view(0, -1);

        assert(begin() <= view.begin() && view.end() <= end());

        *std::copy(
            view.end(), cend(),
            const_cast<char *>(view.begin())
        ) = '\0';
        size_ -= view.size();

        return *this;
    }

    template <typename CharRange>
    auto text::replace (text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return replace(old_substr, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::replace (text_view old_substr, text_view new_substr)
    {
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
        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            buf = std::copy(new_substr.begin(), new_substr.end(), buf);
            buf = std::copy(old_substr.end(), cend(), buf);
            *buf = '\0';
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                *std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                ) = '\0';
            } else if (delta < 0) {
                *std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                ) = '\0';
            }
            char * buf = const_cast<char *>(old_substr.begin());
            std::copy(new_substr.begin(), new_substr.end(), buf);
        }

        return *this;
    }

    inline text & text::replace (text_view old_substr, repeated_text_view new_substr)
    {
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
        int const available = cap_ - 1 - size_;
        if (available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
            buf = std::copy(old_substr.end(), cend(), buf);
            *buf = '\0';
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                *std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                ) = '\0';
            } else if (delta < 0) {
                *std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                ) = '\0';
            }
            char * buf = const_cast<char *>(old_substr.begin());
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
        }

        return *this;
    }



    inline text & operator+= (text & t, text_view view)
    { return t.insert(t.size(), view); }

    inline text & operator+= (text & t, repeated_text_view rv)
    {
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
