#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/detail/utility.hpp>
#include <boost/text/detail/iterator.hpp>

#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text;

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        constexpr text_view (char const * c_str) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(size_ <= INT_MAX); }

        constexpr text_view (char const * c_str, int len) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        constexpr text_view (text const & t) noexcept;

        constexpr text_view (text_view const & rhs) noexcept :
            data_ (rhs.data_),
            size_ (rhs.size_)
        {}
        constexpr text_view & operator= (text_view const & rhs) noexcept
        {
            data_ = rhs.data_;
            size_ = rhs.size_;
            return *this;
        }

        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_iterator cbegin () const noexcept { return begin(); }
        constexpr const_iterator cend () const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

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
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

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

        friend constexpr iterator begin (text_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (text_view v) noexcept
        { return v.end(); }
        friend constexpr iterator cbegin (text_view v) noexcept
        { return v.cbegin(); }
        friend constexpr iterator cend (text_view v) noexcept
        { return v.cend(); }

        friend constexpr reverse_iterator rbegin (text_view v) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (text_view v) noexcept
        { return v.rend(); }
        friend constexpr reverse_iterator crbegin (text_view v) noexcept
        { return v.crbegin(); }
        friend constexpr reverse_iterator crend (text_view v) noexcept
        { return v.crend(); }

        friend std::ostream & operator<< (std::ostream & os, text_view view)
        { return os.write(view.data(), view.size()); }

    private:
        char const * data_;
        int size_;
    };

    namespace literals {

        inline constexpr text_view operator"" _tv (char const * str, std::size_t len) noexcept
        {
            assert(len < INT_MAX);
            return text_view(str, len);
        }

        // TODO: constexpr text_view operator"" _tv (std::char16_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::char32_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::wchar_t const * str, std::size_t len) noexcept

    }

} }

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline constexpr text_view::text_view (text const & t) noexcept :
        data_ (t.data()),
        size_ (t.size())
    {}

} }

/* Rationale

   1: use of int for sizes

   2: including null terminator in string_views

   3: TODO: Remove data(), front(), back()

*/

#endif
