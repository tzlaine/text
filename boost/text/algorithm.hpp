#ifndef BOOST_TEXT_ALGORITHM_HPP
#define BOOST_TEXT_ALGORITHM_HPP

#include <boost/text/text_view.hpp>


namespace boost { namespace text {

    namespace detail {

        template <class ...>
        using void_t = void;

        template <typename T>
        struct remove_cv_ref : std::remove_cv<std::remove_reference_t<T>>
        {};

        template <typename T>
        using remove_cv_ref_t = typename remove_cv_ref<T>::type;

        struct nonesuch {};

        template <
            typename Default,
            typename AlwaysVoid,
            template <class ...> typename Template,
            typename ...Args
        >
        struct detector
        {
            using value_t = std::false_type;
            using type = Default;
        };
 
        template <
            typename Default,
            template <class ...> typename Template,
            typename ...Args
        >
        struct detector<Default, std::void_t<Template<Args...>>, Template, Args...> {
            using value_t = std::true_type;
            using type = Template<Args...>;
        };
 
        template <template<class ...> class Template, class ...Args>
        using is_detected = typename detector<nonesuch, void, Template, Args...>::value_t;

        template <template<class ...> class Template, class ...Args>
        using detected_t = typename detector<nonesuch, void, Template, Args...>::type;

        template <typename Default, template<class ...> class Template, class ...Args>
        using detected_or = typename detector<Default, void, Template, Args...>::type;

        template <typename T>
        using has_member_begin = decltype(*std::declval<T>().begin());
        template <typename T>
        using has_free_unqualified_begin = decltype(*begin(std::declval<T>()));
        template <typename T>
        using has_free_std_begin = decltype(*std::begin(std::declval<T>()));

        template <typename T>
        using has_member_end = decltype(*std::declval<T>().end());
        template <typename T>
        using has_free_unqualified_end = decltype(*end(std::declval<T>()));
        template <typename T>
        using has_free_std_end = decltype(*std::end(std::declval<T>()));

        template <typename T>
        using is_char_range = std::integral_constant<
            bool,
            std::is_same<
                remove_cv_ref_t<
                    detected_or<
                        detected_or<
                            detected_t<has_free_std_begin, T>,
                            has_free_unqualified_begin, T
                        >,
                        has_member_begin, T
                    >
                >,
                char
            >::value &&
            std::is_same<
                remove_cv_ref_t<
                    detected_or<
                        detected_or<
                            detected_t<has_free_std_end, T>,
                            has_free_unqualified_end, T
                        >,
                        has_member_end, T
                    >
                >,
                char
            >::value
        >;

        template <
            typename T,
            typename R1,
            bool R1IsCharRange = is_char_range<R1>{}
        >
        struct rng_alg_ret {};

        template <typename T, typename R1>
        struct rng_alg_ret<T, R1, true>
        { using type = T; };

        template <typename T, typename R1>
        using rng_alg_ret_t = typename rng_alg_ret<T, R1>::type;

        template <
            typename T,
            typename R1,
            typename R2,
            bool R1IsCharRange = is_char_range<R1>{},
            bool R2IsCharRange = is_char_range<R2>{}
        >
        struct rngs_alg_ret {};

        template <typename T, typename R1, typename R2>
        struct rngs_alg_ret<T, R1, R2, true, true>
        { using type = T; };

        template <typename T, typename R1, typename R2>
        using rngs_alg_ret_t = typename rngs_alg_ret<T, R1, R2>::type;

    }

    template <typename LCharRange, typename RCharRange>
    constexpr auto compare (LCharRange const & l, RCharRange const & r) noexcept
        -> detail::rngs_alg_ret_t<int, LCharRange, RCharRange>
    { return detail::compare_impl(&*begin(l), &*end(l), &*begin(r), &*end(r)); }

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_view (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_first_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_last_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_first_not_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto find_last_not_of (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto rfind (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<int, CharRange, PatternCharRange>
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

    template <typename CharRange, typename PatternCharRange>
    constexpr auto rfind_view (CharRange const & r, PatternCharRange const & p) noexcept
        -> detail::rngs_alg_ret_t<text_view, CharRange, PatternCharRange>
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

} }

#endif
