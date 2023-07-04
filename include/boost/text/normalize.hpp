// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/stream_safe.hpp>
#include <boost/text/detail/normalize.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>


namespace boost { namespace text {

    namespace detail {

        template<typename CPIter, typename Sentinel>
        using utf8_range_expr = is_char_iter<decltype(
            text::unpack_iterator_and_sentinel(
                std::declval<CPIter>(), std::declval<Sentinel>())
                .f_)>;

        template<typename CPIter, typename Sentinel>
        using utf8_fast_path =
            detected_or_t<std::false_type, utf8_range_expr, CPIter, Sentinel>;

        template<typename OutIter>
        struct norm_result
        {
            OutIter out_;
            bool normalized_;
        };

        template<
            nf Normalization,
            typename OutIter, // will be bool for the check-only case
            typename CPIter,
            typename Sentinel,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value,
            bool Composition =
                Normalization != nf::d && Normalization != nf::kd>
        struct norm_impl
        {
            // Primary template does decomposition.
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first_, Sentinel last_, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = std::ranges::subrange(first_, last_) | as_utf16;
                auto first = r.begin();
                auto const last = r.end();

                int const chunk_size = 512;
                std::array<uint16_t, chunk_size> input;
                auto input_first = input.data();

                while (first != last) {
                    int n = 0;
                    auto input_last = input_first;
                    for (; first != last && n < chunk_size - 1;
                         ++first, ++input_last, ++n) {
                        *input_last = *first;
                    }
                    if (high_surrogate(*std::ranges::prev(input_last)) &&
                        first != last) {
                        *input_last++ = *first;
                        ++first;
                    }
                    auto const & table = Normalization == nf::kd
                                             ? detail::nfkc_table()
                                             : detail::nfc_table();
                    detail::reordering_appender<Appender> buffer(
                        table, appender);
                    auto const input_new_first = detail::decompose<do_writes>(
                        table, input.data(), input_last, buffer);
                    if (!do_writes && input_new_first != input_last)
                        return norm_result<OutIter>{appender.out(), false};
                    input_first =
                        std::copy(input_new_first, input_last, input.data());
                }

                return norm_result<OutIter>{appender.out(), true};
            }
        };


        template<
            nf Normalization,
            typename OutIter,
            typename CPIter,
            typename Sentinel,
            bool UTF8>
        struct norm_impl<Normalization, OutIter, CPIter, Sentinel, UTF8, true>
        {
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first, Sentinel last, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = std::ranges::subrange(first, last) | as_utf16;
                auto const & table = Normalization == nf::kc
                                         ? detail::nfkc_table()
                                         : detail::nfc_table();
                detail::reordering_appender<Appender> reorder_buffer(
                    table, appender);
                auto const normalized =
                    detail::compose<Normalization == nf::fcc, do_writes>(
                        table, r.begin(), r.end(), reorder_buffer);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };

        template<
            nf Normalization,
            typename OutIter,
            typename CPIter,
            typename Sentinel>
        struct norm_impl<Normalization, OutIter, CPIter, Sentinel, true, true>
        {
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first, Sentinel last, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = std::ranges::subrange(first, last) | as_utf8;
                auto const & table = Normalization == nf::kc
                                         ? detail::nfkc_table()
                                         : detail::nfc_table();
                auto const normalized =
                    detail::compose_utf8<Normalization == nf::fcc, do_writes>(
                        table, r.begin(), r.end(), appender);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value &&
                            Normalization != nf::d && Normalization != nf::kd>
        struct normalization_appender
        {
            using type = utf16_to_utf32_appender<OutIter>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter>
        struct normalization_appender<
            Normalization,
            CPIter,
            Sentinel,
            OutIter,
            true>
        {
            using type = utf8_to_utf32_appender<OutIter>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter>
        using normalization_appender_t = typename normalization_appender<
            Normalization,
            CPIter,
            Sentinel,
            OutIter>::type;
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Writes sequence `[first, last)` in Unicode normalization form
        `Normalization` to `out`.

        \see https://unicode.org/notes/tn5 */
    template<
        nf Normalization,
        code_point_iter I,
        std::sentinel_for<I> S,
        std::output_iterator<uint32_t> O>
    O normalize(I first, S last, O out);

    /** Writes sequence `r` in Unicode normalization form `Normalization` to
        `out`.

        \see https://unicode.org/notes/tn5 */
    template<
        nf Normalization,
        code_point_range R,
        std::output_iterator<uint32_t> O>
    O normalize(R && r, O out);

    /** Returns true iff the given sequence of code points is in Unicode
        normalization form `Normalization`.

        \see https://unicode.org/notes/tn5 */
    template<nf Normalization, code_point_iter I, std::sentinel_for<I> S>
    bool is_normalized(I first, S last);

    /** Returns true iff the given sequence of code points is in Unicode
        normalization form `Normalization`. */
    template<nf Normalization, code_point_range R>
    bool is_normalized(R && r);

#else

    template<
        nf Normalization,
        typename CPIter,
        typename Sentinel,
        typename OutIter>
    auto normalize(CPIter first, Sentinel last, OutIter out)
        ->detail::cp_iter_ret_t<OutIter, CPIter>
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        detail::
            normalization_appender_t<Normalization, CPIter, Sentinel, OutIter>
                appender(out);
        return detail::norm_impl<Normalization, OutIter, CPIter, Sentinel>::
            call(first, last, appender)
                .out_;
    }

