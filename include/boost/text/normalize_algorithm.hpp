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

    /** The result of inserting a sequence of code points `S` into another
        sequence of code points `D`, ensuring proper normalization.  Since the
        insertion operation may need to change some code points just before
        and/or just after the insertion due to normalization, the code points
        bounded by this range may be longer than `S`.  Values of this type
        represent the entire sequence of of code points in `D` that have
        chnaged.

        Note that the iterator type refers to the underlying sequence, which
        may not itself be a sequence of code points.  For example, the
        underlying sequence may be a sequence of `char` which is interpreted
        as UTF-8. */
    template<typename Iter>
    struct replace_result : stl_interfaces::view_interface<replace_result<Iter>>
    {
        using iterator = Iter;

        replace_result() : first_(), last_(first_) {}
        replace_result(iterator first, iterator last) :
            first_(first), last_(last)
        {}

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        Iter first_;
        Iter last_;
    };

    namespace dtl {
        template<nf Normalization, typename CPIter>
        CPIter prev_stable_cp(CPIter first, CPIter last) noexcept
        {
            auto const it = find_if_backward(
                first, last, v1::detail::stable_code_point<Normalization>);
            if (it == last)
                return first;
            return it;
        }

        template<nf Normalization, typename CPIter>
        CPIter next_stable_cp(CPIter first, CPIter last) noexcept
        {
            auto const it = find_if(
                first, last, v1::detail::stable_code_point<Normalization>);
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

        template<typename T, typename R1, typename R2>
        struct cons_iter : stl_interfaces::proxy_iterator_interface<
                               cons_iter<T, R1, R2>,
                               std::bidirectional_iterator_tag,
                               T>
        {
            using first_iterator = decltype(std::declval<R1 &>().begin());
            using second_iterator = decltype(std::declval<R2 &>().begin());

            cons_iter() : in_r1_(false) {}
            cons_iter(
                first_iterator it,
                first_iterator r1_last,
                second_iterator r2_first) :
                r1_last_(r1_last),
                it1_(it),
                r2_first_(r2_first),
                it2_(r2_first),
                in_r1_(true)
            {}
            cons_iter(
                second_iterator it,
                first_iterator r1_last,
                second_iterator r2_first,
                int) :
                r1_last_(r1_last),
                it1_(r1_last),
                r2_first_(r2_first),
                it2_(it),
                in_r1_(false)
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

            cons_iter & operator--() noexcept
            {
                if (!in_r1_) {
                    if (it2_ == r2_first_) {
                        in_r1_ = true;
                        --it1_;
                    } else {
                        --it2_;
                    }
                } else {
                    --it1_;
                }
                return *this;
            }

            T operator*() const noexcept { return in_r1_ ? *it1_ : *it2_; }

            friend bool operator==(cons_iter lhs, cons_iter rhs)
            {
                return lhs.in_r1_ == rhs.in_r1_ &&
                       (lhs.in_r1_ ? lhs.it1_ == rhs.it1_
                                   : lhs.it2_ == rhs.it2_);
            }

            using base_type = stl_interfaces::proxy_iterator_interface<
                cons_iter<T, R1, R2>,
                std::bidirectional_iterator_tag,
                T>;
            using base_type::operator++;
            using base_type::operator--;

        private:
            first_iterator r1_last_;
            first_iterator it1_;
            first_iterator r2_first_;
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

        template<
            typename T,
            typename R1,
            typename R2,
            typename Iter1,
            typename Iter2>
        auto cons_view(R1 & r1, R2 & r2, Iter1 first, Iter2 last)
        {
            using iterator = cons_iter<T, R1, R2>;
            return cons_view_t<T, R1, R2>(
                iterator(first, r1.end(), r2.begin()),
                iterator(last, r1.end(), r2.begin(), 0));
        }

        template<typename String, typename CPIter, typename Buffer>
        replace_result<typename String::iterator> string_buffer_replace(
            String & string, CPIter first_, CPIter last_, Buffer const & buffer)
        {
            auto const unpacked =
                boost::text::v1::detail::unpack_iterator_and_sentinel(
                    first_, last_);
            BOOST_ASSERT((std::is_same<
                          decltype(unpacked.tag_),
                          decltype(v1::detail::unpack_iterator_and_sentinel(
                                       string.begin(), string.end())
                                       .tag_)>::value));
            auto const first = unpacked.f_;
            auto const last = unpacked.l_;
            auto const replaceable_size = std::distance(first, last);
            if ((std::ptrdiff_t)buffer.size() <= replaceable_size) {
                auto const it = std::copy(buffer.begin(), buffer.end(), first);
                string.erase(it, last);
                return {first, it};
            } else {
                auto const copy_last = buffer.begin() + replaceable_size;
                std::copy(buffer.begin(), copy_last, first);
                auto const it = string.insert(last, copy_last, buffer.end());
                return {
                    std::prev(it, replaceable_size),
                    std::next(it, buffer.end() - copy_last)};
            }
        }

        template<
            nf Normalization,
            typename String,
            typename CPIter,
            typename Sentinel,
            bool UTF8 =
                Normalization != nf::d && Normalization !=
                    nf::kd &&
                        v1::detail::utf8_fast_path<CPIter, Sentinel>::value &&
                sizeof(*std::declval<String>().begin()) == 1>
        struct normalized_insert_buffer
        {
            using type = container::small_vector<uint16_t, 512>;
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
            using type = container::small_vector<char, 1024>;
        };

        template<
            nf Normalization,
            typename String,
            typename CPIter,
            typename Sentinel = CPIter>
        using normalized_insert_buffer_t = typename normalized_insert_buffer<
            Normalization,
            String,
            CPIter,
            Sentinel>::type;

        template<nf Normalization, typename String, typename Iter>
        replace_result<Iter> erase_impl(String & string, Iter first, Iter last)
        {
            BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();

            if (first == last)
                return {first, first};

            auto const string_cps =
                boost::text::as_utf32(Iter(string.begin()), Iter(string.end()));
            auto const string_cp_first = boost::text::v1::make_utf32_iterator(
                Iter(string.begin()), first, Iter(string.end()));
            auto const string_cp_last = boost::text::v1::make_utf32_iterator(
                Iter(string.begin()), last, Iter(string.end()));

            using stable_cps_type =
                stable_cps_result_t<decltype(string_cps.begin())>;

            // Find the unstable CPs on either side of the deletion, and make a
            // range for each, using first and last as the other bounds
            // respectively.
            auto const string_prefix_range = stable_cps_type{
                dtl::prev_stable_cp<Normalization>(
                    string_cps.begin(), string_cp_first),
                string_cp_first};
            auto const string_suffix_range = stable_cps_type{
                string_cp_last,
                dtl::next_stable_cp<Normalization>(
                    string_cp_last, string_cps.end())};

            if (string_prefix_range.empty() || string_suffix_range.empty()) {
                string.erase(first, last);
                return {first, first};
            }

            normalized_insert_buffer_t<
                Normalization,
                String,
                typename String::iterator>
                buffer;

            // [first-stable-cp-before-at, first-stable-cp-after-first)
            auto const cons_view = dtl::cons_view<uint32_t>(
                string_prefix_range,
                string_suffix_range,
                string_prefix_range.begin(),
                string_suffix_range.end());
            boost::text::v2::normalize_append<Normalization>(cons_view, buffer);

            auto const first_buffer_mismatch = std::mismatch(
                                                   cons_view.begin(),
                                                   cons_view.end(),
                                                   buffer.begin(),
                                                   buffer.end())
                                                   .second;
            if (first_buffer_mismatch == buffer.end()) {
                string.erase(first, last);
                return {first, first};
            }

            auto const last_buffer_mismatch =
                std::mismatch(
                    std::make_reverse_iterator(cons_view.end()),
                    std::make_reverse_iterator(cons_view.begin()),
                    std::make_reverse_iterator(buffer.end()),
                    std::make_reverse_iterator(
                        std::next(first_buffer_mismatch)))
                    .second.base();
            auto const first_buffer_mismatch_offset =
                first_buffer_mismatch - buffer.begin();
            auto const last_buffer_mismatch_offset =
                last_buffer_mismatch - buffer.begin();

            auto const it = dtl::string_buffer_replace(
                                string,
                                string_prefix_range.begin(),
                                string_suffix_range.end(),
                                buffer)
                                .begin();
            return {
                std::next(it, first_buffer_mismatch_offset),
                std::next(it, last_buffer_mismatch_offset)};
        }
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if 0 // TODO
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
#endif

}}}

