// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_STREAM_SAFE_HPP
#define BOOST_TEXT_STREAM_SAFE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/dangling.hpp>
#include <boost/text/in_out_result.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/view_adaptor.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    /** An alias for `in_out_result` returned by algorithms that perform a
        copy. */
    template<typename I, typename O>
    using copy_result = in_out_result<I, O>;

    namespace detail {
        constexpr int stream_safe_max_nonstarters = 20;

        template<typename I, typename S>
        I next_stream_safe_cp(I first, S last, int & nonstarters)
        {
            for (; first != last; ++first) {
                auto const cp = *first;
                if (detail::ccc(cp) == 0)
                    nonstarters = 0;
                else
                    ++nonstarters;
                if (nonstarters <= stream_safe_max_nonstarters)
                    break;
            }
            return first;
        }

        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            unpack_result<format::utf8, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_8_out(unpacked.first);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            unpack_result<format::utf16, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_16_out(unpacked.first);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            unpack_result<format::utf32, Iter, Sentinel, Repack> unpacked)
        {
            return unpacked.first;
        }

        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            unpack_result<format::utf8, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.first, out.base(), unpacked.last);
        }
        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            unpack_result<format::utf16, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.first, out.base(), unpacked.last);
        }
        template<typename OutIter>
        auto stream_safe_result(detail::tag_t<format::utf32>, OutIter out)
        {
            return out;
        }

        template<typename I, typename S, typename O>
        copy_result<I, O> stream_safe_copy_impl(I first, S last, O out)
        {
            if (first == last)
                return {first, out};
            int nonstarters = detail::ccc(*first) ? 1 : 0;
            do {
                *out = *first;
                ++out;
                ++first;
                first = detail::next_stream_safe_cp(first, last, nonstarters);
            } while (first != last);
            return {first, out};
        }

        template<typename I, typename S>
        auto stream_safe_impl(I first, S last)
        {
            auto const unpacked =
                boost::text::unpack_iterator_and_sentinel(first, last);
            auto out = detail::stream_safe_out(unpacked);
            out = detail::stream_safe_copy_impl(first, last, out).out;
            return detail::stream_safe_result(unpacked, out);
        }

        template<typename I, typename S>
        bool is_stream_safe_impl(I first, S last)
        {
            int nonstarters = 0;
            for (; first != last; ++first) {
                auto const cp = *first;
                if (detail::ccc(cp) == 0)
                    nonstarters = 0;
                else
                    ++nonstarters;
                if (stream_safe_max_nonstarters <= nonstarters)
                    return false;
            }
            return true;
        }
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Writes sequence `[first, last)` to `out`, ensuring Stream-Safe Text
        Format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires std::indirectly_copyable<I, O>
    constexpr copy_result<I, O> stream_safe_copy(I first, S last, O out);

    /** Writes sequence `r` to `out`, ensuring Stream-Safe Text Format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<detail::iterator_t<R>, O>
    constexpr copy_result<std::ranges::borrowed_iterator_t<R>, O>
    stream_safe_copy(R && r, O out);

    /** Copies the stream-safe portion of `[first, last)` to the beginning of
        `[first, last)`, and returns an iterator to the end of the copied
        range.  Note that the type returned may not be `I`, since the result
        may be a new iterator that results from reconstituting unpacked
        iterators out of the input.  The type returned will model
        `code_point_iter`.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr code_point_iter auto stream_safe(I first, S last);

    /** Copies the stream-safe portion of `r` to the beginning of `r`, and
        returns an iterator to the end of the copied range.  Note that the
        type returned may not be `I`, since the result may be a new iterator
        that results from reconstituting unpacked iterators out of the input.
        The type returned will model `code_point_iter`.  The result is
        returned as a `borrowed_iterator_t` in C++20 and later.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_range R>
    constexpr detail::unspecified stream_safe(R && r);

    /** Returns true iff `[first, last)` is in stream-safe format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr bool is_stream_safe(I first, S last);

    /** Returns true iff `r` is in stream-safe format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_range R>
    constexpr bool is_stream_safe(R && r);

    /** Returns a `stream_safe_view` of the range `[first, last)`.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr detail::unspecified as_stream_safe(I first, S last);

    /** Returns a `stream_safe_view` of the range `r`.  The result is returned
        as a `borrowed_view_t` in C++20 and later.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_range R>
    constexpr detail::unspecified as_stream_safe(R && r);

#endif

    template<typename CPIter, typename Sentinel, typename OutIter>
    constexpr auto stream_safe_copy(CPIter first, Sentinel last, OutIter out)
        ->detail::cp_iter_ret_t<copy_result<CPIter, OutIter>, CPIter>
    {
        return detail::stream_safe_copy_impl(first, last, out);
    }

    template<typename CPRange, typename OutIter>
    constexpr auto stream_safe_copy(CPRange && r, OutIter out)
        ->decltype(boost::text::v1::stream_safe_copy(
            detail::begin(r), detail::end(r), out))
    {
        return v1::stream_safe_copy(detail::begin(r), detail::end(r), out);
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto stream_safe(CPIter first, Sentinel last)
        ->detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::stream_safe_impl(first, last);
    }

    template<typename CPRange>
    constexpr auto stream_safe(CPRange && r)
        ->decltype(boost::text::v1::stream_safe(
            detail::begin(r), detail::end(r)))
    {
        return v1::stream_safe(detail::begin(r), detail::end(r));
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto is_stream_safe(CPIter first, Sentinel last)
        ->detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::is_stream_safe_impl(first, last);
    }

    template<typename CPRange>
    constexpr auto is_stream_safe(CPRange && r)
        ->decltype(boost::text::v1::is_stream_safe(
            detail::begin(r), detail::end(r)))
    {
        return boost::text::v1::is_stream_safe(
            detail::begin(r), detail::end(r));
    }

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires indirectly_copyable<I, O>
    constexpr copy_result<I, O> stream_safe_copy(I first, S last, O out)
    {
        return detail::stream_safe_copy_impl(first, last, out);
    }

    template<code_point_range R, std::weakly_incrementable O>
    requires indirectly_copyable<detail::iterator_t<R>, O>
    constexpr copy_result<std::ranges::borrowed_iterator_t<R>, O>
    stream_safe_copy(R && r, O out)
    {
        return detail::stream_safe_copy_impl(
            std::ranges::begin(r), std::ranges::end(r), out);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr code_point_iter auto stream_safe(I first, S last)
    {
        return detail::stream_safe_impl(first, last);
    }

    template<code_point_range R>
    constexpr auto stream_safe(R && r)
    {
        if constexpr (std::ranges::borrowed_range<R>) {
            return v2::stream_safe(std::ranges::begin(r), std::ranges::end(r));
        } else {
            v2::stream_safe(std::ranges::begin(r), std::ranges::end(r));
            return std::ranges::dangling{};
        }
    }

    template<utf32_iter I, std::sentinel_for<I> S>
    constexpr bool is_stream_safe(I first, S last)
    {
        return detail::is_stream_safe_impl(first, last);
    }

    template<utf32_range R>
    constexpr bool is_stream_safe(R && r)
    {
        return boost::text::is_stream_safe(
            std::ranges::begin(r), std::ranges::end(r));
    }

    template<utf32_range V>
        requires std::ranges::view<V>
    class stream_safe_view : public std::ranges::view_interface<stream_safe_view<V>>
    {
        template<bool Const, bool StoreLast = !detail::is_utf_iter<std::ranges::iterator_t<V>>>
        class iterator;
        class sentinel;

        static constexpr bool bidi =
            std::derived_from<detail::uc_view_category_t<V>, std::bidirectional_iterator_tag>;

        V base_ = V();

    public:
        constexpr stream_safe_view() requires std::default_initializable<V> = default;
        constexpr stream_safe_view(V base) : base_{std::move(base)} {}

        constexpr V base() const & requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr iterator<false> begin() { return iterator<false>{base_}; }
        constexpr iterator<true> begin() const requires utf32_range<const V> { return iterator<true>{base_}; }

        constexpr sentinel end() { return sentinel{}; }
        constexpr iterator<false> end() requires bidi { return iterator<false>{base_, std::ranges::end(base_)}; }
        constexpr sentinel end() const requires utf32_range<const V> { return sentinel{}; }
        constexpr iterator<true> end() const requires utf32_range<const V> && bidi { return iterator<true>{base_, std::ranges::end(base_)}; }
    };

    template<utf32_range V>
        requires std::ranges::view<V>
    template<bool Const, bool StoreLast>
    class stream_safe_view<V>::iterator
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
        // exposition-only, and only defined when std::ranges::iterator_t<Base>
        // is not a specialization of utf_iterator.
        // std::ranges::iterator_t<Base> first_;
        // std::ranges::sentinel_t<Base> last_;
        int nonstarters_ = 0;

        friend class sentinel;

    public:
        constexpr iterator() requires std::default_initializable<std::ranges::iterator_t<Base>> = default;
        constexpr iterator(Base & base) : storage_base(base), it_(std::ranges::begin(base)) {
            if (it_ != storage_base::end(it_) && detail::ccc(*it_))
                nonstarters_ = 1;
        }
        constexpr iterator(Base & base, std::ranges::iterator_t<V> it) requires bidi
            : storage_base(base), it_(it) {}
        constexpr iterator(iterator<!Const, StoreLast> i)
            requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
            : storage_base(i), it_(i.it_), nonstarters_(i.nonstarters_) {}

        constexpr const std::ranges::iterator_t<Base> & base() const & noexcept { return it_; }
        constexpr std::ranges::iterator_t<Base> base() && { return std::move(it_); }

        constexpr char32_t operator*() const { return *it_; }

        constexpr iterator & operator++() {
            if (it_ == storage_base::end(it_))
                return *this;
            ++it_;
            it_ = detail::next_stream_safe_cp(it_, storage_base::end(it_), nonstarters_);
            return *this;
        }

        constexpr iterator & operator--() requires bidi {
            auto const first = storage_base::begin(it_);
            if (it_ == first)
                return *this;
            if (0 < nonstarters_) {
                --it_;
                --nonstarters_;
            } else {
                auto const initial_it = it_;
                auto it = text::find_if_backward(
                    first, it_, [](auto cp) { return detail::ccc(cp) == 0; });
                auto const from = it == it_ ? first : std::ranges::next(it);
                std::ptrdiff_t const nonstarters = std::distance(from, it_);
                nonstarters_ = (std::min)(
                    nonstarters, std::ptrdiff_t(detail::stream_safe_max_nonstarters - 1));
                if (nonstarters_)
                    it_ = std::ranges::next(from, nonstarters_);
                if (it_ == initial_it)
                    --it_;
            }
            return *this;
        }

        friend bool operator==(iterator lhs, iterator rhs)
            { return lhs.base() == rhs.base(); }

        using base_type = boost::stl_interfaces::iterator_interface<
            iterator<Const, StoreLast>,
            detail::uc_view_category_t<V>,
            char32_t,
            char32_t>;
        using base_type::operator++;
        using base_type::operator--;
    };

    template<utf32_range V>
        requires std::ranges::view<V>
    class stream_safe_view<V>::sentinel
    {
    public:
        template<bool Const, bool StoreLast>
        friend constexpr bool operator==(const iterator<Const, StoreLast> & it, sentinel) {
            if constexpr (StoreLast) {
                return it.it_ == it.last_;
            } else {
                return it.base().base() == it.base().end();
            }
        }
    };

    template<class R>
    stream_safe_view(R &&) -> stream_safe_view<std::views::all_t<R>>;

    namespace dtl {
        // Repeating this in every place that needs this concept works around
        // an odd GCC 13 bug.
        template<class R>
        concept can_utf32_view0 = requires { as_utf32(std::declval<R>()); };

        template<class T>
        constexpr bool is_stream_safe_view = false;
        template<class T>
        constexpr bool is_stream_safe_view<stream_safe_view<T>> = true;

        struct stream_safe_impl : range_adaptor_closure<stream_safe_impl>
        {
            template<can_utf32_view0 R>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
                    return std::ranges::empty_view<T>{};
                } else if constexpr (is_stream_safe_view<T>) {
                    return std::forward<R>(r);
                } else if constexpr (is_utf32_view<T>) {
                    return stream_safe_view(std::forward<R>(r));
                } else {
                    return stream_safe_view(std::forward<R>(r) | as_utf32);
                }
            }
        };
    }

    inline constexpr dtl::stream_safe_impl as_stream_safe;

    namespace dtl {
        template<class R>
        concept can_stream_safe = requires { as_stream_safe(std::declval<R>()); };
    }

}}}

namespace std::ranges {
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::v2::stream_safe_view<V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
