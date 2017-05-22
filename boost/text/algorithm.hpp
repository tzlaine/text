#ifndef BOOST_TEXT_ALGORITHM_HPP
#define BOOST_TEXT_ALGORITHM_HPP

#include <boost/text/text_view.hpp>


namespace boost { namespace text {

    template <typename LCharRange, typename RCharRange>
    constexpr int compare (LCharRange const & l, RCharRange const & r) noexcept
    { return detail::compare_impl(&*begin(l), &*end(l), &*begin(r), &*end(r)); }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

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
            if (compare(candidate, p) == 0)
                return it - r_first;

            ++it;
        }
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view find_view (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return text_view(r_first, 0);

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return text_view(r_first, 0);

        int const n = find(r, p);
        if (n < 0)
            return text_view(r_last, 0);
        return text_view(r_first + n, p_last - p_first);
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_first_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_first;
        while (it != r_last) {
            for (char const c : p) {
                if (*it == c)
                    return it - r_first;
            }
            ++it;
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_last_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_last;
        while (r_first != it) {
            --it;
            for (char const c : p) {
                if (*it == c)
                    return it - r_first;
            }
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_first_not_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_first;
        while (it != r_last) {
            for (char const c : p) {
                if (*it != c)
                    return it - r_first;
            }
            ++it;
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_last_not_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_last;
        while (r_first != it) {
            --it;
            for (char const c : p) {
                if (*it != c)
                    return it - r_first;
            }
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int rfind (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

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
            if (compare(candidate, p) == 0)
                return candidate_first - r_first;

            it = candidate_first;
        }
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view rfind_view (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return text_view(r_first, 0);

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return text_view(r_first, 0);

        int const n = rfind(r, p);
        if (n < 0)
            return text_view(r_last, 0);
        return text_view(r_first + n, p_last - p_first);
    }

    template <typename CharRange>
    constexpr text_view substr (CharRange const & r, int start, int size) noexcept
    {
        assert(start + size <= r.size());
        return text_view(&*begin(r) + start, size);
    }

} }

#endif
