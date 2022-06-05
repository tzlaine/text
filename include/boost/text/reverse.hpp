// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_REVERSE_HPP
#define BOOST_TEXT_REVERSE_HPP

#include <boost/text/view_adaptor.hpp>


namespace boost { namespace text {

    namespace detail {
        template<bool CommonRange>
        struct set_rev_rng_first
        {
            template<typename V>
            static auto call(V const & v)
            {
                return stl_interfaces::make_reverse_iterator(v.end());
            }
        };

        template<>
        struct set_rev_rng_first<false>
        {
            template<typename V>
            static auto call(V const & v)
            {
                auto v_f = v.begin();
                auto const v_l = v.end();
                while (v_f != v_l) {
                    ++v_f;
                }
                return stl_interfaces::make_reverse_iterator(v_f);
            }
        };

#if BOOST_TEXT_USE_CONCEPTS
        template<typename View>
        requires std::is_object_v<View>
#else
        template<
            typename View,
            typename Enable = std::enable_if_t<std::is_object<View>::value>>
#endif
        struct reverse_view : stl_interfaces::view_interface<reverse_view<View>>
        {
            using v_iter = iterator_t<View>;
            using v_sent = sentinel_t<View>;

            static_assert(
                std::is_base_of<
                    std::bidirectional_iterator_tag,
                    typename std::iterator_traits<v_iter>::iterator_category>::
                    value,
                "A reversed view must have bidirectional iterators.");

            using iterator = stl_interfaces::reverse_iterator<v_iter>;

            constexpr reverse_view() = default;

#if BOOST_TEXT_USE_CONCEPTS
            template<typename V>
            requires std::is_same_v<std::remove_cvref_t<V>, View>
#else
            template<
                typename V,
                typename E = std::enable_if_t<
                    std::is_same<std::remove_reference_t<V>, View>::value>>
#endif
            constexpr reverse_view(int, V && v) : v_{(V &&) v}
            {
                first_ = set_rev_rng_first<
                    std::is_same<v_iter, v_sent>::value>::call(v_);
            }

            constexpr iterator begin() const { return first_; }
            constexpr iterator end() const
            {
                return stl_interfaces::make_reverse_iterator(v_.begin());
            }

            constexpr View base() const { return v_; }

        private:
            View v_ = View();
            iterator first_;
        };

        template<typename T>
        struct is_reverse_view : std::false_type
        {};
        template<typename T>
        struct is_reverse_view<reverse_view<T>> : std::true_type
        {};

        template<
            typename R,
            bool ReverseView =
                is_reverse_view<std::remove_reference_t<R>>::value>
        struct reverse_impl_impl
        {
            static constexpr auto call(R && r) { return ((R &&) r).base(); }
        };
        template<typename R>
        struct reverse_impl_impl<R, false>
        {
            static constexpr auto call(R && r)
            {
                return reverse_view<std::remove_reference_t<R>>(0, (R &&) r);
            }
        };

        struct reverse_impl : range_adaptor_closure<reverse_impl>
        {
            template<typename R>
            constexpr auto operator()(R && r) const
            {
                return reverse_impl_impl<R>::call((R &&) r);
            }
        };
    }

#if defined(BOOST_TEXT_DOXYGEN) || defined(__cpp_inline_variables)
    /** A simplified version of the `std::views::reverse` range adaptor for
        pre-C++20 builds.  Prefer `std::views::reverse` if you have it. */
    inline constexpr detail::reverse_impl reverse;
#else
    namespace {
        constexpr detail::reverse_impl reverse;
    }
#endif

}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<typename View>
    inline constexpr bool
        enable_borrowed_range<boost::text::detail::reverse_view<View>> = true;
}

#endif

#endif
