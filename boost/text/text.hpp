#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

namespace boost { namespace text {

    struct text;
    struct text_view;

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;

        text_view () :
            data_ (nullptr),
            size_ (0)
        {}

        text_view (char const * c_str) :
            data_ (c_str),
            size_ (strlen(c_str))
        {}

        text_view (char const * c_str, int len) :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        text_view (text const & t);

        char const * data() const
        { return data_; }

        int size () const
        { return size_; }

        const_iterator begin () const { return data_; }
        const_iterator end () const { return data_ + size_; }

    private:
        char const * data_;
        int size_;
    };

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;

        text () : data_ (nullptr), size_ (0) {}

        text (text const & t) :
            data_ (new char[t.size()]),
            size_ (t.size())
        { strncpy(data_.get(), t.data(), t.size()); }

        text & operator= (text const & t) :
            data_ (new char[t.size()]),
            size_ (t.size())
        { strncpy(data_.get(), t.data(), t.size()); }

        text (text &&) = default;
        text & operator= (text &&) = default;

        text (char const * c_str) :
            data_ (nullptr),
            size_ (0)
        {
            auto len = strlen(c_str);
            if (INT_MAX < len)
                throw TODO;
            data_.reset(new char[len]);
            strncpy(data_.get(), c_str, len);
            size_ = (int)len;
        }

        text (text_view view) :
            data_ (new char[view.size()]),
            size_ (view.size())
        { strncpy(data_.get(), view.data(), view.size()); }

        char const * data() const
        { return data_; }

        int size () const
        { return size_; }

        const_iterator begin () const { return data_.get(); }
        const_iterator end () const { return data_.get() + size_; }

        iterator begin () { return data_.get(); }
        iterator end () { return data_.get() + size_; }

    private:
        std::unique_ptr<char[]> data_;
        int size_;
    };

    text_view::text_view (text const & t) :
        data_ (t.data()),
        size_ (t.size())
    {}

} }

#endif
