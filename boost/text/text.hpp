#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/detail/iterator.hpp>

#include <memory>
#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text_view;

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () noexcept : data_ (nullptr), size_ (0), cap_ (0) {}

        text (text const & t) :
            data_ (new char[t.size()]),
            size_ (t.size()),
            cap_ (t.size())
        { memcpy(data_.get(), t.data(), t.size()); }

        text & operator= (text const & t)
        {
            data_.reset(new char[t.size()]);
            size_ = t.size();
            cap_ = t.size();
            memcpy(data_.get(), t.data(), t.size());
            return *this;
        }

        text (text && rhs) noexcept :
            data_ (std::move(rhs.data_)),
            size_ (rhs.size_),
            cap_ (rhs.cap_)
        {
            rhs.size_ = 0;
            rhs.cap_ = 0;
        }

        text & operator= (text && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        text (char const * c_str) :
            data_ (nullptr),
            size_ (0),
            cap_ (0)
        {
            auto const len = strlen(c_str);
            if (INT_MAX < len + 1)
                throw std::bad_alloc();
            data_.reset(new char[len + 1]);
            memcpy(data_.get(), c_str, len + 1);
            size_ = (int)len;
        }

        text (text_view view);

        const_iterator begin () const noexcept { return data_.get(); }
        const_iterator end () const noexcept { return data_.get() + size_; }

        iterator begin () noexcept { return data_.get(); }
        iterator end () noexcept { return data_.get() + size_; }

        bool empty () const noexcept
        { return size_ == 0; }

        int size () const noexcept
        { return size_; }

        char const * data() const noexcept
        { return data_.get(); }

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

        // TODO

        friend iterator begin (text & t) noexcept
        { return t.begin(); }
        friend iterator end (text & t) noexcept
        { return t.end(); }

        friend const_iterator begin (text const & t) noexcept
        { return t.begin(); }
        friend const_iterator end (text const & t) noexcept
        { return t.end(); }

    private:
        std::unique_ptr<char[]> data_;
        int size_;
        int cap_;
    };

} }

#include <boost/text/text_view.hpp>

namespace boost { namespace text {

    inline text::text (text_view view) :
        data_ (new char[view.size() + 1]),
        size_ (view.size() + 1),
        cap_ (view.size())
    {
        memcpy(data_.get(), view.data(), view.size());
        data_[size_] = '\0';
    }

} }

#endif
