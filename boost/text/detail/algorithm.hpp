#ifndef BOOST_TEXT_DETAIL_ALGORITHM_HPP
#define BOOST_TEXT_DETAIL_ALGORITHM_HPP

#include <iterator>
#include <type_traits>

#include <cassert>


namespace boost { namespace text { namespace detail {

    template <typename ...>
    struct void_
    { using type = void; };

    template <typename ...T>
    using void_t = typename void_<T...>::type;

    template <typename T>
    struct fixup_ptr
    { using type = T; };

    template <typename T>
    using remove_v_t = typename std::remove_volatile<T>::type;

    template <typename T>
    struct fixup_ptr<T *>
    {
        using type = remove_v_t<T> const *;
    };

    template <typename T>
    using fixup_ptr_t = typename fixup_ptr<T>::type;

    struct nonesuch {};

    template <
        typename Default,
        typename AlwaysVoid,
        template <typename ...> class Template,
        typename ...Args
    >
    struct detector
    {
        using value_t = std::false_type;
        using type = Default;
    };

    template <
        typename Default,
        template <typename ...> class Template,
        typename ...Args
    >
    struct detector<Default, void_t<Template<Args...>>, Template, Args...> {
        using value_t = std::true_type;
        using type = Template<Args...>;
    };

    template <template<typename ...> class Template, typename ...Args>
    using is_detected = typename detector<nonesuch, void, Template, Args...>::value_t;

    template <template<typename ...> class Template, typename ...Args>
    using detected_t = typename detector<nonesuch, void, Template, Args...>::type;

    template <typename Default, template<typename ...> class Template, typename ...Args>
    using detected_or = typename detector<Default, void, Template, Args...>::type;



    template <typename T>
    using has_member_begin = decltype(&*std::declval<T>().begin());
    template <typename T>
    using has_free_unqualified_begin = decltype(&*begin(std::declval<T>()));
    template <typename T>
    using has_free_std_begin = decltype(&*std::begin(std::declval<T>()));

    template <typename T>
    using has_member_end = decltype(&*std::declval<T>().end());
    template <typename T>
    using has_free_unqualified_end = decltype(&*end(std::declval<T>()));
    template <typename T>
    using has_free_std_end = decltype(&*std::end(std::declval<T>()));

    template <typename T>
    using value_type_ = typename std::remove_cv<
        typename std::remove_reference<
            typename std::iterator_traits<T>::value_type
        >::type
    >::type;

    nonesuch callable_iterator_category (...);

    template <typename T>
    typename std::iterator_traits<typename T::iterator>::iterator_category callable_iterator_category (T &&);

    template <typename T>
    using iterator_category_ = decltype(callable_iterator_category(std::declval<T>()));



    template <typename T, typename Exclude1 = void, typename Exclude2 = void>
    using is_char_range = std::integral_constant<
        bool,
        std::is_same<
            fixup_ptr_t<
                detected_or<
                    detected_or<
                        detected_t<has_free_std_begin, T>,
                        has_free_unqualified_begin, T
                    >,
                    has_member_begin, T
                >
            >,
            char const *
        >::value &&
        std::is_same<
            fixup_ptr_t<
                detected_or<
                    detected_or<
                        detected_t<has_free_std_end, T>,
                        has_free_unqualified_end, T
                    >,
                    has_member_end, T
                >
            >,
            char const *
        >::value &&
        std::is_same<
            iterator_category_<T>,
            std::random_access_iterator_tag
        >::value &&
        !std::is_same<T, Exclude1>::value &&
        !std::is_same<T, Exclude2>::value
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



    template <typename T>
    using is_char_iter = std::integral_constant<
        bool,
        std::is_same<detected_t<value_type_, T>, char>::value
    >;

    template <
        typename T,
        typename R1,
        bool R1IsCharRange = is_char_iter<R1>{}
    >
    struct char_iter_ret {};

    template <typename T, typename R1>
    struct char_iter_ret<T, R1, true>
    { using type = T; };

    template <typename T, typename R1>
    using char_iter_ret_t = typename char_iter_ret<T, R1>::type;



    inline BOOST_TEXT_CXX14_CONSTEXPR
    char back_impl (char const * first, char const * last) noexcept
    {
        assert(first != last);
        return *(last - 1);
    }

} } }

#endif
