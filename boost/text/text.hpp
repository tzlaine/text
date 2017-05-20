#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <algorithm>
#include <stdexcept>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


namespace boost { namespace text {

    struct text;
    struct text_view;

    namespace detail {

        constexpr std::size_t strlen (char const * c_str) noexcept
        {
            std::size_t retval = 0;
            while (c_str) {
                retval += 1;
                ++c_str;
            }
            return retval;
        }

    }

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = char const *;
        using const_reverse_iterator = char const *;

        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        constexpr text_view (char const * c_str) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(detail::strlen(c_str) < INT_MAX); }

        constexpr text_view (char const * c_str, int len) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        constexpr text_view (text const & t) noexcept;

        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_iterator cbegin () const noexcept { return begin(); }
        constexpr const_iterator cend () const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin () const noexcept { return end() - 1; }
        constexpr const_reverse_iterator rend () const noexcept { return begin() - 1; }

        constexpr const_reverse_iterator crbegin () const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend () const noexcept { return rend(); }

        constexpr bool empty () const noexcept
        { return size_ == 0; }

        constexpr int size () const noexcept
        { return size_; }

        constexpr char const * data() const noexcept
        { return data_; }

        constexpr char front () const noexcept
        {
            assert(!empty());
            return *data_;
        }

        constexpr char back () const noexcept
        {
            assert(!empty());
            return data_[size_ - 1];
        }

        constexpr char operator[] (int i) const noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        constexpr int max_size () const noexcept
        { return INT_MAX; }

        // TODO: operator<=> () const
        constexpr int compare (text_view rhs) const noexcept
        {
            int const size = (std::min)(size_, rhs.size_);
            if (size == 0)
                return 0;
            int retval = memcmp(data_, rhs.data_, size);
            if (retval == 0) {
                if (size_ < rhs.size_) return -1;
                if (size_ == rhs.size_) return 0;
                return 1;
            }
            return retval;
        }

        constexpr bool operator== (text_view rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (text_view rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (text_view rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (text_view rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (text_view rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (text_view rhs) const noexcept
        { return compare(rhs) >= 0; }

        constexpr void swap (text_view & rhs) noexcept
        {
            {
                char const * tmp = data_;
                data_ = rhs.data_;
                rhs.data_ = tmp;
            }
            {
                int tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
        }

    private:
        char const * data_;
        int size_;
    };

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = char const *;
        using const_reverse_iterator = char const *;

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

        text (text_view view) :
            data_ (new char[view.size()]),
            size_ (view.size()),
            cap_ (view.size())
        { memcpy(data_.get(), view.data(), view.size()); }

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

    private:
        std::unique_ptr<char[]> data_;
        int size_;
        int cap_;
    };

    constexpr text_view::text_view (text const & t) noexcept :
        data_ (t.data()),
        size_ (t.size())
    {}

} }

#endif
