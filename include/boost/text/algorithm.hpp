#ifndef BOOST_TEXT_ALGORITHM_HPP
#define BOOST_TEXT_ALGORITHM_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/detail/algorithm.hpp>

#include <boost/algorithm/searching/boyer_moore.hpp>


namespace boost { namespace text {

    // compare()

    /** Lexicographical compare.  Returns a value < 0 when lhs is
        lexicographically less than rhs, 0 if lhs == rhs, and a value > 0 if
        lhs is lexicographically greater than rhs.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    compare(string_view l, string_view r) noexcept
    {
        return detail::compare_impl(begin(l), end(l), begin(r), end(r));
    }

    /** Lexicographical compare.  Returns a value < 0 when lhs is
        lexicographically less than rhs, 0 if lhs == rhs, and a value > 0 if
        lhs is lexicographically greater than rhs.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    compare(string_view l, CharRange const & r) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return compare(l, string_view(r));
    }

    /** Lexicographical compare.  Returns a value < 0 when lhs is
        lexicographically less than rhs, 0 if lhs == rhs, and a value > 0 if
        lhs is lexicographically greater than rhs.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    compare(CharRange const & l, string_view r) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return compare(string_view(l), r);
    }

    /** Lexicographical compare.  Returns a value < 0 when lhs is
        lexicographically less than rhs, 0 if lhs == rhs, and a value > 0 if
        lhs is lexicographically greater than rhs.

        This function only participates in overload resolution if LCharRange
        and RCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename LCharRange, typename RCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    compare(LCharRange const & l, RCharRange const & r) noexcept
        -> detail::rngs_alg_ret_t<int, LCharRange, RCharRange>
    {
        return compare(string_view(l), string_view(r));
    }



    // find ()

    namespace detail {

#ifdef BOOST_TEXT_NO_CXX14_CONSTEXPR

        inline int find_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return 0;

            if (r_first == r_last)
                return -1;

            auto const pair =
                algorithm::boyer_moore_search(r_first, r_last, p_first, p_last);

            if (pair.first == pair.second)
                return -1;

            return pair.first - r_first;
        }

#else

        inline constexpr int find_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return 0;

            if (r_first == r_last)
                return -1;

            char const p_head = *p_first;
            int const p_len = p_last - p_first;
            const char * it = r_first;
            while (true) {
                int const r_len = r_last - it;
                if (r_len < p_len)
                    return -1;

                it = strchr(it, r_last, p_head);
                if (it == r_last)
                    return -1;

                string_view candidate(it, p_len);
                if (compare_impl(
                        candidate.begin(), candidate.end(), p_first, p_last) ==
                    0)
                    return it - r_first;

                ++it;
            }
        }

#endif
    }

    /** Returns the offset of the first occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the beginning of r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    find(string_view r, string_view p) noexcept
    {
        return detail::find_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the first occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find(r, string_view(p));
    }

    /** Returns the offset of the first occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find(string_view(r), p);
    }

    /** Returns the offset of the first occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return find(string_view(r), string_view(p));
    }



    // find_view ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR string_view find_view_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return string_view(r_first, 0);

            if (r_first == r_last)
                return string_view(nullptr, 0);

            int const n = find_impl(r_first, r_last, p_first, p_last);
            if (n < 0)
                return string_view(nullptr, 0);
            return string_view(r_first + n, p_last - p_first);
        }
    }

    /** Returns the first occurance of pattern p within range r as a
        string_view, or string_view() if p is not found in r.  An empty p is
        always considered to match the beginning of r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR string_view
    find_view(string_view r, string_view p) noexcept
    {
        return detail::find_view_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the first occurance of pattern p within range r as a
        string_view, or string_view() if p is not found in r.  An empty p is
        always considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_view(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<string_view, CharRange>
    {
        return find_view(r, string_view(p));
    }

    /** Returns the first occurance of pattern p within range r as a
        string_view, or string_view() if p is not found in r.  An empty p is
        always considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_view(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<string_view, CharRange>
    {
        return find_view(string_view(r), p);
    }

    /** Returns the first occurance of pattern p within range r as a
        string_view, or string_view() if p is not found in r.  An empty p is
        always considered to match the beginning of r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_view(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<string_view, CharRange, PatternCharRange>
    {
        return find_view(string_view(r), string_view(p));
    }



    // find_first_of ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR int find_first_of_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return 0;

            if (r_first == r_last)
                return -1;

            char const * r_it = r_first;
            while (r_it != r_last) {
                char const * p_it = p_first;
                while (p_it != p_last) {
                    if (*r_it == *p_it)
                        return r_it - r_first;
                    ++p_it;
                }
                ++r_it;
            }

            return -1;
        }
    }

    /** Returns the offset of the first occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    find_first_of(string_view r, string_view p) noexcept
    {
        return detail::find_first_of_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the first occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_of(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_first_of(r, string_view(p));
    }

    /** Returns the offset of the first occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_of(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_first_of(string_view(r), p);
    }

    /** Returns the offset of the first occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_of(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return find_first_of(string_view(r), string_view(p));
    }



    // find_last_of ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR int find_last_of_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return r_last - r_first;

            if (r_first == r_last)
                return -1;

            char const * r_it = r_last;
            while (r_first != r_it) {
                --r_it;
                char const * p_it = p_first;
                while (p_it != p_last) {
                    if (*r_it == *p_it)
                        return r_it - r_first;
                    ++p_it;
                }
            }
            return -1;
        }
    }

    /** Returns the offset of the last occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    find_last_of(string_view r, string_view p) noexcept
    {
        return detail::find_last_of_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the last occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_of(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_last_of(r, string_view(p));
    }

    /** Returns the offset of the last occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_of(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_last_of(string_view(r), p);
    }

    /** Returns the offset of the last occurance within range r of any of the
        chars in p, or a value < 0 if none of the chars in p is found in r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_of(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return find_last_of(string_view(r), string_view(p));
    }



    // find_first_not_of ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR int find_first_not_of_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return 0;

            if (r_first == r_last)
                return -1;

            char const * r_it = r_first;
            while (r_it != r_last) {
                char const * p_it = p_first;
                bool found = true;
                while (p_it != p_last && found) {
                    if (*r_it == *p_it)
                        found = false;
                    ++p_it;
                }
                if (found)
                    return r_it - r_first;
                ++r_it;
            }

            return -1;
        }
    }

    /** Returns the offset of the first char within range r that does not
        match any char in pattern p, or a value < 0 if every char in r is in
        p.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    find_first_not_of(string_view r, string_view p) noexcept
    {
        return detail::find_first_not_of_impl(
            begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the first char within range r that does not
        match any char in pattern p, or a value < 0 if every char in r is in
        p.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_not_of(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_first_not_of(r, string_view(p));
    }

    /** Returns the offset of the first char within range r that does not
        match any char in pattern p, or a value < 0 if every char in r is in
        p.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_not_of(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_first_not_of(string_view(r), p);
    }

    /** Returns the offset of the first char within range r that does not
        match any char in pattern p, or a value < 0 if every char in r is in
        p.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_first_not_of(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return find_first_not_of(string_view(r), string_view(p));
    }



    // find_last_not_of ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR int find_last_not_of_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return r_last - r_first;

            if (r_first == r_last)
                return -1;

            char const * r_it = r_last;
            while (r_first != r_it) {
                --r_it;
                char const * p_it = p_first;
                bool found = true;
                while (p_it != p_last && found) {
                    if (*r_it == *p_it)
                        found = false;
                    ++p_it;
                }
                if (found)
                    return r_it - r_first;
            }
            return -1;
        }
    }

    /** Returns the offset of the last char within range r that does not match
        any char in pattern p, or a value < 0 if every char in r is in p.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    find_last_not_of(string_view r, string_view p) noexcept
    {
        return detail::find_last_not_of_impl(
            begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the last char within range r that does not match
        any char in pattern p, or a value < 0 if every char in r is in p.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_not_of(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_last_not_of(r, string_view(p));
    }

    /** Returns the offset of the last char within range r that does not match
        any char in pattern p, or a value < 0 if every char in r is in p.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_not_of(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return find_last_not_of(string_view(r), p);
    }

    /** Returns the offset of the last char within range r that does not match
        any char in pattern p, or a value < 0 if every char in r is in p.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    find_last_not_of(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return find_last_not_of(string_view(r), string_view(p));
    }



    // rfind ()

    namespace detail {

#ifdef BOOST_TEXT_NO_CXX14_CONSTEXPR

        inline int rfind_impl(
            char const * r_first_,
            char const * r_last_,
            char const * p_first_,
            char const * p_last_) noexcept
        {
            if (p_first_ == p_last_)
                return r_last_ - r_first_;

            if (r_first_ == r_last_)
                return -1;

            const_reverse_char_iterator r_first(r_last_);
            const_reverse_char_iterator r_last(r_first_);
            const_reverse_char_iterator p_first(p_last_);
            const_reverse_char_iterator p_last(p_first_);
            auto const pair =
                algorithm::boyer_moore_search(r_first, r_last, p_first, p_last);

            if (pair.first == pair.second)
                return -1;

            return pair.second.base() - r_first_;
        }

#else

        inline constexpr int rfind_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return r_last - r_first;

            if (r_first == r_last)
                return -1;

            char const p_head = *p_first;
            int const p_len = p_last - p_first;
            const char * it = r_last;
            while (true) {
                int const r_len = it - r_first;
                if (r_len < p_len)
                    return -1;

                auto candidate_first = strrchr(r_first, it, p_head);
                if (candidate_first == r_last)
                    return -1;

                string_view candidate(candidate_first, p_len);
                if (compare_impl(
                        candidate.begin(), candidate.end(), p_first, p_last) ==
                    0)
                    return candidate_first - r_first;

                it = candidate_first;
            }
        }

#endif
    }

    /** Returns the offset of the last occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    rfind(string_view r, string_view p) noexcept
    {
        return detail::rfind_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the offset of the last occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return rfind(r, string_view(p));
    }

    /** Returns the offset of the last occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    {
        return rfind(string_view(r), p);
    }

    /** Returns the offset of the last occurance of pattern p within range r,
        or a value < 0 if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    {
        return rfind(string_view(r), string_view(p));
    }



    // rfind_view ()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR string_view rfind_view_impl(
            char const * r_first,
            char const * r_last,
            char const * p_first,
            char const * p_last) noexcept
        {
            if (p_first == p_last)
                return string_view(r_last, 0);

            if (r_first == r_last)
                return string_view(nullptr, 0);

            int const n = rfind_impl(r_first, r_last, p_first, p_last);
            if (n < 0)
                return string_view(nullptr, 0);
            return string_view(r_first + n, p_last - p_first);
        }
    }

    /** Returns the last occurance of pattern p within range r as a string_view,
        or string_view() if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR string_view
    rfind_view(string_view r, string_view p) noexcept
    {
        return detail::rfind_view_impl(begin(r), end(r), begin(p), end(p));
    }

    /** Returns the last occurance of pattern p within range r as a string_view,
        or string_view() if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind_view(string_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<string_view, CharRange>
    {
        return rfind_view(r, string_view(p));
    }

    /** Returns the last occurance of pattern p within range r as a string_view,
        or string_view() if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind_view(CharRange const & r, string_view p) noexcept
        -> detail::rng_alg_ret_t<string_view, CharRange>
    {
        return rfind_view(string_view(r), p);
    }

    /** Returns the last occurance of pattern p within range r as a string_view,
        or string_view() if p is not found in r.  An empty p is always
        considered to match the end of r.

        This function only participates in overload resolution if CharRange
        and PatternCharRange each model the CharRange concept.

        This function is constexpr in C++14 and later. */
    template<typename CharRange, typename PatternCharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    rfind_view(CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<string_view, CharRange, PatternCharRange>
    {
        return rfind_view(string_view(r), string_view(p));
    }



    // front(), back()

    namespace detail {

        inline BOOST_TEXT_CXX14_CONSTEXPR char
        front_impl(char const * first, char const * last) noexcept
        {
            assert(first != last);
            return *first;
        }

        // back_impl() is in detail/algorithm.hpp to break a dependency.
    }

    /** Returns the first char of range r.

        This function is constexpr in C++14 and later.

        \pre !r.empty() */
    inline BOOST_TEXT_CXX14_CONSTEXPR char front(string_view r) noexcept
    {
        return detail::front_impl(begin(r), end(r));
    }

    /** Returns the first char of range r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later.

        \pre r.begin() != r.end() */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto front(CharRange const & r) noexcept
        -> detail::rng_alg_ret_t<char, CharRange>
    {
        return front(string_view(r));
    }

    /** Returns the last char of range r.

        This function is constexpr in C++14 and later.

        \pre !r.empty() */
    inline BOOST_TEXT_CXX14_CONSTEXPR char back(string_view r) noexcept
    {
        return detail::back_impl(begin(r), end(r));
    }

    /** Returns the last char of range r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept.

        This function is constexpr in C++14 and later.

        \pre r.begin() != r.end() */
    template<typename CharRange>
    BOOST_TEXT_CXX14_CONSTEXPR auto back(CharRange const & r) noexcept
        -> detail::rng_alg_ret_t<char, CharRange>
    {
        return back(string_view(r));
    }

    /** Range-friendly version of std::find(), taking an iterator and a
        sentinel. */
    template<typename BidiIter, typename Sentinel, typename T>
    BidiIter find(BidiIter first, Sentinel last, T const & x)
    {
        while (first != last) {
            if (*first == x)
                return first;
            ++first;
        }
        return first;
    }

    /** A range-friendly compliment to std::find(); returns an iterator to the
        first element not equal to \a x. */
    template<typename BidiIter, typename Sentinel, typename T>
    BidiIter find_not(BidiIter first, Sentinel last, T const & x)
    {
        while (first != last) {
            if (*first != x)
                return first;
            ++first;
        }
        return first;
    }

    /** Range-friendly version of std::find_if(), taking an iterator and a
        sentinel. */
    template<typename BidiIter, typename Sentinel, typename Pred>
    BidiIter find_if(BidiIter first, Sentinel last, Pred p)
    {
        while (first != last) {
            if (p(*first))
                return first;
            ++first;
        }
        return first;
    }

    /** Range-friendly version of std::find_if_not(), taking an iterator and a
        sentinel. */
    template<typename BidiIter, typename Sentinel, typename Pred>
    BidiIter find_if_not(BidiIter first, Sentinel last, Pred p)
    {
        while (first != last) {
            if (!p(*first))
                return first;
            ++first;
        }
        return first;
    }

    /** Analogue of std::find() that finds the last value in [first, last)
        equal to \a x. */
    template<typename BidiIter, typename T>
    BidiIter find_backward(BidiIter first, BidiIter last, T const & x)
    {
        auto it = last;
        while (it != first) {
            if (*--it == x)
                return it;
        }
        return last;
    }

    /** Analogue of std::find() that finds the last value in [first, last) not
        equal to \a x. */
    template<typename BidiIter, typename T>
    BidiIter find_not_backward(BidiIter first, BidiIter last, T const & x)
    {
        auto it = last;
        while (it != first) {
            if (*--it != x)
                return it;
        }
        return last;
    }

    /** Analogue of std::find() that finds the last value \a v in [first,
        last) for which <code>p(v)</code> is true. */
    template<typename BidiIter, typename Pred>
    BidiIter find_if_backward(BidiIter first, BidiIter last, Pred p)
    {
        auto it = last;
        while (it != first) {
            if (p(*--it))
                return it;
        }
        return last;
    }

    /** Analogue of std::find() that finds the last value \a v in [first,
        last) for which <code>p(v)</code> is false. */
    template<typename BidiIter, typename Pred>
    BidiIter find_if_not_backward(BidiIter first, BidiIter last, Pred p)
    {
        auto it = last;
        while (it != first) {
            if (!p(*--it))
                return it;
        }
        return last;
    }

    /** A utility range type returned by foreach_subrange*(). */
    template<typename Iter, typename Sentinel = Iter>
    struct foreach_subrange_range
    {
        using iterator = Iter;
        using sentinel = Sentinel;

        foreach_subrange_range() {}
        foreach_subrange_range(iterator first, sentinel last) :
            first_(first),
            last_(last)
        {}

        iterator begin() const noexcept { return first_; }
        sentinel end() const noexcept { return last_; }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Calls <code>f(sub)</code> for each subrange \a sub in [first, last).
        A subrange is a contiguous subsequence of elements, each of which is
        equal to \a x.  Subranges passed to \a f are non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename T, typename Func>
    Func foreach_subrange(FwdIter first, Sentinel last, T const & x, Func f)
    {
        using value_type = typename std::iterator_traits<FwdIter>::value_type;
        while (first != last) {
            first = find(first, last, x);
            auto next = find_if_not(first, last, [&x](value_type const & elem) {
                return elem == x;
            });
            f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** Calls <code>f(sub)</code> for each subrange \a sub in [first, last).
        A subrange is a contiguous subsequence of elements \a ei for which
        <code>p(ei)</code> is true.  Subranges passed to \a f are
        non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename Pred, typename Func>
    Func foreach_subrange_if(FwdIter first, Sentinel last, Pred p, Func f)
    {
        while (first != last) {
            first = find_if(first, last, p);
            auto next = find_if_not(first, last, p);
            f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** TODO: foreach_partition(), range overloads of the above. */

    // TODO: Separate constexpr versions into their own namespace and header,
    // and only provide the non-constexpr ones here.

}}

#endif
