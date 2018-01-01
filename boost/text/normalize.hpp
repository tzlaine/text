#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/container/static_vector.hpp>


namespace boost { namespace text {

    namespace detail {

        template<typename Iter, std::size_t Capacity>
        void order_canonically(
            Iter first,
            Iter last,
            container::static_vector<int, Capacity> & cccs) noexcept
        {
            assert(first != last);

            std::transform(first, last, cccs.begin(), ccc);

            --last;
            while (first != last) {
                auto it = first;
                auto new_last = first;
                auto ccc_it = cccs.begin();
                while (it != last) {
                    auto next = std::next(it);
                    auto ccc_next = std::next(ccc_it);
                    auto const ccc_a = *ccc_it;
                    auto const ccc_b = *ccc_next;
                    if (0 < ccc_b && ccc_b < ccc_a) {
                        std::iter_swap(it, next);
                        std::iter_swap(ccc_it, ccc_next);
                        new_last = it;
                    }
                    ++it;
                    ++ccc_it;
                }
                last = new_last;
            }
        }

        template<std::size_t Capacity, typename FlushFunc>
        bool flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc & flush)
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                order_canonically(buffer.begin(), buffer.end(), cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<
            typename Iter,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename FlushFunc>
        bool normalize_to_decomposed_impl(
            Iter first,
            Iter last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            FlushFunc && flush)
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                if (!ccc(decomp.storage_[0])) {
                    if (!detail::flush_buffer(buffer, flush))
                        return false;
                }
                buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                ++first;
            }
            if (!detail::flush_buffer(buffer, flush))
                return false;
            return true;
        }

        template<typename Iter, typename OutIter, typename DecomposeFunc>
        OutIter normalize_to_decomposed(
            Iter first, Iter last, OutIter out, DecomposeFunc && decompose)
        {
            container::static_vector<uint32_t, 64> buffer;
            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_decomposed_impl(
                first,
                last,
                buffer,
                decompose,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(
                        utf8::make_from_utf32_iterator(first),
                        utf8::make_from_utf32_iterator(last),
                        out);
                    return true;
                });

