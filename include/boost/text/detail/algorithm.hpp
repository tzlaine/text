#ifndef BOOST_TEXT_DETAIL_ALGORITHM_HPP
#define BOOST_TEXT_DETAIL_ALGORITHM_HPP

#include <iterator>
#include <type_traits>

#include <cassert>


namespace boost { namespace text {

    struct unencoded_rope;
    struct unencoded_rope_view;

}}

namespace boost { namespace text { namespace detail {

    template<typename...>
    struct void_
    {
        using type = void;
    };

    template<typename... T>
    using void_t = typename void_<T...>::type;

    template<typename T>
    struct fixup_ptr
    {
        using type = T;
    };

    template<typename T>
    using remove_v_t = typename std::remove_volatile<T>::type;

    template<typename T>
    struct fixup_ptr<T *>
    {
        using type = remove_v_t<T> const *;
    };

    template<typename T>
    using fixup_ptr_t = typename fixup_ptr<T>::type;

    template<typename T>
    using remove_cv_ref_t =
        typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    struct nonesuch
    {};

    template<
        typename Default,
        typename AlwaysVoid,
        template<typename...> class Template,
        typename... Args>
    struct detector
    {
        using value_t = std::false_type;
        using type = Default;
    };

    template<
        typename Default,
        template<typename...> class Template,
        typename... Args>
    struct detector<Default, void_t<Template<Args...>>, Template, Args...>
    {
        using value_t = std::true_type;
        using type = Template<Args...>;
    };

    template<template<typename...> class Template, typename... Args>
    using is_detected =
        typename detector<nonesuch, void, Template, Args...>::value_t;

    template<template<typename...> class Template, typename... Args>
    using detected_t =
        typename detector<nonesuch, void, Template, Args...>::type;

    template<
        typename Default,
        template<typename...> class Template,
        typename... Args>
    using detected_or =
        typename detector<Default, void, Template, Args...>::type;



    template<typename T>
    using has_member_begin = decltype(*std::declval<T>().begin());
    template<typename T>
    using has_free_unqualified_begin = decltype(*begin(std::declval<T>()));
    template<typename T>
    using has_free_std_begin = decltype(*std::begin(std::declval<T>()));

    template<typename T>
    using has_member_end = decltype(*std::declval<T>().end());
    template<typename T>
    using has_free_unqualified_end = decltype(*end(std::declval<T>()));
    template<typename T>
    using has_free_std_end = decltype(*std::end(std::declval<T>()));

    template<typename T>
    using value_type_ = typename std::remove_cv<
        typename std::remove_reference<typename T::value_type>::type>::type;


    template<typename T>
    using nonpointer_iterator_category_ =
        typename T::iterator::iterator_category;

    template<typename T>
    using iterator_category_ = typename std::conditional<
        std::is_pointer<typename T::iterator>::value,
        std::random_access_iterator_tag,
        detected_t<nonpointer_iterator_category_, T>>::type;



    template<typename T>
    using is_char_range = std::integral_constant<
        bool,
        std::is_same<remove_cv_ref_t<T>, unencoded_rope_view>::value || // TODO
        std::is_same<remove_cv_ref_t<T>, unencoded_rope>::value || // TODO
            (std::is_same<
                remove_cv_ref_t<detected_or<
                    detected_or<
                        detected_t<has_free_std_begin, T>,
                        has_free_unqualified_begin,
                        T>,
                    has_member_begin,
                    T>>,
                char>::value &&
                 std::is_same<
                     remove_cv_ref_t<detected_or<
                         detected_or<
                             detected_t<has_free_std_end, T>,
                             has_free_unqualified_end,
                             T>,
                         has_member_end,
                         T>>,
                     char>::value &&
                     std::is_same<
                         detected_t<iterator_category_, T>,
                         std::random_access_iterator_tag>::value)>;



    template<
        typename T,
        typename R1,
        typename Exclude1,
        typename Exclude2,
        bool R1IsCharRange = is_char_range<R1>::value &&
                             !std::is_same<R1, Exclude1>::value &&
                             !std::is_same<R1, Exclude2>::value>
    struct rng_alg_ret
    {
    };

    template<typename T, typename R1, typename Exclude1, typename Exclude2>
    struct rng_alg_ret<T, R1, Exclude1, Exclude2, true>
    {
        using type = T;
    };

    template<
        typename T,
        typename R1,
        typename Exclude1 = void,
        typename Exclude2 = void>
    using rng_alg_ret_t = typename rng_alg_ret<T, R1, Exclude1, Exclude2>::type;

    template<
        typename T,
        typename R1,
        typename R2,
        bool R1IsCharRange = is_char_range<R1>::value,
        bool R2IsCharRange = is_char_range<R2>::value>
    struct rngs_alg_ret
    {
    };

    template<typename T, typename R1, typename R2>
    struct rngs_alg_ret<T, R1, R2, true, true>
    {
        using type = T;
    };

    template<typename T, typename R1, typename R2>
    using rngs_alg_ret_t = typename rngs_alg_ret<T, R1, R2>::type;



    template<typename T>
    using has_contig_member_begin = decltype(&*std::declval<T>().begin());
    template<typename T>
    using has_contig_free_unqualified_begin =
        decltype(&*begin(std::declval<T>()));
    template<typename T>
    using has_contig_free_std_begin = decltype(&*std::begin(std::declval<T>()));

