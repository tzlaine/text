#ifndef BOOST_TEXT_COLLATE_HPP
#define BOOST_TEXT_COLLATE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/collation_fwd.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/container/small_vector.hpp>

#include <vector>

#ifndef BOOST_TEXT_COLLATE_INSTRUMENTATION
#define BOOST_TEXT_COLLATE_INSTRUMENTATION 0
#endif


namespace boost { namespace text {

    /** A collation sort key.  Consists of a sequence of 32-bit values. */
    struct text_sort_key
    {
        using const_iterator = std::vector<uint32_t>::const_iterator;
        using iterator = const_iterator;
        using value_type = uint32_t;

        text_sort_key() {}
        explicit text_sort_key(std::vector<uint32_t> bytes) :
            storage_(std::move(bytes))
        {}

        const_iterator begin() const noexcept { return storage_.begin(); }
        const_iterator end() const noexcept { return storage_.end(); }

    private:
        std::vector<uint32_t> storage_;
    };

#if BOOST_TEXT_COLLATE_INSTRUMENTATION
    inline std::ostream & operator<<(std::ostream & os, text_sort_key const & k)
    {
        os << std::hex << "[";
        for (auto x : k) {
            os << " " << x;
        }
        os << " ]" << std::dec;
        return os;
    }
#endif