            return out;
        }

        inline constexpr bool hangul_l(uint32_t cp) noexcept
        {
            return 0x1100 <= cp && cp <= 0x1112;
        }
        inline constexpr bool hangul_v(uint32_t cp) noexcept
        {
            return 0x1161 <= cp && cp <= 0x1175;
        }
        inline constexpr bool hangul_t(uint32_t cp) noexcept
        {
            return 0x11a8 <= cp && cp <= 0x11c2;
        }

        template<bool DisallowDiscontiguous, std::size_t Capacity>
        void compose(
            container::static_vector<uint32_t, Capacity> & buffer,
            container::static_vector<int, Capacity> & cccs)
        {
            assert(buffer.size() == cccs.size());
            assert(2 <= buffer.size());

            auto starter_it = buffer.begin();
            auto it = std::next(buffer.begin());
            auto ccc_it = std::next(cccs.begin());
            while (it != buffer.end()) {
                // Hangul composition as described in Unicode 10.0 Section 3.12.
                auto const hangul_cp0 = *starter_it;
                auto const hangul_cp1 = *it;
                if (it == starter_it + 1 && hangul_l(hangul_cp0) &&
                    hangul_v(hangul_cp1)) {
                    auto const cp2_it = it + 1;
                    auto const hangul_cp2 =
                        cp2_it == buffer.end() ? 0 : *cp2_it;
                    if (hangul_t(hangul_cp2)) {
                        *starter_it =
                            compose_hangul(hangul_cp0, hangul_cp1, hangul_cp2);
                        buffer.erase(it, it + 2);
                        cccs.erase(ccc_it, ccc_it + 2);
                    } else {
                        *starter_it = compose_hangul(hangul_cp0, hangul_cp1);
                        buffer.erase(it, it + 1);
                        cccs.erase(ccc_it, ccc_it + 1);
                    }
                } else {
                    auto const prev_ccc = *std::prev(ccc_it);
                    auto const ccc = *ccc_it;
                    uint32_t composition = 0;
                    if ((it == starter_it + 1 ||
                         (!DisallowDiscontiguous &&
                          (prev_ccc != 0 && prev_ccc < ccc))) &&
                        (composition = compose_unblocked(*starter_it, *it))) {
                        *starter_it = composition;
                        buffer.erase(it);
                        cccs.erase(ccc_it);
                    } else {
                        ++it;
                        ++ccc_it;
                        if (it == buffer.end() &&
                            starter_it < buffer.end() - 2) {
                            ++starter_it;
                            it = std::next(starter_it);
                            ccc_it = cccs.begin() + (it - buffer.begin());
                        }
                    }
                }
            }
        }

        template<
            bool DisallowDiscontiguous,
            std::size_t Capacity,
            typename FlushFunc>
        bool compose_and_flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc &&
                flush) noexcept(noexcept(flush(buffer.begin(), buffer.end())))
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                order_canonically(buffer.begin(), buffer.end(), cccs);
            if (2 <= buffer.size())
                compose<DisallowDiscontiguous>(buffer, cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<std::size_t Capacity>
        bool hangul_final_v(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return !buffer.empty() && hangul_l(buffer.back()) && hangul_v(cp);
        }

        template<std::size_t Capacity>
        bool hangul_final_t(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return 2 <= buffer.size() && hangul_l(buffer[buffer.size() - 2]) &&
                   hangul_v(buffer.back()) && hangul_t(cp);
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename QuickCheckFunc,
            typename FlushFunc>
        bool normalize_to_composed_impl(
            Iter first,
            Iter last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_,
            FlushFunc && flush) noexcept
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                auto const it = std::find_if(
                    decomp.begin(), decomp.end(), [&quick_check_](uint32_t cp) {
                        // TODO: Neither of these checks can be omitted.
                        // Experiment with combining these into a single hash
                        // lookup and measure the perf impact.
                        return !ccc(cp) && quick_check_(cp) == quick_check::yes;
                    });
                if (it != decomp.end() && !hangul_final_v(buffer, *it) &&
                    !hangul_final_t(buffer, *it)) {
                    buffer.insert(buffer.end(), decomp.begin(), it);
                    if (!detail::compose_and_flush_buffer<
                            DisallowDiscontiguous>(buffer, flush)) {
                        return false;
                    }
                    buffer.insert(buffer.end(), it, decomp.end());
                } else {
                    buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                }
                ++first;
            }
            if (!detail::compose_and_flush_buffer<DisallowDiscontiguous>(
                    buffer, flush)) {
                return false;
            }
            return true;
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            typename OutIter,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        OutIter normalize_to_composed(
            Iter first,
            Iter last,
            OutIter out,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_)
        {
            container::static_vector<uint32_t, 64> buffer;

            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_composed_impl<DisallowDiscontiguous>(
                first,
                last,
                buffer,
                decompose,
                quick_check_,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(
                        utf8::make_from_utf32_iterator(first),
                        utf8::make_from_utf32_iterator(last),
                        out);
                    return true;
                });

            return out;
        }

#if 0
        // TODO: The logic in
        // http://www.unicode.org/reports/tr15/tr15-45.html#Detecting_Normalization_Forms
        // seems to indicate that if a supplementary character is encountered
        // in normalized_quick_check(), then we should proceed as normal for
        // this iteration, but then do a double increment of the loop control
        // variable.  That looks wrong, so I'm leaving that out for now.
        bool supplemantary(uint32_t cp)
        {
            return 0x10000 <= cp && cp <= 0x10FFFF;
        }