#if defined(__cpp_lib_concepts)

#include <boost/text/concepts.hpp>

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    /** Inserts `[first, last)` into string `string`, replacing the sequence
        `[from, to)` within `string`, and returning a view indicating the
        changed portion of `string`.  Note that the replacement operation may
        mutate some code points just before or just after the inserted
        sequence.  The output is UTF-8 if `sizeof(*s.begin()) == 1`, and
        UTF-16 otherwise.  The inserted string is not normalized if
        `insertion_norm` is `insertion_normalized`.  The code points at either
        end of the insertion may need to be normalized, regardless of whether
        the inserted string does.

        \pre `string` is in normalization form `Normalization`. */
    template<
        nf Normalization,
        utf_string String,
        std::bidirectional_iterator StringIter,
        code_point_iterator I>
    requires std::is_convertible_v<std::ranges::iterator_t<String>, StringIter>
        // clang-format off
    replace_result<StringIter> replace(
        String & string,
        StringIter from,
        StringIter to,
        I first,
        I last,
        insertion_normalization insertion_norm = insertion_unnormalized)
    // clang-format on
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();

        if (first == last)
            return from;

        auto const string_cps =
            boost::text::as_utf32(Iter(string.begin()), Iter(string.end()));
        auto const string_cp_from =
            boost::text::as_utf32(from, string.end()).begin();
        auto const string_cp_to =
            boost::text::as_utf32(to, string.end()).begin();

        using stable_cps_type =
            dtl::stable_cps_result_t<decltype(string_cps.begin())>;

        // Find the unstable CPs on either side of the insertion, and make a
        // range for each, using from and to as the other bound respectively.
        auto const string_prefix_range = stable_cps_type{
            dtl::prev_stable_cp<Normalization>(
                string_cps.begin(), string_cp_from),
            string_cp_from};
        auto const string_suffix_range = stable_cps_type{
            string_cp_to,
            dtl::next_stable_cp<Normalization>(string_cp_to, string_cps.end())};

        auto const insertion_first_stable =
            dtl::next_stable_cp<Normalization>(first, last);
        // TODO: Special-case the situation where insertion_first_stable ==
        // last.  In that case, the prefix, insertion, and suffix all need to
        // be normalized at once.
        auto const insertion_last_stable =
            dtl::prev_stable_cp<Normalization>(first, last);

        dtl::normalized_insert_buffer_t<Normalization, String, I> buffer;

        // [first-stable-cp-before-at, first-stable-cp-after-first)
        auto const lo_cons_view = dtl::cons_view<uint32_t>(
            string_prefix_range,
            string,
            string_prefix_range.begin(),
            insertion_first_stable);
        boost::text::v2::normalize_append<Normalization>(lo_cons_view, buffer);

        // Middle of insertion is [insertion_first_stable,
        // insertion_last_stable)
        if (insertion_norm == insertion_normalized) {
            if constexpr (utf8_string<String>) {
                auto r = text::as_utf8(
                    insertion_first_stable, insertion_last_stable);
                buffer.insert(buffer.end(), r.begin(), r.end());
            } else {
                auto r = text::as_utf16(
                    insertion_first_stable, insertion_last_stable);
                buffer.insert(buffer.end(), r.begin(), r.end());
            }
        } else {
            text::normalize_append<Normalization>(
                text::as_utf32(insertion_first_stable, insertion_last_stable),
                buffer);
        }

        // [first-stable-cp-before-last, first-stable-cp-after-at)
        auto const hi_cons_view = dtl::cons_view<uint32_t>(
            string,
            string_suffix_range,
            insertion_last_stable,
            string_suffix_range.end());
        text::normalize_append<Normalization>(hi_cons_view, buffer);

        auto const retval = dtl::string_buffer_replace(
            string,
            string_prefix_range.begin(),
            string_suffix_range.end(),
            buffer);
        return retval; // TODO: Return range of changed CPs.
    }