    template<typename T>
    using has_contig_member_end = decltype(&*std::declval<T>().end());
    template<typename T>
    using has_contig_free_unqualified_end = decltype(&*end(std::declval<T>()));
    template<typename T>
    using has_contig_free_std_end = decltype(&*std::end(std::declval<T>()));

    template<typename T>
    using is_contig_char_range = std::integral_constant<
        bool,
        std::is_same<
            fixup_ptr_t<detected_or<
                detected_or<
                    detected_t<has_contig_free_std_begin, T>,
                    has_contig_free_unqualified_begin,
                    T>,
                has_contig_member_begin,
                T>>,
            char const *>::value &&
            std::is_same<
                fixup_ptr_t<detected_or<
                    detected_or<
                        detected_t<has_contig_free_std_end, T>,
                        has_contig_free_unqualified_end,
                        T>,
                    has_contig_member_end,
                    T>>,
                char const *>::value &&
            std::is_same<
                iterator_category_<T>,
                std::random_access_iterator_tag>::value &&
            !std::is_same<T, unencoded_rope>::value &&
            !std::is_same<T, unencoded_rope_view>::value>;



    template<
        typename T,
        typename R1,
        bool R1IsContigCharRange = is_contig_char_range<R1>::value>
    struct contig_rng_alg_ret
    {
    };

    template<typename T, typename R1>
    struct contig_rng_alg_ret<T, R1, true>
    {
        using type = T;
    };

    template<typename T, typename R1>
    using contig_rng_alg_ret_t = typename contig_rng_alg_ret<T, R1>::type;

    template<
        typename T,
        typename R1,
        typename R2,
        bool R1IsContigCharRange = is_contig_char_range<R1>::value,
        bool R2IsContigCharRange = is_contig_char_range<R2>::value>
    struct contig_rngs_alg_ret
    {
    };

    template<typename T, typename R1, typename R2>
    struct contig_rngs_alg_ret<T, R1, R2, true, true>
    {
        using type = T;
    };

    template<typename T, typename R1, typename R2>
    using contig_rngs_alg_ret_t = typename contig_rngs_alg_ret<T, R1, R2>::type;



    template<typename T>
    using is_char_iter = std::integral_constant<
        bool,
        std::is_same<char const *, T>::value ||
            std::is_same<char *, T>::value ||
            std::is_same<detected_t<value_type_, T>, char>::value>;

    template<
        typename T,
        typename R1,
        bool R1IsCharRange = is_char_iter<R1>::value>
    struct char_iter_ret
    {
    };

    template<typename T, typename R1>
    struct char_iter_ret<T, R1, true>
    {
        using type = T;
    };

    template<typename T, typename R1>
    using char_iter_ret_t = typename char_iter_ret<T, R1>::type;



    template<typename T>
    using is_code_point = std::integral_constant<
        bool,
        (std::is_unsigned<T>::value && std::is_integral<T>::value &&
         sizeof(T) == 4)>;

    template<typename T>
    using is_cp_iter = std::integral_constant<
        bool,
        ((std::is_pointer<T>::value &&
          is_code_point<typename std::remove_cv<
              typename std::remove_pointer<T>::type>::type>::value) ||
         (is_code_point<typename std::remove_cv<
              detected_t<value_type_, T>>::type>::value))>;

    template<typename T, typename R1, bool R1IsCPRange = is_cp_iter<R1>::value>
    struct cp_iter_ret
    {
    };

    template<typename T, typename R1>
    struct cp_iter_ret<T, R1, true>
    {
        using type = T;
    };

    template<typename T, typename R1>
    using cp_iter_ret_t = typename cp_iter_ret<T, R1>::type;



    template<typename T>
    using is_grapheme_char_range = std::integral_constant<
        bool,
        is_char_iter<remove_cv_ref_t<decltype(
            std::declval<const T>().begin().base().base())>>::value &&
            is_char_iter<remove_cv_ref_t<decltype(
                std::declval<const T>().end().base().base())>>::value>;

    template<
        typename T,
        typename R1,
        bool R1IsGraphemeCharRange = is_grapheme_char_range<R1>::value>
    struct graph_rng_alg_ret
    {
    };

    template<typename T, typename R1>
    struct graph_rng_alg_ret<T, R1, true>
    {
        using type = T;
    };

    template<typename T, typename R1>
    using graph_rng_alg_ret_t = typename graph_rng_alg_ret<T, R1>::type;



    template<typename T>
    using is_contig_grapheme_char_range = std::integral_constant<
        bool,
        (std::is_same<
             decltype(std::declval<const T>().begin().base().base()),
             char const *>::value ||
         std::is_same<
             decltype(std::declval<const T>().begin().base().base()),
             char *>::value) &&
            (std::is_same<
                 decltype(std::declval<const T>().end().base().base()),
                 char const *>::value ||
             std::is_same<
                 decltype(std::declval<const T>().end().base().base()),
                 char *>::value)>;

    template<
        typename T,
        typename R1,
        bool R1IsContigGraphemeCharRange =
            is_contig_grapheme_char_range<R1>::value>
    struct contig_graph_rng_alg_ret
    {
    };

    template<typename T, typename R1>
    struct contig_graph_rng_alg_ret<T, R1, true>
    {
        using type = T;
    };

    template<typename T, typename R1>
    using contig_graph_rng_alg_ret_t =
        typename contig_graph_rng_alg_ret<T, R1>::type;



    inline BOOST_TEXT_CXX14_CONSTEXPR char
    back_impl(char const * first, char const * last) noexcept
    {
        assert(first != last);
        return *(last - 1);
    }

}}}

#endif
