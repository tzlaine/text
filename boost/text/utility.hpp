#ifndef BOOST_TEXT_UTILITY_HPP
#define BOOST_TEXT_UTILITY_HPP

#include <boost/text/string.hpp>
#include <boost/text/utf8.hpp>


namespace boost { namespace text {

    /** TODO */
    template<typename Iter>
    string to_string(Iter first, Iter last)
    {
        return string(
            utf8::from_utf32_iterator<Iter>(first),
            utf8::from_utf32_iterator<Iter>(last));
    }

    /** TODO */
    struct utf32_range
    {
        using iterator = utf8::to_utf32_iterator<char const *>;

        utf32_range() : first_(nullptr), last_(nullptr) {}
        utf32_range(char const * f, char const * l) : first_(f), last_(l) {}
        utf32_range(iterator f, iterator l) : first_(f), last_(l) {}
        /** TODO: This is the discontiguous CharRange. */
        template<typename CharRange>
        utf32_range(CharRange const & r) :
            first_(std::begin(r)),
            last_(std::end(r))
        {}

        bool empty() const noexcept { return first_ == last_; }

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

        friend bool operator==(utf32_range lhs, utf32_range rhs)
        {
            return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
        }
        friend bool operator!=(utf32_range lhs, utf32_range rhs)
        {
            return !(lhs == rhs);
        }

    private:
        iterator first_;
        iterator last_;
    };

}}

#endif
