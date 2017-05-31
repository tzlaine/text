#ifndef BOOST_TEXT_DETAIL_ALGORITHM_HPP
#define BOOST_TEXT_DETAIL_ALGORITHM_HPP

#include <iterator>
#include <type_traits>

#include <cassert>


namespace boost { namespace text { namespace detail {

    template <class ...>
    using void_t = void;

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

    template <template<class ...> class Template, class ...Args>
    using is_detected = typename detector<nonesuch, void, Template, Args...>::value_t;

    template <template<class ...> class Template, class ...Args>
    using detected_t = typename detector<nonesuch, void, Template, Args...>::type;

    template <typename Default, template<class ...> class Template, class ...Args>
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
    using iterator_category_ =
        typename std::iterator_traits<typename T::iterator>::iterator_category;

    template <typename T>
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
            detected_t<iterator_category_, T>,
            std::random_access_iterator_tag
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

    constexpr char back_impl (char const * first, char const * last) noexcept
    {
        assert(first != last);
        return *(last - 1);
    }

} } }

#endif
