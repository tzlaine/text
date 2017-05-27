#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <memory>
#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text_view;
    struct repeated_text_view;

    // TODO: Strong exception safety guarantee.
    // TODO: Guarantee always 0-terminated.
    // TODO: Guarantee always valid UTF-8.
    // TODO: Externalize +=.

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () noexcept : data_ (nullptr), size_ (0), cap_ (0) {}

        text (text const & t) :
            data_ (new char[t.cap_]),
            size_ (t.size_),
            cap_ (t.cap_)
        { std::copy(t.begin(), t.end(), begin()); }

        text (text && rhs) noexcept :
            data_ (std::move(rhs.data_)),
            size_ (rhs.size_),
            cap_ (rhs.cap_)
        {
            rhs.size_ = 0;
            rhs.cap_ = 0;
        }

        text (char const * c_str);

        template <typename CharRange>
        explicit text (
            CharRange const & r,
            detail::rng_alg_ret_t<int *, CharRange> enable = 0
        )
        { *this += r; }

        inline explicit text (text_view view);
        inline explicit text (repeated_text_view view);

        text & operator= (text const & t)
        {
            clear();
            return *this += t;
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

        iterator begin () noexcept { return data_.get(); }
        iterator end () noexcept { return data_.get() + size_; }

        const_iterator begin () const noexcept { return data_.get(); }
        const_iterator end () const noexcept { return data_.get() + size_; }

        const_iterator cbegin () const noexcept { return data_.get(); }
        const_iterator cend () const noexcept { return data_.get() + size_; }

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
            assert(i < size_);
            return data_[i];
        }

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
        { size_ = 0; }

        // TODO: Free function in terms of operator+=() --> append ()
        // TODO: Update the char-range contraints to require random access iterators.

        char & operator[] (int i) noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        template <typename CharRange>
        auto operator+= (CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & operator+= (text_view view);
        inline text & operator+= (repeated_text_view rv);

        template <typename CharRange>
        auto insert (int at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & insert (int at, text_view view);
        inline text & insert (int at, repeated_text_view rv);

        inline text & erase (text_view view);
        inline text & replace (text_view old_substr, text_view new_substr);

        void resize (int new_size, char c)
        {
            int const delta = new_size - size_;
            int const available = cap_ - 1 - size_;
            if (available < delta) {
                std::unique_ptr<char []> new_data = get_new_data(delta - available);
                char * buf = new_data.get();
                buf = std::copy(begin(), end(), buf);
                *buf = '\0';
                new_data.swap(data_);
            } else {
                size_ = new_size;
                data_[size_] = '\0';
            }
        }

        void reserve (int new_size)
        {
            assert(0 < new_size);
            int const new_cap = new_size + 1;
            if (new_cap <= cap_)
                return;
            std::unique_ptr<char []> new_data(new char[new_cap]);
            std::copy(cbegin(), cend() + 1, new_data.get());
            data_.swap(new_data);
            cap_ = new_cap;
        }

        void shrink_to_fit ()
        {
            if (cap_ == 0 || cap_ == size_ + 1)
                return;
            std::unique_ptr<char []> new_data(new char[size_ + 1]);
            std::copy(cbegin(), cend() + 1, new_data.get());
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
            int retval = size_;
            while (retval < min_new_size) {
                retval = retval / 2 * 3;
            }
            return retval;
        }

        std::unique_ptr<char []> get_new_data (int resize_amount)
        {
            int new_size = grow_size(size_ + resize_amount);
            std::unique_ptr<char []> retval(new char [new_size + 1]);
            size_ = new_size;
            cap_ = size_ + 1;
            return retval;
        }

        std::unique_ptr<char []> data_;
        int size_;
        int cap_;
    };

} }

#include <boost/text/text_view.hpp>

namespace boost { namespace text {

    namespace literals {

        inline text operator"" _t (char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            return text(text_view(str, len));
        }

        // TODO: constexpr text_view operator"" _tv (std::char16_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::char32_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::wchar_t const * str, std::size_t len) noexcept

    }

    inline text::text (char const * c_str) :
        data_ (),
        size_ (0),
        cap_ (0)
    { *this += c_str; }

    inline text::text (text_view view) :
        data_ (),
        size_ (0),
        cap_ (0)
    {
        *this += view;
#if 0 // TODO
        if (!view.empty()) {
            if (detail::back_impl(view.begin(), view.end()) == '\0')
                --size_;
            else
                ++cap_;
            data_.reset(new char[cap_]);
            memcpy(data_.get(), view.begin(), view.size());
            data_[size_] = '\0';
        }
#endif
    }

    inline text::text (repeated_text_view rv) :
        data_ (),
        size_ (0),
        cap_ (0)
    { *this += rv; }

    template <typename CharRange>
    auto text::operator= (CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return *this = text_view(&*r.begin(), r.end() - r.begin()); }

    inline text & text::operator= (text_view view)
    {
        if (view.size() <= size()) {
            clear();
            *this += view;
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
            *this += rv;
        } else {
            text tmp(rv);
            swap(tmp);
        }
        return *this;
    }

    inline text::operator text_view () const noexcept
    {
        if (empty())
            return text_view();
        return text_view(data_.get(), size_);
    }

    template <typename CharRange>
    auto text::operator+= (CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return *this += text_view(&*r.begin(), r.end() - r.begin()); }

    inline text & text::operator+= (text_view view)
    { return insert(size_, view); }

    inline text & text::operator+= (repeated_text_view rv)
    {
        reserve(size_ + rv.size());
        for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
            *this += rv.view();
        }
        return *this;
    }

    template <typename CharRange>
    auto text::insert (int at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::insert (int at, text_view view)
    {
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
        assert(begin() <= view.begin() && view.end() <= end());
        *std::copy(
            view.end(), cend(),
            const_cast<char *>(view.begin())
        ) = '\0';
        size_ -= view.size();
        return *this;
    }

    inline text & text::replace (text_view old_substr, text_view new_substr)
    {
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

} }

#endif
