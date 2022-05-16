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
#include <boost/text/view_adaptor.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    /** An alias for `in_out_result` returned by algorithms that perform a
        copy. */
    template<typename I, typename O>
    using copy_result = in_out_result<I, O>;

    namespace detail {
        enum : std::size_t { stream_safe_max_nonstarters = 9 };

        template<typename CPIter, typename Sentinel>
        CPIter next_stream_safe_cp(
            CPIter first, Sentinel last, std::size_t & nonstarters)
        {
            for (; first != last; ++first) {
                auto const cp = *first;
                if (detail::ccc(cp) == 0)
                    nonstarters = 0;
                else
                    ++nonstarters;
                if (nonstarters < stream_safe_max_nonstarters)
                    break;
            }
            return first;
        }

        template<typename Iter, typename Sentinel, typename Repack>
        auto
        stream_safe_out(tagged_range<utf8_tag, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_8_out(unpacked.f_);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            tagged_range<utf16_tag, Iter, Sentinel, Repack> unpacked)
        {
            return boost::text::utf_32_to_16_out(unpacked.f_);
        }
        template<typename Iter, typename Sentinel, typename Repack>
        auto stream_safe_out(
            tagged_range<utf32_tag, Iter, Sentinel, Repack> unpacked)
        {
            return unpacked.f_;
        }

        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            tagged_range<utf8_tag, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.f_, out.base(), unpacked.l_);
        }
        template<
            typename OutIter,
            typename Iter,
            typename Sentinel,
            typename Repack>
        auto stream_safe_result(
            tagged_range<utf16_tag, Iter, Sentinel, Repack> unpacked,
            OutIter out)
        {
            return boost::text::utf32_iterator(
                unpacked.f_, out.base(), unpacked.l_);
        }
        template<typename OutIter>
        auto stream_safe_result(utf32_tag, OutIter out)
        {
            return out;
        }

        template<typename I, typename S, typename O>
        copy_result<I, O> stream_safe_copy_impl(I first, S last, O out)
        {
            if (first == last)
                return {first, out};
            std::size_t nonstarters = detail::ccc(*first) ? 1 : 0;
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
                detail::unpack_iterator_and_sentinel(first, last);
            auto out = detail::stream_safe_out(unpacked);
            out = detail::stream_safe_copy_impl(first, last, out).out;
            return detail::stream_safe_result(unpacked, out);
        }

        template<typename I, typename S>
        bool is_stream_safe_impl(I first, S last)
        {
            std::size_t nonstarters = 0;
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

    /** An iterator that filters out all the non-stream-safe code points from
        a sequence of code points.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct stream_safe_iterator : stl_interfaces::iterator_interface<
                                      stream_safe_iterator<I, S>,
                                      std::bidirectional_iterator_tag,
                                      uint32_t,
                                      uint32_t>
    {
        using iterator = I;
        using sentinel = S;

        stream_safe_iterator() : first_(), it_(), last_() {}
        stream_safe_iterator(iterator first, sentinel last) :
            first_(first),
            it_(first),
            last_(last),
            nonstarters_(it_ != last_ && detail::ccc(*it_) ? 1 : 0)
        {}

        uint32_t operator*() const
        {
            BOOST_ASSERT(it_ != last_);
            return *it_;
        }

        constexpr I base() const { return it_; }

        stream_safe_iterator & operator++()
        {
            BOOST_ASSERT(it_ != last_);
            ++it_;
            it_ = detail::next_stream_safe_cp(it_, last_, nonstarters_);
            return *this;
        }

        stream_safe_iterator & operator--()
        {
            BOOST_ASSERT(it_ != first_);
            if (0 < nonstarters_) {
                --it_;
            } else {
                auto const it = boost::text::find_if_backward(
                    first_, it_, [](auto cp) { return detail::ccc(cp) == 0; });
                auto const from = it == it_ ? first_ : std::next(it);
                std::size_t const nonstarters = std::distance(from, it_);
                nonstarters_ = (std::min)(
                    nonstarters, detail::stream_safe_max_nonstarters - 1);
                it_ = std::next(it == it_ ? first_ : it, nonstarters_);
            }
            return *this;
        }

        friend bool
        operator==(stream_safe_iterator lhs, stream_safe_iterator rhs)
        {
            return lhs.base() == rhs.base();
        }

        using base_type = stl_interfaces::iterator_interface<
            stream_safe_iterator<I, S>,
            std::bidirectional_iterator_tag,
            uint32_t,
            uint32_t>;
        using base_type::operator++;
        using base_type::operator--;

    private:
        iterator first_;
        iterator it_;
        [[no_unique_address]] sentinel last_;
        std::size_t nonstarters_ = 0;
    };

    template<typename I, typename S>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator==(stream_safe_iterator<I, S> const & lhs, S rhs)
        -> decltype(lhs.base() == rhs)
    {
        return lhs.base() == rhs.base();
    }
    template<typename I, typename S>
    auto operator==(S lhs, stream_safe_iterator<I, S> const & rhs)
        -> decltype(rhs.base() == lhs)
    {
        return rhs.base() == lhs;
    }
    template<typename I, typename S>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator!=(stream_safe_iterator<I, S> const & lhs, S rhs)
        -> decltype(lhs.base() != rhs)
    {
        return lhs.base() != rhs.base();
    }
    template<typename I, typename S>
    auto operator!=(S lhs, stream_safe_iterator<I, S> const & rhs)
        -> decltype(rhs.base() != lhs)
    {
        return rhs.base() != lhs;
    }

    namespace detail {
        template<typename T>
        struct is_stream_safe_iter : std::false_type
        {};
        template<typename T, typename U>
        struct is_stream_safe_iter<stream_safe_iterator<T, U>> : std::true_type
        {};
    }

    /** A view that adapts a non-stream safe sequence of code points to a
        stream-safe sequence of code points.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    // clang-format off
#if BOOST_TEXT_USE_CONCEPTS
    template<typename I, std::sentinel_for<I> S = I>
    requires detail::is_stream_safe_iter<I>::value
#else
    template<typename I, typename S = I>
#endif
    struct stream_safe_view
        // clang-format on
        : stl_interfaces::view_interface<stream_safe_view<I, S>>
    {
        using iterator = I;
        using sentinel = S;

        constexpr stream_safe_view() {}
        constexpr stream_safe_view(iterator first, sentinel last) :
            first_(first), last_(last)
        {}

        constexpr iterator begin() const { return first_; }
        constexpr sentinel end() const { return last_; }

        friend constexpr bool
        operator==(stream_safe_view lhs, stream_safe_view rhs)
        {
            return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
        }

    private:
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

    namespace detail {
        template<typename I, typename S>
        constexpr auto as_stream_safe_impl(I first, S last)
        {
            using iterator = stream_safe_iterator<I, S>;
            return stream_safe_view<iterator, S>{iterator{first, last}, last};
        }

        template<typename I>
        constexpr auto as_stream_safe_impl(I first, I last)
        {
            using iterator = stream_safe_iterator<I>;
            return stream_safe_view<iterator>{
                iterator{first, last}, iterator{last, last}};
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
        may be a new iterator that results from reconstituted unpacked
        iterators out of the input.  The type returned will model
        `code_point_iter`.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr code_point_iter auto stream_safe(I first, S last);

    /** Copies the stream-safe portion of `r` to the beginning of `r`, and
        returns an iterator to the end of the copied range.  Note that the
        type returned may not be `I`, since the result may be a new iterator
        that results from reconstituted unpacked iterators out of the input.
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

    namespace dtl {
        struct as_stream_safe_impl : range_adaptor_closure<as_stream_safe_impl>
        {
            template<typename CPIter, typename Sentinel>
            constexpr auto
            operator()(CPIter first, Sentinel last) const
                -> detail::cp_iter_ret_t<
                    stream_safe_view<
                        stream_safe_iterator<CPIter, Sentinel>,
                        Sentinel>,
                    CPIter>
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<typename CPIter>
            constexpr auto
            operator()(CPIter first, CPIter last) const
                -> detail::cp_iter_ret_t<
                    stream_safe_view<stream_safe_iterator<CPIter>>,
                    CPIter>
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<typename CPRange>
            constexpr auto operator()(CPRange && r) const
                -> decltype((*this)(detail::begin(r), detail::end(r)))
            {
                return (*this)(detail::begin(r), detail::end(r));
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_stream_safe_impl as_stream_safe;
#else
    namespace {
        constexpr dtl::as_stream_safe_impl as_stream_safe;
    }
#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires std::indirectly_copyable<I, O>
    constexpr copy_result<I, O> stream_safe_copy(I first, S last, O out)
    {
        return detail::stream_safe_copy_impl(first, last, out);
    }

    template<code_point_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<detail::iterator_t<R>, O>
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

    template<code_point_iter I, std::sentinel_for<I> S>
    constexpr bool is_stream_safe(I first, S last)
    {
        return detail::is_stream_safe_impl(first, last);
    }

    template<code_point_range R>
    constexpr bool is_stream_safe(R && r)
    {
        return boost::text::is_stream_safe(
            std::ranges::begin(r), std::ranges::end(r));
    }

    namespace dtl {
        struct as_stream_safe_impl : range_adaptor_closure<as_stream_safe_impl>
        {
            template<code_point_iter I, std::sentinel_for<I> S>
            constexpr stream_safe_view<stream_safe_iterator<I, S>, S>
            operator()(I first, S last) const
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<code_point_iter I>
            constexpr stream_safe_view<stream_safe_iterator<I>>
            operator()(I first, I last) const
            {
                return detail::as_stream_safe_impl(first, last);
            }

            template<code_point_range R>
            constexpr auto operator()(R && r) const
            {
                if constexpr (std::ranges::borrowed_range<R>)
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                else
                    return std::ranges::dangling{};
            }
        };
    }

    inline constexpr dtl::as_stream_safe_impl as_stream_safe;

}}}

namespace std::ranges {
    template<typename I, std::sentinel_for<I> S>
    // clang-format off
    requires boost::text::detail::is_stream_safe_iter<I>::value
    inline constexpr bool
        // clang-format on
        enable_borrowed_range<boost::text::stream_safe_view<I, S>> = true;
}

#endif

#endif