    template<nf Normalization, typename CPRange, typename OutIter>
    OutIter normalize(CPRange && r, OutIter out)
    {
        return v1::normalize<Normalization>(
            detail::begin(r), detail::end(r), out);
    }

    template<nf Normalization, typename CPIter, typename Sentinel>
    auto is_normalized(CPIter first, Sentinel last)
        ->detail::cp_iter_ret_t<bool, CPIter>
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        detail::null_appender appender;
        return detail::norm_impl<Normalization, bool, CPIter, Sentinel>::call(
                   first, last, appender)
            .normalized_;
    }

    template<nf Normalization, typename CPRange>
    bool is_normalized(CPRange && r)
    {
        return v1::is_normalized<Normalization>(
            detail::begin(r), detail::end(r));
    }

#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<
        nf Normalization,
        utf_iter I,
        std::sentinel_for<I> S,
        std::output_iterator<uint32_t> O>
    O normalize(I first, S last, O out)
    {
        if constexpr (utf32_iter<I>) {
            BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
            detail::normalization_appender_t<Normalization, I, S, O> appender(
                out);
            return detail::norm_impl<Normalization, O, I, S>::call(
                       first, last, appender)
                .out_;
        } else {
            auto const code_points =
                std::ranges::subrange(first, last) | as_utf32;
            return boost::text::normalize(
                code_points.begin(), code_points.end(), out);
        }
    }

    template<
        nf Normalization,
        utf_range_like R,
        std::output_iterator<uint32_t> O>
    O normalize(R && r, O out)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
            return boost::text::normalize<Normalization>(r, null_sentinel, out);
        } else {
            return boost::text::normalize<Normalization>(
                std::ranges::begin(r), std::ranges::end(r), out);
        }
    }

    template<nf N, utf_iter I, std::sentinel_for<I> S>
    bool is_normalized(I first, S last)
    {
        if constexpr (utf32_iter<I>) {
            BOOST_TEXT_STATIC_ASSERT_N();
            detail::null_appender appender;
            return detail::norm_impl<N, bool, I, S>::call(first, last, appender)
                .normalized_;
        } else {
            auto const code_points =
                std::ranges::subrange(first, last) | as_utf32;
            return boost::text::is_normalized<N>(
                code_points.begin(), code_points.end());
        }
    }

