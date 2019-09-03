#ifndef BOOST_TEXT_STRING_UTILITY_HPP
#define BOOST_TEXT_STRING_UTILITY_HPP

#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>
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
        cp_range<CPIter, Sentinel> cps{first, last};
        auto const r = make_from_utf32_range(cps);
        return string(r.begin(), r.end());
    }

    template<typename CPRange>
    auto to_string(CPRange & range) -> detail::cp_rng_alg_ret_t<string, CPRange>
    {
        auto const r = make_from_utf32_range(range);
        return string(std::begin(r), std::end(r));
    }

#endif

}}

#endif
