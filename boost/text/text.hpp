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

        text () : data_ (nullptr), size_ (0), cap_ (0) {}

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

        text (text &&) = default;
        text & operator= (text &&) = default;

        text (char const * c_str) :
            data_ (nullptr),
            size_ (0),
            cap_ (0)
        {
            auto len = strlen(c_str) + 1;
            if (INT_MAX < len)
                throw std::bad_alloc();
            data_.reset(new char[len]);
            memcpy(data_.get(), c_str, len);
            size_ = (int)len;
        }

        text (text_view view);

        const_iterator begin () const { return data_.get(); }
        const_iterator end () const { return data_.get() + size_; }

        iterator begin () { return data_.get(); }
        iterator end () { return data_.get() + size_; }

        bool empty () const
        { return size_ == 0; }

        int size () const
        { return size_; }

        char const * data() const
        { return data_.get(); }

        // TODO

        friend iterator begin (text & t)
        { return t.begin(); }
        friend iterator end (text & t)
        { return t.end(); }

        friend const_iterator begin (text const & t)
        { return t.begin(); }
        friend const_iterator end (text const & t)
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
        data_ (new char[view.size()]),
        size_ (view.size()),
        cap_ (view.size())
    { memcpy(data_.get(), view.data(), view.size()); }

} }

#endif