    /** Returns 0 if the given sort keys are equal, a value < 0 if \a lhs is
        less than \a rhs, and a value > 0 otehrwise. */
    inline int
    compare(text_sort_key const & lhs, text_sort_key const & rhs) noexcept
    {
        auto const pair =
            algorithm::mismatch(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        if (pair.first == lhs.end()) {
            if (pair.second == rhs.end())
                return 0;
            return -1;
        } else {
            if (pair.second == rhs.end())
                return 1;
            auto const lhs_element = *pair.first;
            auto const rhs_element = *pair.second;
            if (lhs_element < rhs_element)
                return -1;
            if (rhs_element < lhs_element)
                return 1;
            return 0;
        }
    }

    // The code in this file implements the UCA as described in
    // http://www.unicode.org/reports/tr10/#Main_Algorithm .  The numbering
    // and some variable naming comes from there.
    namespace detail {

        // http://www.unicode.org/reports/tr10/#Derived_Collation_Elements
        template<typename OutIter, typename LeadByteFunc>
        inline OutIter add_derived_elements(
            uint32_t cp,
            variable_weighting weighting,
            OutIter out,
            detail::collation_trie_t const & trie,
            collation_element const * collation_elements_first,
            LeadByteFunc const & lead_byte,
            collation_strength strength,
            retain_case_bits_t retain_case_bits)
        {
            if (hangul_syllable(cp)) {
                auto cps = decompose_hangul_syllable<3>(cp);
                return s2(
                    cps.begin(),
                    cps.end(),
                    out,
                    trie,
                    collation_elements_first,
                    lead_byte,
                    strength,
                    weighting,
                    retain_case_bits);
            }

            // Core Han Unified Ideographs
            std::array<uint32_t, 12> const CJK_Compatibility_Ideographs = {
                {0xFA0E,
                 0xFA0F,
                 0xFA11,
                 0xFA13,
                 0xFA14,
                 0xFA1F,
                 0xFA21,
                 0xFA23,
                 0xFA24,
                 0xFA27,
                 0xFA28,
                 0xFA29}};

            std::array<uint32_t, 222> const CJK_Unified_Ideographs_Extension_D =
                {{0x2B740, 0x2B741, 0x2B742, 0x2B743, 0x2B744, 0x2B745, 0x2B746,
                  0x2B747, 0x2B748, 0x2B749, 0x2B74A, 0x2B74B, 0x2B74C, 0x2B74D,
                  0x2B74E, 0x2B74F, 0x2B750, 0x2B751, 0x2B752, 0x2B753, 0x2B754,
                  0x2B755, 0x2B756, 0x2B757, 0x2B758, 0x2B759, 0x2B75A, 0x2B75B,
                  0x2B75C, 0x2B75D, 0x2B75E, 0x2B75F, 0x2B760, 0x2B761, 0x2B762,
                  0x2B763, 0x2B764, 0x2B765, 0x2B766, 0x2B767, 0x2B768, 0x2B769,
                  0x2B76A, 0x2B76B, 0x2B76C, 0x2B76D, 0x2B76E, 0x2B76F, 0x2B770,
                  0x2B771, 0x2B772, 0x2B773, 0x2B774, 0x2B775, 0x2B776, 0x2B777,
                  0x2B778, 0x2B779, 0x2B77A, 0x2B77B, 0x2B77C, 0x2B77D, 0x2B77E,
                  0x2B77F, 0x2B780, 0x2B781, 0x2B782, 0x2B783, 0x2B784, 0x2B785,
                  0x2B786, 0x2B787, 0x2B788, 0x2B789, 0x2B78A, 0x2B78B, 0x2B78C,
                  0x2B78D, 0x2B78E, 0x2B78F, 0x2B790, 0x2B791, 0x2B792, 0x2B793,
                  0x2B794, 0x2B795, 0x2B796, 0x2B797, 0x2B798, 0x2B799, 0x2B79A,
                  0x2B79B, 0x2B79C, 0x2B79D, 0x2B79E, 0x2B79F, 0x2B7A0, 0x2B7A1,
                  0x2B7A2, 0x2B7A3, 0x2B7A4, 0x2B7A5, 0x2B7A6, 0x2B7A7, 0x2B7A8,
                  0x2B7A9, 0x2B7AA, 0x2B7AB, 0x2B7AC, 0x2B7AD, 0x2B7AE, 0x2B7AF,
                  0x2B7B0, 0x2B7B1, 0x2B7B2, 0x2B7B3, 0x2B7B4, 0x2B7B5, 0x2B7B6,
                  0x2B7B7, 0x2B7B8, 0x2B7B9, 0x2B7BA, 0x2B7BB, 0x2B7BC, 0x2B7BD,
                  0x2B7BE, 0x2B7BF, 0x2B7C0, 0x2B7C1, 0x2B7C2, 0x2B7C3, 0x2B7C4,
                  0x2B7C5, 0x2B7C6, 0x2B7C7, 0x2B7C8, 0x2B7C9, 0x2B7CA, 0x2B7CB,
                  0x2B7CC, 0x2B7CD, 0x2B7CE, 0x2B7CF, 0x2B7D0, 0x2B7D1, 0x2B7D2,
                  0x2B7D3, 0x2B7D4, 0x2B7D5, 0x2B7D6, 0x2B7D7, 0x2B7D8, 0x2B7D9,
                  0x2B7DA, 0x2B7DB, 0x2B7DC, 0x2B7DD, 0x2B7DE, 0x2B7DF, 0x2B7E0,
                  0x2B7E1, 0x2B7E2, 0x2B7E3, 0x2B7E4, 0x2B7E5, 0x2B7E6, 0x2B7E7,
                  0x2B7E8, 0x2B7E9, 0x2B7EA, 0x2B7EB, 0x2B7EC, 0x2B7ED, 0x2B7EE,
                  0x2B7EF, 0x2B7F0, 0x2B7F1, 0x2B7F2, 0x2B7F3, 0x2B7F4, 0x2B7F5,
                  0x2B7F6, 0x2B7F7, 0x2B7F8, 0x2B7F9, 0x2B7FA, 0x2B7FB, 0x2B7FC,
                  0x2B7FD, 0x2B7FE, 0x2B7FF, 0x2B800, 0x2B801, 0x2B802, 0x2B803,
                  0x2B804, 0x2B805, 0x2B806, 0x2B807, 0x2B808, 0x2B809, 0x2B80A,
                  0x2B80B, 0x2B80C, 0x2B80D, 0x2B80E, 0x2B80F, 0x2B810, 0x2B811,
                  0x2B812, 0x2B813, 0x2B814, 0x2B815, 0x2B816, 0x2B817, 0x2B818,
                  0x2B819, 0x2B81A, 0x2B81B, 0x2B81C, 0x2B81D}};

            double const spacing = implicit_weights_spacing_times_ten / 10.0;

            for (auto seg : g_implicit_weights_segments) {
                if (seg.first_ <= cp && cp < seg.last_) {
                    if (seg.first_ == CJK_Compatibility_Ideographs[0] &&
                        ((cp & ~OR_CJK_Compatibility_Ideographs) ||
                         !std::count(
                             CJK_Compatibility_Ideographs.begin(),
                             CJK_Compatibility_Ideographs.end(),
                             cp))) {
                        continue;
                    }

                    if (seg.first_ == CJK_Unified_Ideographs_Extension_D[0] &&
                        ((cp & ~OR_CJK_Unified_Ideographs_Extension_D) ||
                         !std::binary_search(
                             CJK_Unified_Ideographs_Extension_D.begin(),
                             CJK_Unified_Ideographs_Extension_D.end(),
                             cp))) {
                        continue;
                    }

                    uint32_t const primary_weight_low_bits =
                        seg.primary_offset_ + (cp - seg.first_) * spacing;
                    assert(
                        (primary_weight_low_bits & 0xfffff) ==
                        primary_weight_low_bits);
                    uint32_t const bytes[4] = {
                        implicit_weights_first_lead_byte,
                        ((primary_weight_low_bits >> 12) & 0xfe) | 0x1,
                        ((primary_weight_low_bits >> 5) & 0xfe) | 0x1,
                        (primary_weight_low_bits >> 0) & 0x3f};
                    uint32_t const primary = bytes[0] << 24 | bytes[1] << 16 |
                                             bytes[2] << 8 | bytes[3] << 0;
                    collation_element ce{primary, 0x0500, 0x0500, 0x0};

                    ce.l1_ = replace_lead_byte(ce.l1_, lead_byte(ce));

                    *out++ = ce;
                    return out;
                }
            }

            // This is not tailorable, so we won't use lead_byte here.
            *out++ = collation_element{
                (implicit_weights_final_lead_byte << 24) | (cp & 0xffffff),
                0x0500,
                0x0500,
                0x0};
            return out;
        }

        inline bool variable(collation_element ce) noexcept
        {
            auto const lo = min_variable_collation_weight;
            auto const hi = max_variable_collation_weight;
            return lo <= ce.l1_ && ce.l1_ <= hi;
        }

        inline bool ignorable(collation_element ce) noexcept
        {
            return ce.l1_ == 0;
        }

        // http://www.unicode.org/reports/tr10/#Variable_Weighting
        template<typename CEIter>
        inline bool s2_3(
            CEIter first,
            CEIter last,
            collation_strength strength,
            variable_weighting weighting,
            bool after_variable,
            retain_case_bits_t retain_case_bits)
        {
            if (retain_case_bits == retain_case_bits_t::no) {
                for (auto it = first; it != last; ++it) {
                    auto & ce = *it;
                    // The top two bits in each byte in FractionalUCA.txt's L3
                    // weights are for the case level.
                    // http://www.unicode.org/reports/tr35/tr35-collation.html#File_Format_FractionalUCA_txt
                    uint16_t const l3 = ce.l3_ & disable_case_level_mask;

                    ce.l3_ = l3;
                }
            }

            if (weighting == variable_weighting::non_ignorable)
                return after_variable;

            if (strength == collation_strength::primary)
                return after_variable;

            // http://www.unicode.org/reports/tr10/#Implicit_Weights says: "If
            // a fourth or higher weights are used, then the same pattern is
            // followed for those weights. They are set to a non-zero value in
            // the first collation element and zero in the second."
            //
            // Even though this appears in the section on implicit weights
            // that "do not have explicit mappings in the DUCET", this
            // apparently applies to any pair of collation elements that
            // matches the pattern produced by the derived weight algorithm,
            // since that's what CollationTest_SHIFTED.txt expects.
            bool second_of_implicit_weight_pair = false;

            for (auto it = first; it != last; ++it) {
                auto & ce = *it;
                if (after_variable && ignorable(ce)) {
                    ce.l1_ = 0;
                    ce.l2_ = 0;
                    ce.l3_ = 0;
                    ce.l4_ = 0;
                } else if (!ce.l1_) {
                    if (!ce.l2_ && !ce.l3_) {
                        ce.l4_ = 0x0000;
                    } else if (ce.l3_) {
                        if (after_variable)
                            ce.l4_ = 0x0000;
                        else
                            ce.l4_ = 0xffffffff;
                    }
                    after_variable = false;
                } else if (variable(ce)) {
                    ce.l4_ = ce.l1_;
                    ce.l1_ = 0;
                    ce.l2_ = 0;
                    ce.l3_ = 0;
                    after_variable = true;
                } else {
                    if (ce.l1_)
                        ce.l4_ = 0xffffffff;
                    after_variable = false;
                }
                if (second_of_implicit_weight_pair) {
                    ce.l4_ = 0;
                    second_of_implicit_weight_pair = false;
                }
#if 0
                 // Not necessary with FractionalUCA.txt-derived data, in
                 // which each implicit weight before the unassigned code
                 // points is only one CE.
                second_of_implicit_weight_pair =
                    implicit_weights_first_lead_byte <= l1 &&
                    l1 <= implicit_weights_final_lead_byte;
#endif
            }

            return after_variable;
        }

        template<
            typename CPIter,
            typename CPOutIter,
            typename LeadByteFunc,
            typename SizeOutIter = std::ptrdiff_t *>
        auto
        s2(CPIter first,
           CPIter last,
           CPOutIter out,
           detail::collation_trie_t const & trie,
           collation_element const * collation_elements_first,
           LeadByteFunc const & lead_byte,
           collation_strength strength,
           variable_weighting weighting,
           retain_case_bits_t retain_case_bits,
           SizeOutIter * size_out = nullptr)
            -> detail::cp_iter_ret_t<CPOutIter, CPIter>
        {
            container::small_vector<collation_element, 64> ce_buffer;

            bool after_variable = false;
            while (first != last) {
                // S2.1 Find longest prefix that results in a collation trie
                // match.
                trie_match_t collation_;
                collation_ = trie.longest_match(first, last);
                if (!collation_.match) {
                    // S2.2
                    collation_element derived_ces[32];
                    auto const derived_ces_end = add_derived_elements(
                        *first++,
                        weighting,
                        derived_ces,
                        trie,
                        collation_elements_first,
                        lead_byte,
                        strength,
                        retain_case_bits);
                    after_variable = s2_3(
                        derived_ces,
                        derived_ces_end,
                        strength,
                        weighting,
                        after_variable,
                        retain_case_bits);
                    out = std::copy(derived_ces, derived_ces_end, out);
                    if (size_out)
                        *(*size_out)++ = 1;
                    continue;
                }
                first += collation_.size;

                // S2.1.1 Process any nonstarters following S.
                auto nonstarter_last = first;
                if (!collation_.leaf) {
                    nonstarter_last = std::find_if(
                        first, last, [](uint32_t cp) { return ccc(cp) == 0; });
                }

                // TODO: Optimize the ccc() calls.

                // S2.1.2
                auto nonstarter_first = first;
                while (!collation_.leaf &&
                       nonstarter_first != nonstarter_last &&
                       ccc(*(nonstarter_first - 1)) < ccc(*nonstarter_first)) {
                    auto const cp = *nonstarter_first;
                    auto coll = trie.extend_subsequence(collation_, cp);
                    // S2.1.3
                    if (coll.match && collation_.size < coll.size) {
                        std::copy_backward(
                            first, nonstarter_first, nonstarter_first + 1);
                        *first++ = cp;
                        collation_ = coll;
                    }
                    ++nonstarter_first;
                }

                auto const collation_it = const_trie_iterator_t(collation_);

                // S2.4
                ce_buffer.resize(collation_it->value.size());
                auto const ce_buffer_first = ce_buffer.begin();
                auto const ce_buffer_last = std::copy(
                    collation_it->value.begin(collation_elements_first),
                    collation_it->value.end(collation_elements_first),
                    ce_buffer_first);

                // S2.3
                after_variable = s2_3(
                    ce_buffer_first,
                    ce_buffer_last,
                    strength,
                    weighting,
                    after_variable,
                    retain_case_bits);

                out = std::copy(ce_buffer_first, ce_buffer_last, out);
                if (size_out)
                    *(*size_out)++ = collation_it->value.size();
            }

            return out;
        }

        using level_sort_key_values_t = container::small_vector<uint32_t, 256>;
        using level_sort_key_bytes_t = container::small_vector<uint8_t, 1024>;

        // In-place compression of values such that 8-bit byte values are
        // packed into a 32-bit dwords (e.g. 0x0000XX00, 0x0000YYZZ ->
        // 0x00XXYYZZ), based on
        // https://www.unicode.org/reports/tr10/#Reducing_Sort_Key_Lengths
        // 9.1.3.
        inline level_sort_key_bytes_t
        pack_words(level_sort_key_values_t const & values)
        {
            level_sort_key_bytes_t retval;

            // We cannot treat the inputs naively as a sequence of bytes,
            // because we don't know the endianness.
            for (auto x : values) {
                uint8_t const bytes[4] = {
                    uint8_t(x >> 24),
                    uint8_t((x >> 16) & 0xff),
                    uint8_t((x >> 8) & 0xff),
                    uint8_t(x & 0xff),
                };
                if (bytes[0])
                    retval.push_back(bytes[0]);
                if (bytes[1])
                    retval.push_back(bytes[1]);
                if (bytes[2])
                    retval.push_back(bytes[2]);
                if (bytes[3])
                    retval.push_back(bytes[3]);
            }

            return retval;
        }

        // In-place run-length encoding, based on
        // https://www.unicode.org/reports/tr10/#Reducing_Sort_Key_Lengths
        // 9.1.4.
        inline void
        rle(level_sort_key_bytes_t & bytes,
            uint8_t min_,
            uint8_t common,
            uint8_t max_)
        {
            uint8_t const min_top = (common - 1) - (min_ - 1);
            uint8_t const max_bottom = (common + 1) + (0xff - max_);
            int const bound = (min_top + max_bottom) / 2;

            auto it = bytes.begin();
            auto const end = bytes.end();
            auto out = bytes.begin();
            while (it != end) {
                if (*it == common) {
                    auto const last_common = find_not(it, end, common);
                    auto const size = last_common - it;
                    if (last_common == end || *last_common < common) {
                        int const synthetic_low = min_top + size;
                        if (bound <= synthetic_low) {
                            auto const max_compressible_copies =
                                (bound - 1) - min_top;
                            auto const repetitions =
                                size / max_compressible_copies;
                            out = std::fill_n(out, repetitions, bound - 1);
                            auto const remainder =
                                size % max_compressible_copies;
                            if (remainder)
                                *out++ = min_top + remainder;
                        } else {
                            *out++ = synthetic_low;
                        }
                    } else {
                        int const synthetic_high = max_bottom - size;
                        if (synthetic_high < bound) {
                            auto const max_compressible_copies =
                                max_bottom - bound;
                            auto const repetitions =
                                size / max_compressible_copies;
                            out = std::fill_n(out, repetitions, bound);
                            auto const remainder =
                                size % max_compressible_copies;
                            if (remainder)
                                *out++ = max_bottom - remainder;
                        } else {
                            *out++ = synthetic_high;
                        }
                    }
                    it = last_common;
                } else {
                    if (min_ <= *it && *it < common)
                        *out = *it - (min_ - 1);
                    else if (common < *it && *it <= max_)
                        *out = *it + 0xff - max_;
                    else
                        *out = *it;
                    ++it;
                    ++out;
                }
            }

            bytes.erase(out, end);
        }

        inline void pad_words(level_sort_key_bytes_t & bytes)
        {
            int remainder = bytes.size() % 4;
            if (remainder) {
                remainder = 4 - remainder;
                bytes.resize(bytes.size() + remainder, 0);
            }
        }

        inline void level_bytes_to_values(
            level_sort_key_bytes_t const & bytes,
            level_sort_key_values_t & values)
        {
            assert(bytes.size() % 4 == 0);

            values.resize(bytes.size() / 4);

            auto out = values.begin();
            for (auto it = bytes.begin(), end = bytes.end(); it != end;
                 it += 4) {
                uint32_t const x = *(it + 0) << 24 | *(it + 1) << 16 |
                                   *(it + 2) << 8 | *(it + 3) << 0;
                *out++ = x;
            }
        }

        // https://www.unicode.org/reports/tr35/tr35-collation.html#Case_Weights
        inline collation_element modify_for_case(
            collation_element ce,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl) noexcept
        {
            if (case_1st == case_first::off && case_lvl == case_level::off) {
                ce.l3_ &= disable_case_level_mask;
                return ce;
            }

            uint16_t c = 0; // Set 1, 2, or 3 below.
            auto const case_bits = ce.l3_ & case_level_bits_mask;

            if (case_1st == case_first::upper) {
                c = (case_bits == upper_case_bits)
                        ? 1
                        : ((case_bits == mixed_case_bits) ? 2 : 3);
            } else {
                c = (case_bits == upper_case_bits)
                        ? 3
                        : ((case_bits == mixed_case_bits) ? 2 : 1);
            }

            if (case_lvl == case_level::on) {
                if (strength == collation_strength::primary) {
                    // Ensure we use values >= min_secondary_byte.
                    c += min_secondary_byte - 1;
                    if (!ce.l1_)
                        ce.l2_ = 0;
                    else
                        ce.l2_ = c << 8; // Shift bits into lead L2 byte.
                    ce.l3_ = 0;
                } else {
                    ce.l4_ = ce.l3_ & disable_case_level_mask;
                    // Ensure we use values >= min_tertiary_byte.
                    c += min_tertiary_byte - 1;
                    if (!ce.l1_ && !ce.l2_)
                        ce.l3_ = 0;
                    else
                        ce.l3_ = c << 8; // Shift into L3 lead byte.
                }
            } else {
                ce.l3_ &= disable_case_level_mask;
                if (ce.l2_)
                    ce.l3_ |= c << 14; // Shift into high 2 bits of L3.
                else if (ce.l3_)
                    ce.l3_ |= 3 << 14; // Shift into high 2 bits of L3.
            }

            return ce;
        }

        template<typename CEIter, typename CPIter, typename Container>
        auto
        s3(CEIter ces_first,
           CEIter ces_last,
           int ces_size,
           collation_strength strength,
           case_first case_1st,
           case_level case_lvl,
           l2_weight_order l2_order,
           CPIter cps_first,
           CPIter cps_last,
           int cps_size,
           Container & bytes) -> detail::cp_iter_ret_t<void, CPIter>
        {
            level_sort_key_values_t l1;
            level_sort_key_values_t l2;
            level_sort_key_values_t l3;
            level_sort_key_values_t l4;
            // For when case level bumps L4.
            level_sort_key_values_t l4_overflow;

            auto const strength_for_copies =
                case_lvl == case_level::on
                    ? collation_strength(static_cast<int>(strength) + 1)
                    : strength;
            for (; ces_first != ces_last; ++ces_first) {
                auto ce = *ces_first;
                ce = modify_for_case(ce, strength, case_1st, case_lvl);
                if (ce.l1_)
                    l1.push_back(ce.l1_);
                if (collation_strength::secondary <= strength_for_copies) {
                    if (ce.l2_)
                        l2.push_back(ce.l2_);
                    if (collation_strength::tertiary <= strength_for_copies) {
                        if (ce.l3_)
                            l3.push_back(ce.l3_);
                        if (collation_strength::quaternary <=
                            strength_for_copies) {
                            if (ce.l4_) {
                                l4.push_back(ce.l4_);
                                if (ces_first->l4_)
                                    l4_overflow.push_back(ces_first->l4_);
                            }
                        }
                    }
                }
            }

            if (l1.empty() && l2.empty() && l3.empty() && l4.empty() &&
                l4_overflow.empty()) {
                return;
            }

            if (!l2.empty()) {
                if (l2_order == l2_weight_order::backward)
                    std::reverse(l2.begin(), l2.end());
                auto packed_l2 = pack_words(l2);
                rle(packed_l2,
                    min_secondary_byte,
                    common_secondary_byte,
                    max_secondary_byte);
                pad_words(packed_l2);
                level_bytes_to_values(packed_l2, l2);
                if (!l3.empty()) {
                    auto packed_l3 = pack_words(l3);
                    rle(packed_l3,
                        min_tertiary_byte,
                        common_tertiary_byte,
                        max_tertiary_byte);
                    pad_words(packed_l3);
                    level_bytes_to_values(packed_l3, l3);
                }
            }

            int const separators = static_cast<int>(strength_for_copies);

            level_sort_key_values_t nfd;
            if (collation_strength::quaternary < strength)
                normalize_to_nfd(cps_first, cps_last, std::back_inserter(nfd));

            int size = l1.size();
            if (collation_strength::primary < strength_for_copies) {
                size += l2.size();
                if (collation_strength::secondary < strength_for_copies) {
                    size += l3.size();
                    if (collation_strength::tertiary < strength_for_copies) {
                        size += l4.size();
                        if (!l4_overflow.empty()) {
                            ++size;
                            size += l4_overflow.size();
                        }
                        if (collation_strength::quaternary <
                            strength_for_copies)
                            size += nfd.size();
                    }
                }
            }
            size += separators;

            bytes.resize(bytes.size() + size);

            auto it = bytes.end() - size;
            it = std::copy(l1.begin(), l1.end(), it);
            if (collation_strength::primary < strength_for_copies) {
                *it++ = 0x0000;
                it = std::copy(l2.begin(), l2.end(), it);
                if (collation_strength::secondary < strength_for_copies) {
                    *it++ = 0x0000;
                    it = std::copy(l3.begin(), l3.end(), it);
                    if (collation_strength::tertiary < strength_for_copies) {
                        *it++ = 0x0000;
                        it = std::copy(l4.begin(), l4.end(), it);
                        if (!l4_overflow.empty()) {
                            *it++ = 0x0000;
                            it = std::copy(
                                l4_overflow.begin(), l4_overflow.end(), it);
                        }
                        if (collation_strength::quaternary <
                            strength_for_copies) {
                            *it++ = 0x0000;
                            it = std::copy(nfd.begin(), nfd.end(), it);
                        }
                    }
                }
            }
            assert(it == bytes.end());
        }

        template<typename CPIter>
        auto collation_sort_key(
            CPIter first,
            CPIter last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table)
            -> detail::cp_iter_ret_t<text_sort_key, CPIter>;

        template<typename CPIter1, typename CPIter2>
        auto collate(
            CPIter1 lhs_first,
            CPIter1 lhs_last,
            CPIter2 rhs_first,
            CPIter2 rhs_last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table)
            -> detail::cp_iter_ret_t<int, CPIter1>
        {
            text_sort_key const lhs_sk = collation_sort_key(
                lhs_first,
                lhs_last,
                strength,
                case_1st,
                case_lvl,
                weighting,
                l2_order,
                table);
            text_sort_key const rhs_sk = collation_sort_key(
                rhs_first,
                rhs_last,
                strength,
                case_1st,
                case_lvl,
                weighting,
                l2_order,
                table);
            return compare(lhs_sk, rhs_sk);
        }
    }

    // TODO: Versions of the functions below that do not assume FCC?

    // TODO: Sentinels!

    /** Returns a collation sort key for the code points in <code>[first,
        last)</code>, using the given collation table.  Any optional settings
        such as case_first will be honored, so long as they do not conlfict
        with the settings on the given table. */
    template<typename CPIter>
    auto collation_sort_key(
        CPIter first,
        CPIter last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward)
        -> detail::cp_iter_ret_t<text_sort_key, CPIter>
    {
        return detail::collation_sort_key(
            first,
            last,
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order,
            table);
    }

    /** Returns a collation sort key for the code points in \a r, using the
        given collation table.  Any optional settings such as case_first will
        be honored, so long as they do not conlfict with the settings on the
        given table. */
    template<typename CPRange>
    text_sort_key collation_sort_key(
        CPRange const & r,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        using std::begin;
        using std::end;
        return collation_sort_key(
            begin(r),
            end(r),
            table,
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order);
    }


    /** Returns the result of calling compare() on collation sort keys
        produced using <code>[lhs_first, lhs_last)</code> and
        <code>[rhs_first, rhs_last)</code>, respectively.  Any optional
        settings such as case_first will be honored, so long as they do not
        conlfict with the settings on the given table. */
    template<typename CPIter1, typename CPIter2>
    auto collate(
        CPIter1 lhs_first,
        CPIter1 lhs_last,
        CPIter2 rhs_first,
        CPIter2 rhs_last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward)
        -> detail::cp_iter_ret_t<int, CPIter1>
    {
        return detail::collate(
            lhs_first,
            lhs_last,
            rhs_first,
            rhs_last,
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order,
            table);
    }

    /** Returns the result of calling compare() on collation sort keys
        produced using \a r1 and \a r2, respectively.  Any optional settings
        such as case_first will be honored, so long as they do not conlfict
        with the settings on the given table. */
    template<typename CPRange1, typename CPRange2>
    int collate(
        CPRange1 const & r1,
        CPRange2 const & r2,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        using std::begin;
        using std::end;
        return collate(
            begin(r1),
            end(r1),
            begin(r2),
            end(r2),
            table,
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order);
    }

}}

#include <boost/text/collation_table.hpp>

namespace boost { namespace text { namespace detail {