#endif

        // TODO: Experiment with writing out teh ccc values for reuse in case
        // the result is not quick_check::yes.
        template<typename Iter, typename QuickCheckFunc>
        quick_check normalized_quick_check(
            Iter first, Iter last, QuickCheckFunc && quick_check_) noexcept
        {
            quick_check retval = quick_check::yes;
            int prev_ccc = 0;
            while (first != last) {
                auto const cp = *first;
#if 0
                // See note above.
                if (supplemantary(cp))
                    ++first;
#endif
                auto const check = quick_check_(cp);
                if (check == quick_check::no)
                    return quick_check::no;
                if (check == quick_check::maybe)
                    retval = quick_check::maybe;
                auto const ccc_ = ccc(cp);
                if (ccc_ && ccc_ < prev_ccc)
                    return quick_check::no;
                prev_ccc = ccc_;
                ++first;
            }
            return retval;
        }

        template<typename Iter, typename DecomposeFunc, typename QuickCheckFunc>
        bool normalized_decomposed(
            Iter first,
            Iter last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return normalize_to_decomposed_impl(
                    first,
                    last,
                    buffer,
                    decompose,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }

        template<typename Iter, typename DecomposeFunc, typename QuickCheckFunc>
        bool normalized_composed(
            Iter first,
            Iter last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return normalize_to_composed_impl<false>(
                    first,
                    last,
                    buffer,
                    decompose,
                    quick_check_,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }
    }

    // TODO: This produces different results than the string& overload.  The
    // output contains code points above 0xffffff.
    /** TODO */
    template<typename Iter, typename OutIter>
    inline OutIter normalize_to_nfd(Iter first, Iter last, OutIter out)
    {
        return detail::normalize_to_decomposed(
            first, last, out, [](uint32_t cp) {
                return detail::canonical_decompose(cp);
            });
    }

    /** TODO */
    inline void normalize_to_nfd(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfd(
            as_utf32.begin(), as_utf32.end(), std::inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** TODO */
    template<typename Iter, typename OutIter>
    inline OutIter normalize_to_nfkd(Iter first, Iter last, OutIter out)
    {
        return detail::normalize_to_decomposed(
            first, last, out, [](uint32_t cp) {
                return detail::compatible_decompose(cp);
            });
    }

    /** TODO */
    inline void normalize_to_nfkd(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfkd(
            as_utf32.begin(), as_utf32.end(), std::inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** TODO */
    template<typename Iter, typename OutIter>
    inline OutIter normalize_to_nfc(Iter first, Iter last, OutIter out)
    {
        return detail::normalize_to_composed<false>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** TODO */
    inline void normalize_to_nfc(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(temp.size());

        normalize_to_nfc(
            as_utf32.begin(), as_utf32.end(), std::inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** TODO */
    template<typename Iter, typename OutIter>
    inline OutIter normalize_to_nfkc(Iter first, Iter last, OutIter out)
    {
        return detail::normalize_to_composed<false>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });
    }

    /** TODO */
    inline void normalize_to_nfkc(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfkc(
            as_utf32.begin(), as_utf32.end(), std::inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    // TODO: Create a uint32 iterator concept and apply it below.

    // TODO: Document how the normalized_*() functions should be used.
    // TODO: Document the assumption of safe stream format.

    /** Returns true iff the given sequence of code points is normalized
        NFD. */
    template<typename Iter>
    bool normalized_nfd(Iter first, Iter last) noexcept
    {
        return detail::normalized_decomposed(
            first,
            last,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfd_code_point(cp); });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFKD. */
    template<typename Iter>
    bool normalized_nfkd(Iter first, Iter last) noexcept
    {
        return detail::normalized_decomposed(
            first,
            last,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkd_code_point(cp);
            });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFC. */
    template<typename Iter>
    bool normalized_nfc(Iter first, Iter last) noexcept
    {
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFKC. */
    template<typename Iter>
    bool normalized_nfkc(Iter first, Iter last) noexcept
    {
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });
    }

    /** Returns true iff the given sequence of code points is in the
        pseudonormalized FCD form. */
    template<typename Iter>
    bool pseudonormalized_fcd(Iter first, Iter last) noexcept
    {
        // http://www.unicode.org/notes/tn5/#FCD_Test
        int prev_ccc = 0;
        while (first != last) {
            auto const cp = *first;
            auto const decomp = detail::canonical_decompose(cp);
            auto const ccc = detail::ccc(*decomp.begin());
            if (ccc && ccc < prev_ccc)
                return false;
            prev_ccc =
                decomp.size_ == 1 ? ccc : detail::ccc(*(decomp.end() - 1));
            ++first;
        }
        return false;
    }

    /** TODO */
    template<typename Iter, typename OutIter>
    inline OutIter pseudonormalize_to_fcc(Iter first, Iter last, OutIter out)
    {
        return detail::normalize_to_composed<true>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** TODO */
    inline void pseudonormalize_to_fcc(string & s)
    {
        // http://www.unicode.org/notes/tn5/#FCC
        utf32_range as_utf32(s);
        if (pseudonormalized_fcd(as_utf32.begin(), as_utf32.end()))
            return;

        string temp;
        temp.reserve(s.size());

        pseudonormalize_to_fcc(
            as_utf32.begin(), as_utf32.end(), std::inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

}}

#endif
