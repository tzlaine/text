#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/normalization_data.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>

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

        template<typename DecomposeFunc>
        void normalize_to_decomposed(string & s, DecomposeFunc && decompose)
        {
            string temp;
            temp.reserve(s.size());
            utf32_range as_utf32(s);

            container::static_vector<uint32_t, 64> buffer;
            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_decomposed_impl(
                as_utf32.begin(),
                as_utf32.end(),
                buffer,
                decompose,
                [&temp](buffer_iterator first, buffer_iterator last) {
                    temp.insert(
                        temp.size(),
                        utf8::make_from_utf32_iterator(first),
                        utf8::make_from_utf32_iterator(last));
                    return true;
                });

            if (temp.size() <= s.capacity())
                s = temp;
            else
                s.swap(temp);
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

        template<std::size_t Capacity>
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
                         (prev_ccc != 0 && prev_ccc < ccc)) &&
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
                            ccc_it =
                                cccs.begin() + (starter_it - buffer.begin());
                        }
                    }
                }
            }
        }

        template<std::size_t Capacity, typename FlushFunc>
        bool compose_and_flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc &&
                flush) noexcept(noexcept(flush(buffer.begin(), buffer.end())))
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                order_canonically(buffer.begin(), buffer.end(), cccs);
            if (2 <= buffer.size())
                compose(buffer, cccs);
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
                    if (!detail::compose_and_flush_buffer(buffer, flush))
                        return false;
                    buffer.insert(buffer.end(), it, decomp.end());
                } else {
                    buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                }
                ++first;
            }
            if (!detail::compose_and_flush_buffer(buffer, flush))
                return false;
            return true;
        }

        template<typename DecomposeFunc, typename QuickCheckFunc>
        void normalize_to_composed(
            string & s,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_)
        {
            // TODO: Experiment with using normalized_quick_check() here as an
            // optimization.

            string temp;
            temp.reserve(s.size());
            utf32_range as_utf32(s);
            container::static_vector<uint32_t, 64> buffer;

            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_composed_impl(
                as_utf32.begin(),
                as_utf32.end(),
                buffer,
                decompose,
                quick_check_,
                [&temp](buffer_iterator first, buffer_iterator last) {
                    temp.insert(
                        temp.size(),
                        utf8::make_from_utf32_iterator(first),
                        utf8::make_from_utf32_iterator(last));
                    return true;
                });

            if (temp.size() <= s.capacity())
                s = temp;
            else
                s.swap(temp);
        }

        template<typename Iter, typename QuickCheckFunc>
        quick_check normalized_quick_check(
            Iter first, Iter last, QuickCheckFunc && quick_check_) noexcept
        {
            bool need_full_check = false;
            while (first != last) {
                auto const check = quick_check_(*first);
                if (check == quick_check::no)
                    return quick_check::no;
                if (check == quick_check::maybe)
                    need_full_check = true;
                ++first;
            }
            if (need_full_check)
                return quick_check::maybe;
            return quick_check::yes;
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
                    quick_check_,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                            return true;
                        }
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
                return normalize_to_composed_impl(
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
                            return true;
                        }
                    });
            }
            return check == quick_check::yes;
        }
    }

    /** TODO */
    inline void normalize_to_nfd(string & s)
    {
        detail::normalize_to_decomposed(
            s, [](uint32_t cp) { return canonical_decompose(cp); });
    }

    /** TODO */
    inline void normalize_to_nfkd(string & s)
    {
        detail::normalize_to_decomposed(
            s, [](uint32_t cp) { return compatible_decompose(cp); });
    }

    /** TODO */
    inline void normalize_to_nfc(string & s)
    {
        detail::normalize_to_composed(
            s,
            [](uint32_t cp) { return canonical_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfc_code_point(cp); });
    }

    /** TODO */
    inline void normalize_to_nfkc(string & s)
    {
        detail::normalize_to_composed(
            s,
            [](uint32_t cp) { return compatible_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfkc_code_point(cp); });
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
            [](uint32_t cp) { return canonical_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfd_code_point(cp); });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFKD. */
    template<typename Iter>
    bool normalized_nfkd(Iter first, Iter last) noexcept
    {
        return detail::normalized_decomposed(
            first,
            last,
            [](uint32_t cp) { return compatible_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfkd_code_point(cp); });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFC. */
    template<typename Iter>
    bool normalized_nfc(Iter first, Iter last) noexcept
    {
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return canonical_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfc_code_point(cp); });
    }

    /** Returns true iff the given sequence of code points is normalized
        NFKC. */
    template<typename Iter>
    bool normalized_nfkc(Iter first, Iter last) noexcept
    {
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return compatible_decompose(cp); },
            [](uint32_t cp) { return quick_check_nfkc_code_point(cp); });
    }

}}

#endif
