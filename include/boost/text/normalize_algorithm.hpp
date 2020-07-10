// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_NORMALIZE_ALGORITHM_HPP
#define BOOST_TEXT_NORMALIZE_ALGORITHM_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/normalize_string.hpp>


namespace boost { namespace text {

    /** An enumeration used with the container `insert()` free functions, to
        indicate whether the inserted text is known to be normalized. */
    enum insertion_normalization {
        insertion_normalized,  // The inserted text is known to be in the
                               // correct normalization form.
        insertion_unnormalized // The inserted text is in an unknown
                               // normalization form.
    };

    namespace detail {
        template<nf Normalization, typename CPIter>
        CPIter prev_stable_cp(CPIter first, CPIter last) noexcept
        {
            auto const it =
                find_if_backward(first, last, stable_code_point<Normalization>);
            if (it == last)
                return first;
            return it;
        }

        template<nf Normalization, typename CPIter>
        CPIter next_stable_cp(CPIter first, CPIter last) noexcept
        {
            auto const it =
                find_if(first, last, stable_code_point<Normalization>);
            return it;
        }

        template<typename CPIter>
        struct stable_cps_result_t
        {
            bool empty() const noexcept { return first_ == last_; }
            CPIter begin() const noexcept { return first_; }
            CPIter end() const noexcept { return last_; }
            CPIter first_;
            CPIter last_;
        };

        template<nf Normalization, typename CPIter>
        stable_cps_result_t<CPIter>
        stable_cps(CPIter first, CPIter at, CPIter last)
        {
            return {
                prev_stable_cp<Normalization>(first, at),
                next_stable_cp<Normalization>(at, last)};
        }

        template<typename T, typename R1, typename R2>
        struct cons_iter : stl_interfaces::proxy_iterator_interface<
                               cons_iter<T, R1, R2>,
                               std::forward_iterator_tag,
                               T>
        {
            using first_iterator = typename R1::const_iterator;
            using second_iterator = typename R2::const_iterator;

            cons_iter(
                first_iterator it,
                first_iterator r1_last,
                second_iterator r2_first) :
                r1_last_(r1_llast), it1_(it), it2_(r2_first), in_r1_(true)
            {}
            // end-iterator ctor
            cons_iter(second_iterator it) :
                r1_last_(), it1_(), it2_(it), in_r1_(false)
            {}

            cons_iter & operator++() noexcept
            {
                if (in_r1_) {
                    BOOST_ASSERT(it1_ != r1_last_);
                    ++it1_;
                    if (it1_ == r1_last_)
                        in_r1_ = false;
                } else {
                    ++it2_;
                }
                return *this;
            }

            T operator*() const noexcept { return lhs.in_r1_ ? *it1_ : *it2_; }

            friend bool operator==(cons_iter lhs, cons_iter rhs)
            {
                return lhs.in_r1_ == rhs.in_r1_ &&
                       (lhs.in_r1_ ? lhs.it1_ == rhs.it1_
                                   : lhs.it2_ == rhs.it2_);
            }

            using base_type = stl_interfaces::proxy_iterator_interface<
                cons_iter<T, R1, R2>,
                std::forward_iterator_tag,
                T>;
            using base_type::operator++;

        private:
            first_iterator r1_last_;
            first_iterator it1_;
            second_iterator it2_;
            bool in_r1_;
        };

        template<typename T, typename R1, typename R2>
        struct cons_view_t
            : stl_interfaces::view_interface<cons_view_t<T, R1, R2>>
        {
            using iterator = cons_iter<T, R1, R2>;

            cons_view_t(iterator first, iterator last) :
                first_(first), last_(last)
            {}

            iterator begin() const noexcept { return first_; }
            iterator end() const noexcept { return last_; }

        private:
            iterator first_;
            iterator last_;
        };

        template<typename T, typename R1, typename R2>
        auto cons_view(
            R1 const & r1,
            R2 const & r2,
            typename R1::const_iterator first,
            typename R2::const_iterator last)
        {
            using iterator = cons_iter<T, R1, R2>;
            return cons_view_t<T, R1, R2>(
                iterator(first, r1.end(), r2.begin()), iterator(last));
        }

