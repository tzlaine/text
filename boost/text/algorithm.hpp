#ifndef BOOST_TEXT_ALGORITHM_HPP
#define BOOST_TEXT_ALGORITHM_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/detail/algorithm.hpp>


namespace boost { namespace text {

    // compare()

    template <typename LCharRange, typename RCharRange>
    constexpr auto compare (LCharRange const & l, RCharRange const & r) noexcept
        -> detail::rngs_alg_ret_t<int, LCharRange, RCharRange>
    { return detail::compare_impl(&*begin(l), &*end(l), &*begin(r), &*end(r)); }

    inline constexpr int compare (text_view l, text_view r) noexcept
    { return detail::compare_impl(begin(l), end(l), begin(r), end(r)); }

    template <typename CharRange>
    constexpr auto compare (text_view l, CharRange const & r) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::compare_impl(begin(l), end(l), &*begin(r), &*end(r)); }

    template <typename CharRange>
    constexpr auto compare (CharRange const & l, text_view r) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::compare_impl(&*begin(l), &*end(l), begin(r), end(r)); }



    // find ()

    namespace detail {

        inline constexpr int find_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
                return -1;

            char const p_head = *p_first;
            int const p_len = p_last - p_first;
            const char * it = r_first;
            while (true) {
                int const r_len = r_last - it;
                if (r_len < p_len)
                    return -1;

                it = strchr(it, p_head);
                if (it == nullptr)
                    return -1;

                text_view candidate(it, p_len);
                if (compare_impl(candidate.begin(), candidate.end(), p_first, p_last) == 0)
                    return it - r_first;

                ++it;
            }
        }
    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    { return detail::find_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int find (text_view r, text_view p) noexcept
    { return detail::find_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // find_view ()

    namespace detail {

        inline constexpr text_view find_view_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return text_view(r_first, 0);

            if (p_first == p_last)
                return text_view(r_first, 0);

            int const n = find_impl(r_first, r_last, p_first, p_last);
            if (n < 0)
                return text_view(r_last, 0);
            return text_view(r_first + n, p_last - p_first);
        }

    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_view (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::find_view_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr text_view find_view (text_view r, text_view p) noexcept
    { return detail::find_view_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find_view (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<text_view, CharRange>
    { return detail::find_view_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find_view (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<text_view, CharRange>
    { return detail::find_view_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // find_first_of ()

    namespace detail {

        inline constexpr int find_first_of_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_first_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::find_first_of_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int find_first_of (text_view r, text_view p) noexcept
    { return detail::find_first_of_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find_first_of (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_first_of_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find_first_of (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_first_of_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // find_last_of ()

    namespace detail {

        inline constexpr int find_last_of_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_last_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::find_last_of_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int find_last_of (text_view r, text_view p) noexcept
    { return detail::find_last_of_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find_last_of (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_last_of_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find_last_of (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_last_of_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // find_first_not_of ()

    namespace detail {

        inline constexpr int find_first_not_of_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
                return -1;

            char const * r_it = r_first;
            while (r_it != r_last) {
                char const * p_it = p_first;
                while (p_it != p_last) {
                    if (*r_it != *p_it)
                        return r_it - r_first;
                    ++p_it;
                }
                ++r_it;
            }

            return -1;
        }

    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_first_not_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::find_first_not_of_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int find_first_not_of (text_view r, text_view p) noexcept
    { return detail::find_first_not_of_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find_first_not_of (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_first_not_of_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find_first_not_of (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_first_not_of_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // find_last_not_of ()

    namespace detail {

        inline constexpr int find_last_not_of_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
                return -1;

            char const * r_it = r_last;
            while (r_first != r_it) {
                --r_it;
                char const * p_it = p_first;
                while (p_it != p_last) {
                    if (*r_it != *p_it)
                        return r_it - r_first;
                    ++p_it;
                }
            }
            return -1;
        }

    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_last_not_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::find_last_not_of_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int find_last_not_of (text_view r, text_view p) noexcept
    { return detail::find_last_not_of_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto find_last_not_of (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_last_not_of_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto find_last_not_of (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::find_last_not_of_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // rfind ()

    namespace detail {

        inline constexpr int rfind_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return -1;

            if (p_first == p_last)
                return -1;

            char const p_head = *p_first;
            int const p_len = p_last - p_first;
            const char * it = r_last;
            while (true) {
                int const r_len = it - r_first;
                if (r_len < p_len)
                    return -1;

                auto candidate_first = detail::strrchr(r_first, it, p_head);
                if (candidate_first == nullptr)
                    return -1;

                text_view candidate(candidate_first, p_len);
                if (compare_impl(candidate.begin(), candidate.end(), p_first, p_last) == 0)
                    return candidate_first - r_first;

                it = candidate_first;
            }
        }

    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto rfind (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
    { return detail::rfind_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr int rfind (text_view r, text_view p) noexcept
    { return detail::rfind_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto rfind (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::rfind_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto rfind (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<int, CharRange>
    { return detail::rfind_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // rfind_view ()

    namespace detail {

        inline constexpr text_view rfind_view_impl (
            char const * r_first, char const * r_last,
            char const * p_first, char const * p_last
        ) {
            if (r_first == r_last)
                return text_view(r_first, 0);

            if (p_first == p_last)
                return text_view(r_first, 0);

            int const n = rfind_impl(r_first, r_last, p_first, p_last);
            if (n < 0)
                return text_view(r_last, 0);
            return text_view(r_first + n, p_last - p_first);
        }

    }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto rfind_view (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
    { return detail::rfind_view_impl(&*begin(r), &*end(r), &*begin(p), &*end(p)); }

    inline constexpr text_view rfind_view (text_view r, text_view p) noexcept
    { return detail::rfind_view_impl(begin(r), end(r), begin(p), end(p)); }

    template <typename CharRange>
    constexpr auto rfind_view (text_view r, CharRange const & p) noexcept
        -> detail::rng_alg_ret_t<text_view, CharRange>
    { return detail::rfind_view_impl(begin(r), end(r), &*begin(p), &*end(p)); }

    template <typename CharRange>
    constexpr auto rfind_view (CharRange const & r, text_view p) noexcept
        -> detail::rng_alg_ret_t<text_view, CharRange>
    { return detail::rfind_view_impl(&*begin(r), &*end(r), begin(p), end(p)); }



    // substr ()

    template <typename CharRange>
    constexpr auto substr (CharRange const & r, int start, int size) noexcept
        -> detail::rng_alg_ret_t<text_view, CharRange>
    {
        if (size < 0) {
            assert(start <= r.size());
            size = r.size() - start;
        }
        assert(start + size <= r.size());
        return text_view(&*begin(r) + start, size);
    }

    inline constexpr auto substr (text_view r, int start, int size) noexcept
    {
        if (size < 0) {
            assert(start <= r.size());
            size = r.size() - start;
        }
        assert(start + size <= r.size());
        return text_view(begin(r) + start, size);
    }

} }

#endif
