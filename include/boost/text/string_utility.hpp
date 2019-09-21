#ifndef BOOST_TEXT_STRING_UTILITY_HPP
#define BOOST_TEXT_STRING_UTILITY_HPP

#include <boost/text/string.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/sentinel_tag.hpp>


namespace boost { namespace text {

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns a UTF-8-encoded string constructed from [first, last).

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    string to_string(CPIter first, Sentinel last);

    /** Returns a UTF-8-encoded string constructed from range.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept. */
    template<typename CPRange>
    string to_string(CPRange & range);

#else

    template<typename CPIter, typename Sentinel>
    auto to_string(CPIter first, Sentinel last)
        -> detail::cp_iter_ret_t<string, CPIter>
    {
        auto const r = as_utf8(first, last);
        return string(r.begin(), r.end());
    }

    template<typename CPRange>
    auto to_string(CPRange & range) -> detail::cp_rng_alg_ret_t<string, CPRange>
    {
        return boost::text::to_string(std::begin(range), std::end(range));
    }

#endif

}}

#endif