        template<
            nf Normalization,
            typename String,
            typename CPIter,
            typename Sentinel,
            bool UTF8 = Normalization != nf::d && Normalization !=
                            nf::kd && utf8_fast_path<CPIter, Sentinel>::value &&
                        sizeof(*std::declval<String>().begin()) == 1>
        struct normalized_insert_buffer
        {
            using type = container::small_vector<uint16_t>;
        };

        template<
            nf Normalization,
            typename String,
            typename CPIter,
            typename Sentinel>
        struct normalized_insert_buffer<
            Normalization,
            String,
            CPIter,
            Sentinel,
            true>
        {
            using type = container::small_vector<char>;
        };

        template<
            nf Normalization,
            typename String,
            typename CPIter,
            typename Sentinel>
        using normalized_insert_buffer_t = typename normalized_insert_buffer<
            Normalization,
            String,
            CPIter,
            Sentinel>::type;
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    /** Inserts sequence `[first, last)` into `cont` at location `at`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<
        nf Normalization,
        typename CPIter,
        typename Sentinel,
        typename Cont>
    inline auto insert(
        Cont & cont,
        typename Cont::iterator at,
        CPIter first,
        Sentinel last,
        insertion_normalization insertion_norm = insertion_unnormalized)
        -> detail::cp_iter_ret_t<typename Cont::iterator, CPIter>
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        // TODO
        return at;
    }

    /** Writes sequence `[first, last)` to `out`, ensuring Stream-Safe Text
        Format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<nf Normalization, typename CPRange, typename Cont>
    inline typename Cont::iterator insert(
        Cont & cont,
        typename Cont::iterator at,
        CPRange const & r,
        insertion_normalization insertion_norm = insertion_unnormalized)
    {
        return boost::text::v1::insert<Normalization>(
            cont, at, std::begin(r), std::end(r), insertion_norm);
    }

}}}

#if defined(__cpp_lib_concepts)

#include <boost/text/concepts.hpp>

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    /** Inserts `[first, last)` into string `string` at location `at`,
        returning an iterator to the first inserted code point.  The output is
        UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16 otherwise.  The
        inserted string is not normalized if `insertion_norm` is
        `insertion_normalized`.  The code points at either end of the
        insertion may need to be normalized, regardless of whether the
        inserted string does. */
    template<nf Normalization, utf_string String, code_point_iterator I>
    std::ranges::iterator_t<String> insert(
        String & string,
        std::ranges::iterator_t<String> at,
        I first,
        I last,
        insertion_normalization insertion_norm = insertion_unnormalized)
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();

        auto const string_cps = boost::text::v1::as_utf32(string);
        auto const string_cp_at =
            boost::text::v1::as_utf32(at, string.end()).begin();

        // Find the stable CPs around the insertion point, and make a range
        // for each, using at as the other bound.
        auto const stable_cps = detail::stable_cps(
            string_cps.begin(), string_cp_at, string_cps.end());
        auto string_prefix_range = stable_cps;
        auto string_suffix_range = stable_cps;
        string_prefix_range.last_ = string_cp_at;
        string_suffix_range.first_ = string_cp_at;

        auto const insertion_first_stable =
            next_stable_cp<Normalization>(first, last);
        auto const insertion_last_stable =
            prev_stable_cp<Normalization>(first, last);

        detail::normalized_insert_buffer<
            Normalization,
            typename String,
            typename I,
            typename S>
            buffer;

        // [first-stable-cp-before-at, first-stable-cp-after-first)
        auto const lo_cons_view = detail::cons_view<uint32_t>(
            string_prefix_range,
            string,
            string_prefix_range.begin(),
            insertion_first_stable);
        boost::text::v2::normalize_append<Normalization>(lo_cons_view, buffer);

