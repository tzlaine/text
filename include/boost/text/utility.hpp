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
        using iterator = utf8::to_utf32_iterator<char const *, char const *>;

        utf32_range() :
            first_(nullptr, nullptr, nullptr),
            last_(nullptr, nullptr, nullptr)
        {}
        utf32_range(char const * f, char const * l) :
            first_(f, f, l),
            last_(f, l, l)
        {}
        utf32_range(iterator f, iterator l) : first_(f), last_(l) {}
        /** TODO: This is the discontiguous CharRange. */
        template<typename CharRange>
        utf32_range(CharRange const & r) :
            first_(std::begin(r), std::begin(r), std::end(r)),
            last_(std::begin(r), std::end(r), std::end(r))
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

    /** TODO */
    template<typename CPIter, typename Sentinel = CPIter>
    struct cp_range
    {
        using iterator = CPIter;
        using sentinel = Sentinel;

        cp_range() {}
        cp_range(iterator first, sentinel last) : first_(first), last_(last) {}

        bool empty() const noexcept { return first_ == last_; }

        iterator begin() const { return first_; }
        sentinel end() const { return last_; }

        friend bool operator==(cp_range lhs, cp_range rhs)
        {
            return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
        }
        friend bool operator!=(cp_range lhs, cp_range rhs)
        {
            return !(lhs == rhs);
        }

    private:
        iterator first_;
        sentinel last_;
    };

    namespace detail {
        template<typename T>
        using remove_cv_ref_t = typename std::remove_cv<
            typename std::remove_reference<T>::type>::type;
    }

}}

#endif