    template<typename CPIter>
    auto collation_sort_key(
        CPIter first,
        CPIter last,
        collation_strength strength,
        case_first case_1st,
        case_level case_lvl,
        variable_weighting weighting,
        l2_weight_order l2_order,
        collation_table const & table)
        -> detail::cp_iter_ret_t<text_sort_key, CPIter>
    {
        auto const initial_first = first;

        if (table.l2_order())
            l2_order = *table.l2_order();
        if (table.weighting())
            weighting = *table.weighting();
        if (table.case_1st())
            case_1st = *table.case_1st();
        if (table.case_lvl())
            case_lvl = *table.case_lvl();

        container::small_vector<collation_element, 1024> ces;
        std::array<uint32_t, 256> buffer;
        auto buf_it = buffer.begin();
        auto it = first;
        int cps = 0;
        while (it != last) {
            for (; it != last && buf_it != buffer.end();
                 ++buf_it, ++it, ++cps) {
                *buf_it = *it;
            }

            // The chunk we pass to S2 should end at the earliest
            // contiguous starter (ccc == 0) we find searching backward
            // from the end.  This is because 1) we don't want to cut off
            // trailing combining characters that may participate in
            // longest-match determination in S2.1, and 2) in S2.3 we need
            // to know if earlier CPs are variable-weighted or not.
            auto s2_it = buf_it;
            if (s2_it == buffer.end()) {
                while (s2_it != buffer.begin()) {
                    if (ccc(*--s2_it))
                        break;
                }
                while (s2_it != buffer.begin()) {
                    if (!ccc(*--s2_it))
                        break;
                }
                ++s2_it;
            }

            auto const end_of_raw_input = std::prev(it, s2_it - buf_it);
            table.copy_collation_elements(
                buffer.begin(),
                s2_it,
                std::back_inserter(ces),
                strength,
                case_1st,
                case_lvl,
                weighting);
            buf_it = std::copy(s2_it, buf_it, buffer.begin());
            first = end_of_raw_input;
        }

        std::vector<uint32_t> bytes;
        s3(ces.begin(),
           ces.end(),
           ces.size(),
           strength,
           case_1st,
           case_lvl,
           l2_order,
           initial_first,
           last,
           cps,
           bytes);

        return text_sort_key(std::move(bytes));
    }

}}}

#endif
