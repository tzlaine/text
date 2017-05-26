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
        { memcpy(data_.get(), t.begin(), t.cap_); }

        text (text && rhs) noexcept :
            data_ (std::move(rhs.data_)),
            size_ (rhs.size_),
            cap_ (rhs.cap_)
        {
            rhs.size_ = 0;
            rhs.cap_ = 0;
        }

        text (char const * c_str) :
            data_ (nullptr),
            size_ (0),
            cap_ (0)
        {
            auto const len = strlen(c_str);
            auto const cap = len + 1;
            if (INT_MAX < cap)
                throw std::bad_alloc();
            data_.reset(new char[cap]);
            memcpy(data_.get(), c_str, cap);
            size_ = (int)len;
            cap_ = cap;
        }

        explicit text (text_view view);
        explicit text (repeated_text_view view);

        text & operator= (text const & t)
        {
            data_.reset(new char[t.cap_]);
            size_ = t.size_;
            cap_ = t.cap_;
            memcpy(data_.get(), t.begin(), t.cap_);
            return *this;
        }

        text & operator= (text && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        text & operator= (text_view view);
        text & operator= (repeated_text_view view);

#if 0
        template <typename InputIter>
        text (InputIter first, InputIter last)
        {
            // TODO
        }
#endif

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

#if 0
        // TODO

        insert ();
        erase ();

        // TODO: Free function in terms of operator+=() --> append ()

        replace ();
#endif

        template <typename CharRange>
        auto operator+= (CharRange const & r) noexcept
            -> detail::rng_alg_ret_t<text_view, CharRange>
        {
            // TODO
            return *this;
        }

        text & operator+= (text_view view);
        text & operator+= (repeated_text_view rv);

        char & operator[] (int i) noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        void reserve (int new_size)
        {
            int const new_cap = new_size + 1;
            if (new_cap <= cap_)
                return;
            std::unique_ptr<char[]> new_data(new char[new_cap]);
            memcpy(new_data.get(), data_.get(), cap_);
            data_.swap(new_data);
            cap_ = new_cap;
        }

        void shrink_to_fit ()
        {
            if (cap_ == 0 || cap_ == size_ + 1)
                return;
            std::unique_ptr<char[]> new_data(new char[size_ + 1]);
            memcpy(new_data.get(), data_.get(), size_ + 1);
            data_.swap(new_data);
            cap_ = size_ + 1;
        }

        void swap (text & rhs) noexcept
        {
            data_ = std::move(rhs.data_);
            {
                int const tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
            {
                int const tmp = cap_;
                cap_ = rhs.cap_;
                rhs.cap_ = tmp;
            }
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
        std::unique_ptr<char[]> data_;
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

    inline text::text (text_view view) :
        data_ (),
        size_ (view.size()),
        cap_ (view.size())
    {
        if (!view.empty()) {
            if (detail::back_impl(view.begin(), view.end()) == '\0')
                --size_;
            else
                ++cap_;
            data_.reset(new char[cap_]);
            memcpy(data_.get(), view.begin(), view.size());
            data_[size_] = '\0';
        }
    }

    inline text::text (repeated_text_view rv) :
        data_ (),
        size_ (rv.size()),
        cap_ (rv.size())
    {
        reserve(rv.size());
        *this += rv;
        data_[size_] = '\0';
    }

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

    inline text & text::operator+= (text_view view)
    {
        // TODO
        return *this;
    }

    inline text & text::operator+= (repeated_text_view rv)
    {
        for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
            *this += rv.view();
        }
        return *this;
    }

} }

#endif