#if 0 // TODO: Only do this if the other range overloads below (with the
      // string subview V) are done.
    /** Inserts `r` into string `string`, replacing the sequence `[from, to)`
        within `string`, and returning a view indicating the changed portion
        of `string`.  Note that the replacement operation may mutate some code
        points just before or just after the inserted sequence.  The output is
        UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16 otherwise.  The
        inserted string is not normalized if `insertion_norm` is
        `insertion_normalized`.  The code points at either end of the
        insertion may need to be normalized, regardless of whether the
        inserted string does.

        \pre `string` is in normalization form `Normalization`. */
    template<nf Normalization, utf_string String, code_point_iterator R>
    replace_result<std::ranges::iterator_t<String>> replace(
        String & string,
        std::ranges::iterator_t<String> from,
        std::ranges::iterator_t<String> to,
        R const & r,
        insertion_normalization insertion_norm = insertion_unnormalized)
    {
        return boost::text::v2::replace<Normalization>(
            string, from, to, std::begin(r), std::end(r), insertion_norm);
    }
#endif

    /** Inserts `[first, last)` into string `string` at location `at`,
        returning a view indicating the changed portion of `string`.  Note
        that the insertion operation may mutate some code points just before
        or just after the inserted sequence.  The output is UTF-8 if
        `sizeof(*s.begin()) == 1`, and UTF-16 otherwise.  The inserted string
        is not normalized if `insertion_norm` is `insertion_normalized`.  The
        code points at either end of the insertion may need to be normalized,
        regardless of whether the inserted string does.

        \pre `string` is in normalization form `Normalization`. */
    template<
        nf Normalization,
        utf_string String,
        std::bidirectional_iterator StringIter,
        code_point_iterator I>
    requires std::is_convertible_v<std::ranges::iterator_t<String>, StringIter>
        // clang-format off
    replace_result<StringIter> insert(
        String & string,
        StringIter at,
        I first,
        I last,
        insertion_normalization insertion_norm = insertion_unnormalized)
    // clang-format on
    {
        return boost::text::v2::replace<Normalization>(
            string, at, at, first, last, insertion_norm);
    }

    /** Inserts `r` into string `string` at location `at`, returning a view
        indicating the changed portion of `string`.  Note that the insertion
        operation may mutate some code points just before or just after the
        inserted sequence.  The output is UTF-8 if `sizeof(*s.begin()) == 1`,
        and UTF-16 otherwise.  The inserted string is not normalized if
        `insertion_norm` is `insertion_normalized`.  The code points at either
        end of the insertion may need to be normalized, regardless of whether
        the inserted string does.

        \pre `string` is in normalization form `Normalization`. */
    template<
        nf Normalization,
        utf_string String,
        std::bidirectional_iterator StringIter,
        code_point_range R>
    requires std::is_convertible_v<std::ranges::iterator_t<String>, StringIter>
        // clang-format off
    replace_result<StringIter> insert(
        String & string,
        StringIter at,
        R const & r,
        insertion_normalization insertion_norm = insertion_unnormalized)
    // clang-format on
    {
        return boost::text::v2::replace<Normalization>(
            string, at, at, std::begin(r), std::end(r), insertion_norm);
    }

    /** Erases the subsequence `[from, to)` within `string`, returning a view
        indicating the changed portion of `string`.  Note that the insertion
        operation may mutate some code points just before or just after the
        erased sequence.

        \pre `string` is in normalization form `Normalization`. */
    template<
        nf Normalization,
        utf_string String,
        std::bidirectional_iterator StringIter>
    requires std::is_convertible_v<std::ranges::iterator_t<String>, StringIter>
        // clang-format off
    replace_result<StringIter> erase(
        String & string, StringIter from, StringIter to)
    // clang-format on
    {
        return dtl::erase_impl<Normalization>(string, from, to);
    }

    /* TODO: Do these? What are the constraints on V that make sense?

    template<nf Normalization, utf_string String, std::ranges::view V>
    std::ranges::iterator_t<String> erase(String & string, V substring);

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

#endif