    template<nf N, utf_range_like R>
    bool is_normalized(R && r)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
            return boost::text::is_normalized<N>(r, null_sentinel);
        } else {
            return boost::text::is_normalized<N>(
                std::ranges::begin(r), std::ranges::end(r));
        }
    }

    template<nf N, utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class normalize_view : public std::ranges::view_interface<normalize_view<N, V>>
    {
        template<bool Const, bool StoreLast = !detail::is_utf_iter<std::ranges::iterator_t<V>>>
        class iterator;
        class sentinel;

        static constexpr bool bidi =
            std::derived_from<detail::uc_view_category_t<V>, std::bidirectional_iterator_tag>;

        V base_ = V();

        static_assert(nf::c <= N && N <= nf::fcc);

    public:
        static constexpr nf normalization_form = N;

        constexpr normalize_view() requires std::default_initializable<V> = default;
        constexpr normalize_view(V base) : base_{std::move(base)} {}

        constexpr V base() const & requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr iterator<false> begin() { return iterator<false>{base_}; }
        constexpr iterator<true> begin() const requires utf32_range<const V> { return iterator<true>{base_}; }

        constexpr sentinel end() { return sentinel{}; }
        constexpr iterator<false> end() requires bidi { return iterator<false>{base_, std::ranges::end(base_)}; }
        constexpr sentinel end() const requires utf32_range<const V> { return sentinel{}; }
        constexpr iterator<true> end() const requires utf32_range<const V> && bidi { return iterator<true>{base_, std::ranges::end(base_)}; }
    };

    template<nf N, utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    template<bool Const, bool StoreLast>
    class normalize_view<N, V>::iterator
        : detail::first_last_storage<detail::maybe_const<Const, V>>,
          public boost::stl_interfaces::iterator_interface<
              iterator<Const, StoreLast>,
              detail::uc_view_category_t<V>,
              char32_t,
              char32_t>
    {
        using Base = detail::maybe_const<Const, V>;
        using storage_base = detail::first_last_storage<Base>;

        static constexpr bool bidi =
            std::derived_from<detail::uc_view_category_t<V>, std::bidirectional_iterator_tag>;

        std::ranges::iterator_t<Base> it_;
        std::ranges::iterator_t<Base> chunk_last_;
        // exposition-only, and only defined when std::ranges::iterator_t<Base>
        // is not a specialization of utf_iterator.
        // std::ranges::iterator_t<Base> first_;
        // std::ranges::sentinel_t<Base> last_;

        boost::container::static_vector<char32_t, 32> buf_;
        int index_ = 0;

        constexpr void read_chunk_and_normalize(bool reverse) {
            boost::container::static_vector<char32_t, 32> temp_buf_;
            auto search_it = it_;
            if (search_it != storage_base::end(it_))
                ++search_it;
            auto last = reverse ?
                chunk_last_ : detail::first_stable_cp<N>(search_it, storage_base::end(it_));
            chunk_last_ = std::ranges::copy(it_, last, std::back_inserter(temp_buf_)).in;
            buf_.clear();
            text::normalize<N>(temp_buf_, std::back_inserter(buf_));
        }

        friend class sentinel;

    public:
        constexpr iterator() requires std::default_initializable<std::ranges::iterator_t<Base>> = default;
        constexpr iterator(Base & base) : storage_base(base), it_(std::ranges::begin(base)) {
            read_chunk_and_normalize(false);
        }
        constexpr iterator(Base & base, std::ranges::iterator_t<V> it) requires bidi
            : storage_base(base), it_(it), chunk_last_(it) {}
        constexpr iterator(iterator<!Const, StoreLast> i)
            requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
            : storage_base(i), it_(i.it_), chunk_last_(i.chunk_last_), buf_(i.buf_), index_(i.index_) {}

        constexpr const std::ranges::iterator_t<Base> & base() const & noexcept { return it_; }
        constexpr std::ranges::iterator_t<Base> base() && { return std::move(it_); }

        constexpr char32_t operator*() const { return buf_[index_]; }

        constexpr iterator & operator++() {
            auto const last = storage_base::end(it_);
            if (it_ == last && index_ == (int)buf_.size())
                return *this;
            ++index_;
            if ((int)buf_.size() == index_ && it_ != last) {
                it_ = chunk_last_;
                read_chunk_and_normalize(false);
                index_ = 0;
            }
            return *this;
        }

        constexpr iterator & operator--() requires bidi {
            auto const first = storage_base::begin(it_);
            if (it_ == first && !index_)
                return *this;
            if (!index_) {
                chunk_last_ = it_;
                it_ = detail::last_stable_cp<N>(first, it_);
                read_chunk_and_normalize(true);
                index_ = buf_.size();
            }
            --index_;
            return *this;
        }

        friend bool operator==(iterator lhs, iterator rhs)
        {
            return lhs.it_ == rhs.it_ && (lhs.index_ == rhs.index_ ||
                                          (lhs.index_ == (int)lhs.buf_.size() &&
                                           rhs.index_ == (int)rhs.buf_.size()));
        }

        using base_type = boost::stl_interfaces::iterator_interface<
            iterator<Const, StoreLast>,
            detail::uc_view_category_t<V>,
            char32_t,
            char32_t>;
        using base_type::operator++;
        using base_type::operator--;
    };

    template<nf N, utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class normalize_view<N, V>::sentinel
    {
    public:
        template<bool Const, bool StoreLast>
        friend constexpr bool operator==(const iterator<Const, StoreLast> & it, sentinel) {
            if (it.index_ < (int)it.buf_.size())
                return false;
            if constexpr (StoreLast) {
                return it.it_ == it.last_;
            } else {
                return it.base().base() == it.base().end();
            }
        }
    };


    template<utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class nfc_view : public normalize_view<nf::c, V>
    {
    public:
        constexpr nfc_view() requires std::default_initializable<V> = default;
        constexpr nfc_view(V base) :
            normalize_view<nf::c, V>{std::move(base)}
        {}
    };
    template<utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class nfkc_view : public normalize_view<nf::kc, V>
    {
    public:
        constexpr nfkc_view() requires std::default_initializable<V> = default;
        constexpr nfkc_view(V base) :
            normalize_view<nf::kc, V>{std::move(base)}
        {}
    };
    template<utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class nfd_view : public normalize_view<nf::d, V>
    {
    public:
        constexpr nfd_view() requires std::default_initializable<V> = default;
        constexpr nfd_view(V base) :
            normalize_view<nf::d, V>{std::move(base)}
        {}
    };
    template<utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class nfkd_view : public normalize_view<nf::kd, V>
    {
    public:
        constexpr nfkd_view() requires std::default_initializable<V> = default;
        constexpr nfkd_view(V base) :
            normalize_view<nf::kd, V>{std::move(base)}
        {}
    };
    template<utf32_range V>
        requires std::ranges::view<V> && std::ranges::forward_range<V>
    class fcc_view : public normalize_view<nf::fcc, V>
    {
    public:
        constexpr fcc_view() requires std::default_initializable<V> = default;
        constexpr fcc_view(V base) :
            normalize_view<nf::fcc, V>{std::move(base)}
        {}
    };

    template<class R>
    nfc_view(R &&) -> nfc_view<std::views::all_t<R>>;
    template<class R>
    nfkc_view(R &&) -> nfkc_view<std::views::all_t<R>>;
    template<class R>
    nfd_view(R &&) -> nfd_view<std::views::all_t<R>>;
    template<class R>
    nfkd_view(R &&) -> nfkd_view<std::views::all_t<R>>;
    template<class R>
    fcc_view(R &&) -> fcc_view<std::views::all_t<R>>;


    namespace dtl {
        template<class T>
        constexpr bool is_normalize_view = false;
        template<nf N, class T>
        constexpr bool is_normalize_view<normalize_view<N, T>> = true;
        template<class T>
        constexpr bool is_normalize_view<nfc_view<T>> = true;
        template<class T>
        constexpr bool is_normalize_view<nfkc_view<T>> = true;
        template<class T>
        constexpr bool is_normalize_view<nfd_view<T>> = true;
        template<class T>
        constexpr bool is_normalize_view<nfkd_view<T>> = true;
        template<class T>
        constexpr bool is_normalize_view<fcc_view<T>> = true;

        template<template<class> class View, nf N>
        struct normalize_impl : range_adaptor_closure<normalize_impl<View, N>>
        {
            template<can_stream_safe R>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
                    return std::ranges::empty_view<T>{};
                } else if constexpr (is_normalize_view<T>) {
                    if constexpr (N == T::normalization_form) {
                        return std::forward<R>(r);
                    } else if constexpr (requires { View(std::forward<R>(r).base()); }) {
                        return View(std::forward<R>(r).base());
                    } else {
                        return View(std::forward<R>(r));
                    }
                } else {
                    return View(std::forward<R>(r) | as_stream_safe);
                }
            }
        };
    }

    inline constexpr dtl::normalize_impl<nfc_view, nf::c> as_nfc;
    inline constexpr dtl::normalize_impl<nfkc_view, nf::kc> as_nfkc;
    inline constexpr dtl::normalize_impl<nfd_view, nf::d> as_nfd;
    inline constexpr dtl::normalize_impl<nfkd_view, nf::kd> as_nfkd;
    inline constexpr dtl::normalize_impl<fcc_view, nf::fcc> as_fcc;

}}}

namespace std::ranges {
    template<boost::text::nf N, class V>
    inline constexpr bool enable_borrowed_range<boost::text::normalize_view<N, V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::nfc_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::nfkc_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::nfd_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::nfkd_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::fcc_view<V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