        // Middle of insertion is [insertion_first_stable,
        // insertion_last_stable)
        if (insertion_norm == insertion_normalized) {
            if constexpr (utf8_string<String>) {
                auto r = boost::text::v2::as_utf8(
                    insertion_first_stable, insertion_last_stable);
                buffer.insert(buffer.end(), r.begin(), r.end());
            } else {
                auto r = boost::text::v2::as_utf16(
                    insertion_first_stable, insertion_last_stable);
                buffer.insert(buffer.end(), r.begin(), r.end());
            }
        } else {
            boost::text::v2::normalize_append<Normalization>(
                boost::text::v1::as_utf32(
                    insertion_first_stable, insertion_last_stable),
                buffer);
        }

        // [first-stable-cp-before-last, first-stable-cp-after-at)
        auto const hi_cons_view = detail::cons_view<uint32_t>(
            string,
            string_suffix_range,
            insertion_last_stable,
            string_suffix_range.end());
        boost::text::v2::normalize_append<Normalization>(hi_cons_view, buffer);

        auto const unpacked_stable_cps = detail::unpack_iterator_and_sentinel(
            stable_cps.begin(), stable_cps.end());
        BOOST_ASSERT(
            unpacked_stable_cps.tag_ ==
            detail::unpack_iterator_and_sentinel(string.begin(), string.end())
                .tag_);
        auto const replaceable_size =
            std::distance(unpacked_stable_cps.f_, unpacked_stable_cps.l_);
        if (buffer.size() <= replaceable_size) {
            std::ranges::copy(buffer, unpacked_stable_cps.f_);
            return string.erase(
                std::next(
                    unpacked_stable_cps.f_, replaceable_size - buffer.size()),
                unpacked_stable_cps.l_);
        } else {
            auto const copy_last = buffer.begin() + replaceable_size;
            std::ranges::copy(
                buffer.begin(), copy_last, unpacked_stable_cps.f_);
            return string.insert(
                unpacked_stable_cps.l_, copy_last, buffer.end());
        }
    }

    /** Inserts `r` into string `string` at location `at`, returning an
        iterator to the first inserted code point.  The output is UTF-8 if
        `sizeof(*s.begin()) == 1`, and UTF-16 otherwise.  The inserted string
        is not normalized if `insertion_norm` is `insertion_normalized`.  The
        code points at either end of the insertion may need to be normalized,
        regardless of whether the inserted string does. */
    template<nf Normalization, utf_string String, code_point_range R>
    std::ranges::iterator_t<String> insert(
        String & string,
        std::ranges::iterator_t<String> at,
        R const & r,
        insertion_normalization insertion_norm = insertion_unnormalized)
    {
        return boost::text::v1::insert<Normalization>(
            string, at, std::begin(r), std::end(r), insertion_norm);
    }

    /* TODO
    template<nf Normalization, utf_string String>
    std::ranges::iterator_t<String> erase(
        String & string,
        std::ranges::iterator_t<String> from,
        std::ranges::iterator_t<String> to);

    template<nf Normalization, utf_string String, std::ranges::view V>
    std::ranges::iterator_t<String> erase(String & string, V substring);

    template<nf Normalization, utf_string String, code_point_iterator I>
    String & replace(
        String & string,
        std::ranges::iterator_t<String> from,
        std::ranges::iterator_t<String> to,
        I first,
        I last,
        insertion_normalization insertion_norm = insertion_unnormalized);

    template<
        nf Normalization,
        utf_string String,
        std::ranges::view V,
        code_point_iterator I>
    String & replace(
        String & string,
        V substring,
        I first,
        I last,
        insertion_normalization insertion_norm = insertion_unnormalized);

    template<nf Normalization, utf_string String, std::ranges::range R>
    String & replace(
        String & string,
        std::ranges::iterator_t<String> from,
        std::ranges::iterator_t<String> to,
        R const & r,
        insertion_normalization insertion_norm = insertion_unnormalized);

    template<
        nf Normalization,
        utf_string String,
        std::ranges::view V,
        std::ranges::range R>
    String & replace(
        String & string,
        V substring,
        R const & r,
        insertion_normalization insertion_norm = insertion_unnormalized);
    */

}}}

#endif
