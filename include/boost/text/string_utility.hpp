#ifndef BOOST_TEXT_STRING_UTILITY_HPP
#define BOOST_TEXT_STRING_UTILITY_HPP

#include <boost/text/string.hpp>
#include <boost/text/detail/sentinel_tag.hpp>


namespace boost { namespace text {

    namespace detail {

        template<typename CPIter, typename Sentinel>
        auto to_string_impl(CPIter first, Sentinel last, sentinel_tag)
            -> detail::cp_iter_ret_t<string, CPIter>
        {
            return string(
                utf8::from_utf32_iterator<CPIter, Sentinel>(first, first, last),
                last);
        }

        template<typename CPIter>
        auto to_string_impl(CPIter first, CPIter last, non_sentinel_tag)
            -> detail::cp_iter_ret_t<string, CPIter>
        {
            return string(
                utf8::from_utf32_iterator<CPIter>(first, first, last),
                utf8::from_utf32_iterator<CPIter>(first, last, last));
        }
    }

    /** Returns a UTF-8-encoded string constructed from [first, last). */
    template<typename CPIter, typename Sentinel>
    auto to_string(CPIter first, Sentinel last)
        -> detail::cp_iter_ret_t<string, CPIter>
    {
        return detail::to_string_impl(
            first,
            last,
            typename std::conditional<
                std::is_same<CPIter, Sentinel>::value,
                detail::non_sentinel_tag,
                detail::sentinel_tag>::type{});
    }

}}

#endif
