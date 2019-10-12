#ifndef BOOST_TEXT_COLLATE_HPP
#define BOOST_TEXT_COLLATE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/collation_fwd.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/normalize_string.hpp>
#include <boost/text/string.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/container/small_vector.hpp>

#include <vector>

#ifndef BOOST_TEXT_DOXYGEN

#ifndef BOOST_TEXT_COLLATE_INSTRUMENTATION
#define BOOST_TEXT_COLLATE_INSTRUMENTATION 0
#endif

#endif


namespace boost { namespace text { inline namespace v1 {

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

        std::size_t size() const noexcept { return storage_.size(); }

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

    /** Returns 0 if the given sort keys are equal, a value < 0 if `lhs` is
        less than `rhs`, and a value > 0 otherwise. */
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

    inline bool operator==(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    inline bool operator!=(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return !(lhs == rhs);
    }

    inline bool operator<(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    inline bool operator<=(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return boost::text::v1::compare(lhs, rhs) < 0;
    }

    inline bool operator>(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return std::lexicographical_compare(
            rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
    }

    inline bool operator>=(text_sort_key const & lhs, text_sort_key const & rhs)
    {
        return 0 < compare(lhs, rhs);
    }

    // The code in this file implements the UCA as described in
    // http://www.unicode.org/reports/tr10/#Main_Algorithm .  The numbering
    // and some variable naming comes from there.
    namespace detail {

        // http://www.unicode.org/reports/tr10/#Derived_Collation_Elements
        template<typename OutIter, typename LeadByteFunc>
        inline OutIter add_derived_elements(
            uint32_t cp,
            OutIter out,
            detail::collation_trie_t const & trie,
            collation_element const * collation_elements_first,
            LeadByteFunc const & lead_byte)
        {
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

            for (auto seg : make_implicit_weights_segments()) {
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
                    BOOST_ASSERT(
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

                    ce.l1_ = detail::replace_lead_byte(ce.l1_, lead_byte(ce));

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

        inline bool ignorable(collation_element ce) noexcept
        {
            return ce.l1_ == 0;
        }

        template<typename CEIter>
        inline void s2_3_case_bits(CEIter first, CEIter last)
        {
            for (auto it = first; it != last; ++it) {
                auto & ce = *it;
                // The top two bits in each byte in FractionalUCA.txt's L3
                // weights are for the case level.
                // http://www.unicode.org/reports/tr35/tr35-collation.html#File_Format_FractionalUCA_txt
                uint16_t const l3 = ce.l3_ & disable_case_level_mask;

                ce.l3_ = l3;
            }
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
                if (after_variable && detail::ignorable(ce)) {
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
                } else if (detail::variable(ce)) {
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

        inline std::array<bool, 256> const &
        get_derived_element_high_two_bytes()
        {
            static std::array<bool, 256> retval = {{}};
            for (auto seg : make_implicit_weights_segments()) {
                for (uint32_t i = (seg.first_ >> 12),
                              end = (seg.last_ >> 12) + 1;
                     i != end;
                     ++i) {
                    BOOST_ASSERT(i < 256u);
                    retval[i] = true;
                }
            }
            return retval;
        }

        template<typename CPIter>
        CPIter s2_1_1(CPIter first, CPIter last, trie_match_t collation)
        {
            // S2.1.1 Process any nonstarters following S.
            auto retval = first;
            if (!collation.leaf) {
                retval = std::find_if(first, last, [](uint32_t cp) {
                    return detail::ccc(cp) == 0;
                });
            }
            return retval;
        }

        template<typename CPIter>
        trie_match_t s2_1_2(
            CPIter & first,
            CPIter nonstarter_last,
            trie_match_t collation,
            detail::collation_trie_t const & trie,
            bool primaries_only = false)
        {
            // S2.1.2
            auto nonstarter_first = first;
            while (!collation.leaf && nonstarter_first != nonstarter_last &&
                   detail::ccc(*(nonstarter_first - 1)) <
                       detail::ccc(*nonstarter_first)) {
                auto const cp = *nonstarter_first;
                auto coll = trie.extend_subsequence(collation, cp);
                // S2.1.3
                if (coll.match && collation.size < coll.size) {
                    std::copy_backward(
                        first, nonstarter_first, nonstarter_first + 1);
                    *first++ = cp;
                    collation = coll;
                } else if (primaries_only) {
                    ++first;
                }
                ++nonstarter_first;
            }

            return collation;
        }

        template<typename CPIter>
        trie_match_t s2_1(
            CPIter & first,
            CPIter last,
            trie_match_t collation,
            detail::collation_trie_t const & trie)
        {
            auto nonstarter_last = s2_1_1(first, last, collation);
            return s2_1_2(first, nonstarter_last, collation, trie);
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
            std::array<bool, 256> const & derived_element_high_two_bytes =
                get_derived_element_high_two_bytes();

            bool after_variable = false;
            while (first != last) {
                // S2.1 Find longest prefix that results in a collation trie
                // match.
                trie_match_t collation_;
                collation_ = trie.longest_match(first, last);
                if (!collation_.match) {
                    // S2.2
                    uint32_t cp = *first++;
                    if (detail::hangul_syllable(cp)) {
                        auto cps = detail::decompose_hangul_syllable<3>(cp);
                        out =
                            s2(cps.begin(),
                               cps.end(),
                               out,
                               trie,
                               collation_elements_first,
                               lead_byte,
                               strength,
                               weighting,
                               retain_case_bits);
                        if (size_out)
                            *(*size_out)++ = 1;
                        continue;
                    }

                    if (!derived_element_high_two_bytes[cp >> 12]) {
                        // This is not tailorable, so we won't use lead_byte
                        // here.
                        *out++ = collation_element{
                            (implicit_weights_final_lead_byte << 24) |
                                (cp & 0xffffff),
                            0x0500,
                            0x0500,
                            0x0};
                        if (size_out)
                            *(*size_out)++ = 1;
                        continue;
                    }

                    collation_element derived_ces[32];
                    auto const derived_ces_end = detail::add_derived_elements(
                        cp,
                        derived_ces,
                        trie,
                        collation_elements_first,
                        lead_byte);
                    if (weighting == variable_weighting::shifted &&
                        strength != collation_strength::primary) {
                        after_variable = detail::s2_3(
                            derived_ces,
                            derived_ces_end,
                            strength,
                            weighting,
                            after_variable,
                            retain_case_bits);
                    }
                    out = std::copy(derived_ces, derived_ces_end, out);
                    if (size_out)
                        *(*size_out)++ = 1;
                    continue;
                }

                {
                    auto it_16 = boost::text::v1::as_utf16(first, last).begin();
                    auto s = collation_.size;
                    int cps = 0;
                    while (s) {
                        if (boost::text::v1::high_surrogate(*it_16)) {
                            s -= 2;
                            std::advance(it_16, 2);
                        } else {
                            --s;
                            --it_16;
                        }
                        ++cps;
                    }
                    first += cps;
                }

                // S2.1
                collation_ = s2_1(first, last, collation_, trie);

                auto const & collation_value = *trie[collation_];

                // S2.4
                auto const initial_out = out;
                out = std::copy(
                    collation_value.begin(collation_elements_first),
                    collation_value.end(collation_elements_first),
                    out);

                // S2.3
                if (retain_case_bits == retain_case_bits_t::no &&
                    collation_strength::tertiary <= strength) {
                    s2_3_case_bits(initial_out, out);
                }
                if (weighting != variable_weighting::non_ignorable &&
                    strength != collation_strength::primary) {
                    after_variable = detail::s2_3(
                        initial_out,
                        out,
                        strength,
                        weighting,
                        after_variable,
                        retain_case_bits);
                }

                if (size_out) {
                    *(*size_out)++ = collation_value.size();
                    for (int i = 1; i < collation_.size; ++i) {
                        *(*size_out)++ = 0;
                    }
                }
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
                    auto const last_common =
                        boost::text::v1::find_not(it, end, common);
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
            BOOST_ASSERT(bytes.size() % 4 == 0);

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

        template<
            typename CEIter,
            typename CPIter,
            typename Sentinel,
            typename Container>
        auto
        s3(CEIter ces_first,
           CEIter ces_last,
           int ces_size,
           collation_strength strength,
           case_first case_1st,
           case_level case_lvl,
           l2_weight_order l2_order,
           CPIter cps_first,
           Sentinel cps_last,
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
                ce = detail::modify_for_case(ce, strength, case_1st, case_lvl);
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
                detail::rle(packed_l2,
                    min_secondary_byte,
                    common_secondary_byte,
                    max_secondary_byte);
                detail::pad_words(packed_l2);
                detail::level_bytes_to_values(packed_l2, l2);
                if (!l3.empty()) {
                    auto packed_l3 = detail::pack_words(l3);
                    detail::rle(packed_l3,
                        min_tertiary_byte,
                        common_tertiary_byte,
                        max_tertiary_byte);
                    detail::pad_words(packed_l3);
                    detail::level_bytes_to_values(packed_l3, l3);
                }
            }

            int const separators = static_cast<int>(strength_for_copies);

            level_sort_key_values_t nfd;
            if (collation_strength::quaternary < strength) {
                boost::text::v1::normalize_to_nfd(
                    cps_first, cps_last, std::back_inserter(nfd));
            }

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
            BOOST_ASSERT(it == bytes.end());
        }

        template<typename CPIter, typename Sentinel>
        auto collation_sort_key(
            CPIter first,
            Sentinel last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table)
            -> detail::cp_iter_ret_t<text_sort_key, CPIter>;

        template<typename Result, typename Iter>
        auto make_iterator(Result first, Iter it, null_sentinel s)
            -> decltype(Result(first.base(), it, s))
        {
            return Result(first.base(), it, s);
        }

        template<typename Result, typename Iter>
        auto make_iterator(Result first, Iter it, Result last)
            -> decltype(Result(first.base(), it, last.base()))
        {
            return Result(first.base(), it, last.base());
        }

        template<typename Iter>
        Iter make_iterator(Iter first, Iter it, Iter last)
        {
            return it;
        }

        template<typename Iter, typename Sentinel, typename LeadByteFunc>
        auto collate_impl(
            utf8_tag,
            Iter lhs_first,
            Sentinel lhs_last,
            utf8_tag,
            Iter rhs_first,
            Sentinel rhs_last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table,
            collation_trie_t const & trie,
            collation_element const * ces_first,
            LeadByteFunc const & lead_byte);

        template<
            typename Tag1,
            typename Iter1,
            typename Sentinel1,
            typename Tag2,
            typename Iter2,
            typename Sentinel2,
            typename LeadByteFunc>
        auto collate_impl(
            Tag1,
            Iter1 lhs_first,
            Sentinel1 lhs_last,
            Tag2,
            Iter2 rhs_first,
            Sentinel2 rhs_last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table,
            collation_trie_t const & trie,
            collation_element const * ces_first,
            LeadByteFunc const & lead_byte)
        {
            auto const lhs = boost::text::v1::as_utf32(lhs_first, lhs_last);
            auto const rhs = boost::text::v1::as_utf32(rhs_first, rhs_last);
            text_sort_key const lhs_sk = detail::collation_sort_key(
                lhs.begin(),
                lhs.end(),
                strength,
                case_1st,
                case_lvl,
                weighting,
                l2_order,
                table);
            text_sort_key const rhs_sk = detail::collation_sort_key(
                rhs.begin(),
                rhs.end(),
                strength,
                case_1st,
                case_lvl,
                weighting,
                l2_order,
                table);
            return boost::text::v1::compare(lhs_sk, rhs_sk);
        }

        template<
            typename CPIter1,
            typename Sentinel1,
            typename CPIter2,
            typename Sentinel2>
        int collate(
            CPIter1 lhs_first,
            Sentinel1 lhs_last,
            CPIter2 rhs_first,
            Sentinel2 rhs_last,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table);
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns a collation sort key for the code points in `[first, last)`,
        using the given collation table.  Any optional settings such as
        `case_1st` will be honored, so long as they do not conflict with the
        settings on the given table.

        Consider using one of the overloads that takes collation_flags
        instead.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \pre `[first, last)` is normalized FCC. */
    template<typename CPIter, typename Sentinel>
    text_sort_key collation_sort_key(
        CPIter first,
        Sentinel last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward);

    /** Returns a collation sort key for the code points in `[first, last)`,
        using the given collation table.  Any optional settings flags will be
        honored, so long as they do not conflict with the settings on the
        given table.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \pre `[first, last)` is normalized FCC. */
    template<typename CPIter, typename Sentinel>
    text_sort_key collation_sort_key(
        CPIter first,
        Sentinel last,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

    /** Returns a collation sort key for the code points in `r`, using the
        given collation table.  Any optional settings flags will be honored,
        so long as they do not conflict with the settings on the given table.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.

        \pre r is normalized FCC. */
    template<typename CPRange>
    text_sort_key collation_sort_key(
        CPRange const & r,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

    /** Returns a collation sort key for the graphemes in `r`, using the given
        collation table.  Any optional settings flags will be honored, so long
        as they do not conflict with the settings on the given table.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept.

        \pre r is normalized FCC. */
    template<typename GraphemeRange>
    text_sort_key collation_sort_key(
        GraphemeRange const & r,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

    /** Creates sort keys for `[lhs_first, lhs_last)` and `[rhs_first,
        rhs_last)`, then returns the result of calling compare() on the
        keys. Any optional settings such as `case_1st` will be honored, so
        long as they do not conflict with the settings on the given table.

        Consider using one of the overloads that takes collation_flags
        instead.

        This function only participates in overload resolution if `CPIter1`
        models the CPIter concept.

        \pre `[lhs_first, lhs_last)` is normalized FCC.
        \pre `[rhs_first, rhs_last)` is normalized FCC. */
    template<
        typename CPIter1,
        typename Sentinel1,
        typename CPIter2,
        typename Sentinel2>
    int collate(
        CPIter1 lhs_first,
        Sentinel1 lhs_last,
        CPIter2 rhs_first,
        Sentinel2 rhs_last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable,
        l2_weight_order l2_order = l2_weight_order::forward);

    /** Creates sort keys for `[lhs_first, lhs_last)` and `[rhs_first,
        rhs_last)`, then returns the result of calling compare() on the keys.
        Any optional settings flags will be honored, so long as they do not
        conflict with the settings on the given table.

        This function only participates in overload resolution if `CPIter1`
        models the CPIter concept.

        \pre `[lhs_first, lhs_last)` is normalized FCC.
        \pre `[rhs_first, rhs_last)` is normalized FCC. */
    template<
        typename CPIter1,
        typename Sentinel1,
        typename CPIter2,
        typename Sentinel2>
    int collate(
        CPIter1 lhs_first,
        Sentinel1 lhs_last,
        CPIter2 rhs_first,
        Sentinel2 rhs_last,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

    /** Creates sort keys for `r1` and `r2`, then returns the result of
        calling compare() on the keys.  Any optional settings flags will be
        honored, so long as they do not conflict with the settings on the
        given table.

        This function only participates in overload resolution if `CPRange1`
        models the CPRange concept.

        \pre `r1` is normalized FCC.
        \pre `r2` is normalized FCC. */
    template<typename CPRange1, typename CPRange2>
    int collate(
        CPRange1 const & r1,
        CPRange2 const & r2,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

    /** Creates sort keys for `r1` and `r2`, then returns the result of
        calling compare() on the keys.  Any optional settings flags will be
        honored, so long as they do not conflict with the settings on the
        given table.

        This function only participates in overload resolution if
        `GraphemeRange1` models the GraphemeRange concept.

        \pre `r1` is normalized FCC.
        \pre `r2` is normalized FCC. */
    template<typename GraphemeRange1, typename GraphemeRange2>
    int collate(
        GraphemeRange1 const & r1,
        GraphemeRange2 const & r2,
        collation_table const & table,
        collation_flags flags = collation_flags::none);

#else

    template<typename CPIter, typename Sentinel>
    auto collation_sort_key(
        CPIter first,
        Sentinel last,
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

    template<typename CPIter, typename Sentinel>
    auto collation_sort_key(
        CPIter first,
        Sentinel last,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::cp_iter_ret_t<text_sort_key, CPIter>
    {
        return detail::collation_sort_key(
            first,
            last,
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags),
            table);
    }

    template<typename CPRange>
    auto collation_sort_key(
        CPRange const & r,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::cp_rng_alg_ret_t<text_sort_key, CPRange>
    {
        return detail::collation_sort_key(
            std::begin(r),
            std::end(r),
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags),
            table);
    }

    template<typename GraphemeRange>
    auto collation_sort_key(
        GraphemeRange const & r,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::graph_rng_alg_ret_t<text_sort_key, GraphemeRange>
    {
        return detail::collation_sort_key(
            std::begin(r).base(),
            std::end(r).base(),
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags),
            table);
    }

    template<
        typename CPIter1,
        typename Sentinel1,
        typename CPIter2,
        typename Sentinel2>
    auto collate(
        CPIter1 lhs_first,
        Sentinel1 lhs_last,
        CPIter2 rhs_first,
        Sentinel2 rhs_last,
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

    template<
        typename CPIter1,
        typename Sentinel1,
        typename CPIter2,
        typename Sentinel2>
    auto collate(
        CPIter1 lhs_first,
        Sentinel1 lhs_last,
        CPIter2 rhs_first,
        Sentinel2 rhs_last,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::cp_iter_ret_t<int, CPIter1>
    {
        return detail::collate(
            lhs_first,
            lhs_last,
            rhs_first,
            rhs_last,
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags),
            table);
    }

    template<typename CPRange1, typename CPRange2>
    auto collate(
        CPRange1 const & r1,
        CPRange2 const & r2,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::cp_rng_alg_ret_t<int, CPRange1>
    {
        return collate(
            std::begin(r1),
            std::end(r1),
            std::begin(r2),
            std::end(r2),
            table,
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags));
    }

    template<typename GraphemeRange1, typename GraphemeRange2>
    auto collate(
        GraphemeRange1 const & r1,
        GraphemeRange2 const & r2,
        collation_table const & table,
        collation_flags flags = collation_flags::none)
        -> detail::graph_rng_alg_ret_t<int, GraphemeRange1>
    {
        return collate(
            std::begin(r1).base(),
            std::end(r1).base(),
            std::begin(r2).base(),
            std::end(r2).base(),
            table,
            detail::to_strength(flags),
            detail::to_case_first(flags),
            detail::to_case_level(flags),
            detail::to_weighting(flags),
            detail::to_l2_order(flags));
    }

#endif

}}}

#include <boost/text/collation_table.hpp>

namespace boost { namespace text { inline namespace v1 { namespace detail {

    template<typename CPIter, typename Sentinel, std::size_t N, std::size_t M>
    CPIter get_collation_elements(
        CPIter first,
        Sentinel last,
        collation_strength strength,
        case_first case_1st,
        case_level case_lvl,
        variable_weighting weighting,
        l2_weight_order l2_order,
        collation_table const & table,
        std::array<uint32_t, N> & buffer,
        typename std::array<uint32_t, N>::iterator & buf_it,
        int & cps,
        container::small_vector<collation_element, M> & ces)
    {
        auto it = first;
        {
            auto u = detail::unpack_iterator_and_sentinel(it, last);
            auto copy_result = detail::transcode_to_32<true>(
                u.tag_, u.f_, u.l_, buffer.end() - buf_it, buf_it);
            it = detail::make_iterator(first, copy_result.iter, last);
            buf_it = copy_result.out;
        }

        // The chunk we pass to S2 should end at the earliest contiguous
        // starter (ccc == 0) we find searching backward from the end.  This
        // is because 1) we don't want to cut off trailing combining
        // characters that may participate in longest-match determination in
        // S2.1, and 2) in S2.3 we need to know if earlier CPs are
        // variable-weighted or not.
        auto s2_it = buf_it;
        if (s2_it == buffer.end()) {
            while (s2_it != buffer.begin()) {
                if (detail::ccc(*--s2_it) == 0)
                    break;
            }
            // TODO: This will produce incorrect results if std::prev(s2_it)
            // points to a CP with variable-weighted CEs.
        }

        auto const end_of_raw_input = std::prev(it, s2_it - buf_it);
        auto const ces_size = ces.size();
        ces.resize(ces_size + M);
        auto ces_end = table.copy_collation_elements(
            buffer.begin(),
            s2_it,
            ces.begin() + ces_size,
            strength,
            case_1st,
            case_lvl,
            weighting);
        ces.resize(ces_end - ces.begin());
        buf_it = std::copy(s2_it, buf_it, buffer.begin());
        first = end_of_raw_input;

        return first;
    }

    template<typename CPIter, typename Sentinel>
    auto collation_sort_key(
        CPIter first,
        Sentinel last,
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

        std::array<uint32_t, 128> buffer;
        container::small_vector<collation_element, 128 * 10> ces;
        auto buf_it = buffer.begin();
        int cps = 0;
        while (first != last) {
            first = get_collation_elements(
                first,
                last,
                strength,
                case_1st,
                case_lvl,
                weighting,
                l2_order,
                table,
                buffer,
                buf_it,
                cps,
                ces);
        }

        std::vector<uint32_t> bytes;
        detail::s3(ces.begin(),
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

    template<
        typename CPIter1,
        typename Sentinel1,
        typename CPIter2,
        typename Sentinel2>
    int collate(
        CPIter1 lhs_first,
        Sentinel1 lhs_last,
        CPIter2 rhs_first,
        Sentinel2 rhs_last,
        collation_strength strength,
        case_first case_1st,
        case_level case_lvl,
        variable_weighting weighting,
        l2_weight_order l2_order,
        collation_table const & table)
    {
        if (table.l2_order())
            l2_order = *table.l2_order();
        if (table.weighting())
            weighting = *table.weighting();
        if (table.case_1st())
            case_1st = *table.case_1st();
        if (table.case_lvl())
            case_lvl = *table.case_lvl();

        auto lhs_u = detail::unpack_iterator_and_sentinel(lhs_first, lhs_last);
        auto rhs_u = detail::unpack_iterator_and_sentinel(rhs_first, rhs_last);
        return collate_impl(
            lhs_u.tag_,
            lhs_u.f_,
            lhs_u.l_,
            rhs_u.tag_,
            rhs_u.f_,
            rhs_u.l_,
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order,
            table,
            table.data_->trie_,
            table.collation_elements_begin(),
            [&table](detail::collation_element ce) {
                return detail::lead_byte(
                    ce,
                    table.data_->nonsimple_reorders_,
                    table.data_->simple_reorders_);
            });
    }

    template<typename Iter>
    struct next_primary_result
    {
        Iter it_;
        uint32_t cp_;
        unsigned char lead_primary_;
        uint32_t derived_primary_;
        trie_match_t coll_;
    };

    template<typename Iter, typename Sentinel, typename LeadByteFunc>
    auto collate_impl(
        utf8_tag,
        Iter lhs_first,
        Sentinel lhs_last,
        utf8_tag,
        Iter rhs_first,
        Sentinel rhs_last,
        collation_strength strength,
        case_first case_1st,
        case_level case_lvl,
        variable_weighting weighting,
        l2_weight_order l2_order,
        collation_table const & table,
        collation_trie_t const & trie,
        collation_element const * ces_first,
        LeadByteFunc const & lead_byte)
    {
#define INSTRUMENT 0
#if INSTRUMENT
        std::cout << "\n\ncollate_impl():\n";
#endif

        auto lhs_it = lhs_first;
        auto rhs_it = rhs_first;

        if (l2_order == l2_weight_order::forward) {
            // This is std::ranges::mismatch(), but I can't use that yet.
            for (; lhs_it != lhs_last && rhs_it != rhs_last;
                 ++lhs_it, ++rhs_it) {
                if (*lhs_it != *rhs_it) {
                    // Back up to the start of the current CP.
                    while (lhs_it != lhs_first) {
                        --lhs_it;
                        --rhs_it;
                        if (!boost::text::v1::continuation(*lhs_it))
                            break;
                    }
                    break;
                }
            }
            if (lhs_it == lhs_last && rhs_it == rhs_last)
                return 0;
        }

        auto const lhs_identical_prefix = lhs_it;
        auto const rhs_identical_prefix = rhs_it;

        auto unshifted_primary_seq =
            [&](auto first, auto last, auto & primaries) {
                uint32_t retval = 0;
                for (auto it = first; it != last; ++it) {
                    if (it->l1_ &&
                        (weighting == variable_weighting::non_ignorable ||
                         strength == collation_strength::primary ||
                         !detail::variable(*it))) {
                        if (!retval)
                            retval = it->l1_;
                        primaries.push_back(it->l1_);
                    }
                }
                return retval;
            };

        auto unshifted_derived_primary = [&](uint32_t cp, auto & primaries) {
            collation_element ces[32];
            auto ces_end = detail::s2(
                &cp,
                &cp + 1,
                ces,
                trie,
                ces_first,
                lead_byte,
                collation_strength::primary,
                variable_weighting::non_ignorable,
                retain_case_bits_t::yes);
            return unshifted_primary_seq(ces, ces_end, primaries);
        };

        container::small_vector<uint32_t, 128> l_primaries;
        container::small_vector<uint32_t, 128> r_primaries;

        // Returns the CP that starts the primary-bearing sequence of CEs, and
        // the iterator just past the CP.
        auto next_primary = [&](Iter it, Sentinel last, auto & primaries) {
            using result_t = next_primary_result<Iter>;
            for (; it != last;) {
                unsigned char const c = *it;
                if (c < 0x80) {
                    int incr = 1;
#if BOOST_TEXT_USE_SIMD
                    if ((int)sizeof(__m128i) <= last - it) {
                        __m128i chunk = load_chars_for_sse(it);
                        int32_t const mask = _mm_movemask_epi8(chunk);
                        incr = mask == 0 ? 16 : trailing_zeros(mask);
                    }
#endif
                    for (auto const end = it + incr; it < end; ++it) {
                        uint32_t const cp = (unsigned char)*it;
                        trie_match_t coll =
                            trie.longest_subsequence((uint16_t)cp);
                        if (coll.match) {
                            auto lead_primary =
                                trie[coll]->lead_primary(weighting);
                            if (lead_primary)
                                return result_t{
                                    ++it, cp, lead_primary, 0, coll};
                        } else {
                            auto const p =
                                unshifted_derived_primary(cp, primaries);
                            if (p)
                                return result_t{++it, cp, 0, p, coll};
                        }
                    }
                } else {
                    auto next = it;
                    uint32_t cp = detail::advance(next, last);
                    trie_match_t coll = trie.longest_subsequence(cp);
                    it = next;
                    if (coll.match) {
                        auto lead_primary = trie[coll]->lead_primary(weighting);
                        if (lead_primary)
                            return result_t{it, cp, lead_primary, 0, coll};
                    } else {
                        auto const p = unshifted_derived_primary(cp, primaries);
                        if (p)
                            return result_t{it, cp, 0, p, coll};
                    }
                }
            }
            return result_t{it, 0, 0, 0, trie_match_t{}};
        };

        auto back_up_before_nonstarters = [&](Iter first,
                                              Iter & it,
                                              uint32_t & cp,
                                              unsigned char & lead_primary,
                                              auto & primaries,
                                              auto prev_primaries_size) {
            bool retval = false;
            auto prev = detail::decrement(first, it);
            while (it != first && table.nonstarter(cp)) {
                auto it2 = prev;
                auto prev2 = detail::decrement(first, prev);
                auto temp = prev2;
                auto cp2 = detail::advance(temp, it);

                // Check that moving backward one CP still includes the
                // current CP cp.
                uint32_t cps[] = {cp2, cp};
                auto const cus = boost::text::v1::as_utf16(cps);
                auto coll = trie.longest_subsequence(cps, cps + 2);
                if (coll.size != std::distance(cus.begin(), cus.end()))
                    break;

                it = it2;
                prev = prev2;
                cp = cp2;

                lead_primary = 0;
                primaries.resize(prev_primaries_size);

                retval = true;
#if INSTRUMENT
                std::cout << "    backing up one CP.\n";
#endif
            }
            return retval;
        };

        container::small_vector<uint32_t, 64> cps;
        container::small_vector<Iter, 64> cp_end_iters;

        auto get_primary =
            [&](Iter & it,
                Sentinel last,
                uint32_t cp,
                trie_match_t coll,
                bool backed_up,
                auto & primaries) {
                if (backed_up)
                    coll = trie.longest_subsequence(cp);
                if (coll.match) {
                    // S2.1
                    if (!coll.leaf) {
                        // Find the longest match, one CP at a time.
                        auto next = it;
                        cps.clear();
                        cp_end_iters.clear();
                        cps.push_back(cp);
                        cp_end_iters.push_back(next);
                        auto last_match = coll;
                        while (next != last) {
                            auto temp = next;
                            cp = detail::advance(temp, last);
                            auto extended_coll =
                                trie.extend_subsequence(coll, cp);
                            if (extended_coll == coll)
                                break;
                            next = temp;
                            coll = extended_coll;
                            if (coll.match)
                                last_match = coll;
                            cps.push_back(cp);
                            cp_end_iters.push_back(next);
                        }

                        auto const last_match_cps_size = cps.size();

                        // S2.1.1
                        while (next != last) {
                            cp = detail::advance(next, last);
                            if (detail::ccc(cp) == 0)
                                break;
                            cps.push_back(cp);
                            cp_end_iters.push_back(next);
                        }

#if INSTRUMENT
                        auto const & collation_value = *trie[coll];
                        std::cout << "coll.match, coll_value="
                                  << collation_value.first() << " "
                                  << collation_value.last()
                                  << " getting CEs for: ";
                        for (auto cp : cps) {
                            std::cout << "0x" << std::hex << cp << " ";
                        }
                        std::cout << std::dec << "\n";
#endif

                        // S2.1.2
                        auto cps_it = cps.begin() + last_match_cps_size;
                        coll =
                            detail::s2_1_2(cps_it, cps.end(), coll, trie, true);
                        it = cp_end_iters[cps_it - cps.begin() - 1];
                    }

                    auto const & collation_value = *trie[coll];
#if INSTRUMENT
                    std::cout << "final coll.match coll.size=" << coll.size
                              << " coll.value=" << collation_value.first()
                              << " " << collation_value.last() << "\n";
#endif

                    return unshifted_primary_seq(
                        collation_value.begin(ces_first),
                        collation_value.end(ces_first),
                        primaries);
                } else {
                    return unshifted_derived_primary(cp, primaries);
                }
            };

        // Look for a non-ignorable primary, or the end of each sequence.

        while (lhs_it != lhs_last || rhs_it != rhs_last) {
            auto prev_l_primaries_size = l_primaries.size();
            auto prev_r_primaries_size = r_primaries.size();
            auto l_prim = next_primary(lhs_it, lhs_last, l_primaries);
            auto r_prim = next_primary(rhs_it, rhs_last, r_primaries);

#if INSTRUMENT
            std::cout << "l_prim.cp_=" << std::hex << "0x" << l_prim.cp_
                      << std::dec << "\n";
            std::cout << "r_prim.cp_=" << std::hex << "0x" << r_prim.cp_
                      << std::dec << "\n";
#endif

            bool l_backed_up = false;
            if (table.nonstarter(l_prim.cp_)) {
#if INSTRUMENT
                std::cout << "backing up the left.\n";
#endif
                l_backed_up = back_up_before_nonstarters(
                    lhs_first,
                    l_prim.it_,
                    l_prim.cp_,
                    l_prim.lead_primary_,
                    l_primaries,
                    prev_l_primaries_size);
            }
            bool r_backed_up = false;
            if (table.nonstarter(r_prim.cp_)) {
#if INSTRUMENT
                std::cout << "backing up the right.\n";
#endif
                r_backed_up = back_up_before_nonstarters(
                    rhs_first,
                    r_prim.it_,
                    r_prim.cp_,
                    r_prim.lead_primary_,
                    r_primaries,
                    prev_r_primaries_size);
            }

            if ((l_prim.coll_.leaf || lhs_it == lhs_last) &&
                (r_prim.coll_.leaf || rhs_it == rhs_last) &&
                l_primaries.empty() && r_primaries.empty()) {
                uint32_t l_primary = l_prim.derived_primary_
                                         ? l_prim.derived_primary_ >> 24
                                         : l_prim.lead_primary_;
                uint32_t r_primary = r_prim.derived_primary_
                                         ? r_prim.derived_primary_ >> 24
                                         : r_prim.lead_primary_;
                if (l_primary && r_primary) {
                    if (l_primary < r_primary) {
#if INSTRUMENT
                        std::cout << "early return -1 (lead byte)\n";
                        std::cout << "left: "
                                  << "0x" << std::hex << l_primary << std::dec
                                  << " right: "
                                  << "0x" << std::hex << r_primary << std::dec
                                  << "\n";
#endif
                        return -1;
                    }
                    if (r_primary < l_primary) {
#if INSTRUMENT
                        std::cout << "early return 1 (lead byte)\n";
                        std::cout << "left: "
                                  << "0x" << std::hex << l_primary << std::dec
                                  << " right: "
                                  << "0x" << std::hex << r_primary << std::dec
                                  << "\n";
#endif
                        return 1;
                    }
                }
            }

#if INSTRUMENT
            {
                std::cout << "before getting primaries:\n";

                std::cout << " left: cps: " << std::hex;
                auto const l_r = v1::as_utf32(l_prim.it_, lhs_last);
                for (auto cp : l_r) {
                    std::cout << "0x" << cp << " ";
                }
                std::cout << "\n";

                std::cout << "right: cps: " << std::hex;
                auto const r_r = v1::as_utf32(r_prim.it_, rhs_last);
                for (auto cp : r_r) {
                    std::cout << "0x" << cp << " ";
                }
                std::cout << std::dec << "\n";
            }
#endif

            uint32_t l_primary = l_prim.derived_primary_;
            if (!l_primary && lhs_it != lhs_last) {
#if INSTRUMENT
                std::cout << "left get_primary()\n";
#endif
                get_primary(
                    l_prim.it_,
                    lhs_last,
                    l_prim.cp_,
                    l_prim.coll_,
                    l_backed_up,
                    l_primaries);
            }
            uint32_t r_primary = r_prim.derived_primary_;
            if (!r_primary && rhs_it != rhs_last) {
#if INSTRUMENT
                std::cout << "right get_primary()\n";
#endif
                get_primary(
                    r_prim.it_,
                    rhs_last,
                    r_prim.cp_,
                    r_prim.coll_,
                    r_backed_up,
                    r_primaries);
            }

#if INSTRUMENT
            {
                std::cout << "after getting primaries:\n";

                std::cout << " left: cps: " << std::hex;
                auto const l_r = v1::as_utf32(l_prim.it_, lhs_last);
                for (auto cp : l_r) {
                    std::cout << "0x" << cp << " ";
                }
                std::cout << "\n";

                std::cout << "right: cps: " << std::hex;
                auto const r_r = v1::as_utf32(r_prim.it_, rhs_last);
                for (auto cp : r_r) {
                    std::cout << "0x" << cp << " ";
                }
                std::cout << std::dec << "\n";
            }
#endif

            auto const mismatches = algorithm::mismatch(
                l_primaries.begin(),
                l_primaries.end(),
                r_primaries.begin(),
                r_primaries.end());

            auto const l_at_end = mismatches.first == l_primaries.end();
            auto const r_at_end = mismatches.second == r_primaries.end();
            if (!l_at_end && !r_at_end) {
                if (*mismatches.first < *mismatches.second) {
#if INSTRUMENT
                    std::cout << "early return -1\n";
                    std::cout << " left: " << std::hex;
                    for (auto p : l_primaries) {
                        std::cout << "0x" << p << " ";
                    }
                    std::cout << "\n";
                    std::cout << "right: " << std::hex;
                    for (auto p : r_primaries) {
                        std::cout << "0x" << p << " ";
                    }
                    std::cout << std::dec << "\n";
#endif
                    return -1;
                } else {
#if INSTRUMENT
                    std::cout << "early return 1\n";
                    std::cout << " left: " << std::hex;
                    for (auto p : l_primaries) {
                        std::cout << "0x" << p << " ";
                    }
                    std::cout << "\n";
                    std::cout << "right: " << std::hex;
                    for (auto p : r_primaries) {
                        std::cout << "0x" << p << " ";
                    }
                    std::cout << std::dec << "\n";
#endif
                    return 1;
                }
            } else if (l_at_end && !r_at_end && lhs_it == lhs_last) {
#if INSTRUMENT
                std::cout << "early return -1 (right at end)\n";
                std::cout << " left: " << std::hex;
                for (auto p : l_primaries) {
                    std::cout << "0x" << p << " ";
                }
                std::cout << "\n";
                std::cout << "right: " << std::hex;
                for (auto p : r_primaries) {
                    std::cout << "0x" << p << " ";
                }
                std::cout << std::dec << "\n";
#endif
                return -1;
            } else if (!l_at_end && r_at_end && rhs_it == rhs_last) {
#if INSTRUMENT
                std::cout << "early return 1 (left at end)\n";
                std::cout << " left: " << std::hex;
                for (auto p : l_primaries) {
                    std::cout << "0x" << p << " ";
                }
                std::cout << "\n";
                std::cout << "right: " << std::hex;
                for (auto p : r_primaries) {
                    std::cout << "0x" << p << " ";
                }
                std::cout << std::dec << "\n";
#endif
                return 1;
            }

            l_primaries.erase(l_primaries.begin(), mismatches.first);
            r_primaries.erase(r_primaries.begin(), mismatches.second);

            lhs_it = l_prim.it_;
            rhs_it = r_prim.it_;

            BOOST_ASSERT(boost::text::v1::starts_encoded(lhs_it, lhs_last));
            BOOST_ASSERT(boost::text::v1::starts_encoded(rhs_it, rhs_last));

#if INSTRUMENT
            std::cout << "**************** at end of loop:\n";

            std::cout << " left: cps: " << std::hex;
            auto const l_r = v1::as_utf32(lhs_it, lhs_last);
            for (auto cp : l_r) {
                std::cout << "0x" << cp << " ";
            }
            std::cout << "\n       prims: ";
            for (auto p : l_primaries) {
                std::cout << "0x" << p << " ";
            }
            std::cout << "\n";

            std::cout << "right: cps: " << std::hex;
            auto const r_r = v1::as_utf32(rhs_it, rhs_last);
            for (auto cp : r_r) {
                std::cout << "0x" << cp << " ";
            }
            std::cout << "\n       prims: ";
            for (auto p : r_primaries) {
                std::cout << "0x" << p << " ";
            }
            std::cout << std::dec << "\n";
#endif
        }

        if (strength == collation_strength::primary)
            return 0;

        auto const lhs =
            boost::text::v1::as_utf32(lhs_identical_prefix, lhs_last);
        auto const rhs =
            boost::text::v1::as_utf32(rhs_identical_prefix, rhs_last);
        text_sort_key const lhs_sk = detail::collation_sort_key(
            lhs.begin(),
            lhs.end(),
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order,
            table);
        text_sort_key const rhs_sk = detail::collation_sort_key(
            rhs.begin(),
            rhs.end(),
            strength,
            case_1st,
            case_lvl,
            weighting,
            l2_order,
            table);

        return boost::text::v1::compare(lhs_sk, rhs_sk);
    }

}}}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::v1::text_sort_key>
    {
        using argument_type = boost::text::v1::text_sort_key;
        using result_type = std::size_t;
        result_type operator()(argument_type const & key) const noexcept
        {
            return std::accumulate(
                key.begin(),
                key.end(),
                result_type(key.size()),
                boost::text::v1::detail::hash_combine_);
        }
    };
}

#endif

#endif


/*
          Start 1731: tailoring_rule_test_ar_standard_000_g6
1728/1970 Test #1728: tailoring_rule_test_af_standard_000_g6 ............***Failed    0.99 sec
          Start 1732: tailoring_rule_test_ar_standard_001_g6
1729/1970 Test #1730: tailoring_rule_test_ar_compat_000_g6 ..............***Failed    1.05 sec
          Start 1733: tailoring_rule_test_as_standard_000_g6
1730/1970 Test #1733: tailoring_rule_test_as_standard_000_g6 ............***Failed    1.08 sec
          Start 1734: tailoring_rule_test_az_search_000_g6
1731/1970 Test #1734: tailoring_rule_test_az_search_000_g6 ..............***Failed    1.43 sec
          Start 1736: tailoring_rule_test_be_standard_000_g6
1733/1970 Test #1736: tailoring_rule_test_be_standard_000_g6 ............***Failed    1.01 sec
          Start 1738: tailoring_rule_test_bn_standard_000_g6
1735/1970 Test #1738: tailoring_rule_test_bn_standard_000_g6 ............***Failed    1.01 sec
          Start 1739: tailoring_rule_test_bn_traditional_000_g6
1736/1970 Test #1739: tailoring_rule_test_bn_traditional_000_g6 .........***Failed    1.32 sec
          Start 1740: tailoring_rule_test_bn_traditional_001_g6
1737/1970 Test #1740: tailoring_rule_test_bn_traditional_001_g6 .........***Failed    0.02 sec
          Start 1742: tailoring_rule_test_bs_search_000_g6
1739/1970 Test #1742: tailoring_rule_test_bs_search_000_g6 ..............***Failed    1.35 sec
          Start 1743: tailoring_rule_test_bs_standard_000_g6
1740/1970 Test #1743: tailoring_rule_test_bs_standard_000_g6 ............***Failed    1.07 sec
          Start 1744: tailoring_rule_test_ca_search_000_g6
1741/1970 Test #1744: tailoring_rule_test_ca_search_000_g6 ..............***Failed    1.23 sec
          Start 1746: tailoring_rule_test_cs_standard_000_g6
1743/1970 Test #1746: tailoring_rule_test_cs_standard_000_g6 ............***Failed    1.01 sec
          Start 1747: tailoring_rule_test_cy_standard_000_g6
1744/1970 Test #1732: tailoring_rule_test_ar_standard_001_g6 ............***Failed  174.82 sec
          Start 1748: tailoring_rule_test_da_search_000_g6
1745/1970 Test #1748: tailoring_rule_test_da_search_000_g6 ..............***Failed    1.28 sec
          Start 1749: tailoring_rule_test_da_standard_000_g6
1746/1970 Test #1749: tailoring_rule_test_da_standard_000_g6 ............***Failed    1.00 sec
          Start 1750: tailoring_rule_test_de_AT_phonebook_000_g6
1747/1970 Test #1750: tailoring_rule_test_de_AT_phonebook_000_g6 ........***Failed    0.98 sec
          Start 1751: tailoring_rule_test_de_phonebook_000_g6
1748/1970 Test #1751: tailoring_rule_test_de_phonebook_000_g6 ...........***Failed    0.96 sec
          Start 1752: tailoring_rule_test_de_search_000_g6
1749/1970 Test #1752: tailoring_rule_test_de_search_000_g6 ..............***Failed    1.24 sec
          Start 1753: tailoring_rule_test_dsb_standard_000_g6
1750/1970 Test #1753: tailoring_rule_test_dsb_standard_000_g6 ...........***Failed    1.01 sec
          Start 1754: tailoring_rule_test_ee_standard_000_g6
1751/1970 Test #1754: tailoring_rule_test_ee_standard_000_g6 ............***Failed    1.06 sec
          Start 1757: tailoring_rule_test_eo_standard_000_g6
1754/1970 Test #1757: tailoring_rule_test_eo_standard_000_g6 ............***Failed    0.97 sec
          Start 1758: tailoring_rule_test_es_search_000_g6
1755/1970 Test #1758: tailoring_rule_test_es_search_000_g6 ..............***Failed    1.24 sec
          Start 1759: tailoring_rule_test_es_standard_000_g6
1756/1970 Test #1759: tailoring_rule_test_es_standard_000_g6 ............***Failed    0.96 sec
          Start 1760: tailoring_rule_test_es_traditional_000_g6
1757/1970 Test #1760: tailoring_rule_test_es_traditional_000_g6 .........***Failed    1.01 sec
          Start 1762: tailoring_rule_test_fa_AF_standard_000_g6
1759/1970 Test #1731: tailoring_rule_test_ar_standard_000_g6 ............***Failed  195.26 sec
          Start 1763: tailoring_rule_test_fa_standard_000_g6
1760/1970 Test #1763: tailoring_rule_test_fa_standard_000_g6 ............***Failed    1.05 sec
          Start 1764: tailoring_rule_test_fi_search_000_g6
1761/1970 Test #1764: tailoring_rule_test_fi_search_000_g6 ..............***Failed    1.24 sec
          Start 1765: tailoring_rule_test_fi_standard_000_g6
1762/1970 Test #1735: tailoring_rule_test_az_standard_000_g6 ............***Failed  224.25 sec
          Start 1766: tailoring_rule_test_fi_traditional_000_g6
1763/1970 Test #1766: tailoring_rule_test_fi_traditional_000_g6 .........***Failed    0.97 sec
          Start 1767: tailoring_rule_test_fil_standard_000_g6
1764/1970 Test #1767: tailoring_rule_test_fil_standard_000_g6 ...........***Failed    0.98 sec
          Start 1768: tailoring_rule_test_fo_search_000_g6
1765/1970 Test #1768: tailoring_rule_test_fo_search_000_g6 ..............***Failed    1.30 sec
          Start 1769: tailoring_rule_test_fo_standard_000_g6
1766/1970 Test #1769: tailoring_rule_test_fo_standard_000_g6 ............***Failed    1.02 sec
          Start 1771: tailoring_rule_test_gl_search_000_g6
1768/1970 Test #1771: tailoring_rule_test_gl_search_000_g6 ..............***Failed    1.24 sec
          Start 1772: tailoring_rule_test_gl_standard_000_g6
1769/1970 Test #1772: tailoring_rule_test_gl_standard_000_g6 ............***Failed    0.96 sec
          Start 1773: tailoring_rule_test_gu_standard_000_g6
1770/1970 Test #1773: tailoring_rule_test_gu_standard_000_g6 ............***Failed    1.02 sec
          Start 1774: tailoring_rule_test_ha_standard_000_g6
1771/1970 Test #1774: tailoring_rule_test_ha_standard_000_g6 ............***Failed    1.02 sec
          Start 1775: tailoring_rule_test_haw_standard_000_g6
1772/1970 Test #1775: tailoring_rule_test_haw_standard_000_g6 ...........***Failed    0.97 sec
          Start 1776: tailoring_rule_test_he_search_000_g6
1773/1970 Test #1776: tailoring_rule_test_he_search_000_g6 ..............***Failed    1.24 sec
          Start 1778: tailoring_rule_test_hi_standard_000_g6
1775/1970 Test #1778: tailoring_rule_test_hi_standard_000_g6 ............***Failed    1.02 sec
          Start 1779: tailoring_rule_test_hr_search_000_g6
1776/1970 Test #1779: tailoring_rule_test_hr_search_000_g6 ..............***Failed    1.35 sec
          Start 1780: tailoring_rule_test_hr_standard_000_g6
1777/1970 Test #1747: tailoring_rule_test_cy_standard_000_g6 ............***Failed  218.86 sec
          Start 1781: tailoring_rule_test_hsb_standard_000_g6
1778/1970 Test #1781: tailoring_rule_test_hsb_standard_000_g6 ...........***Failed    1.01 sec
          Start 1782: tailoring_rule_test_hu_standard_000_g6
1779/1970 Test #1782: tailoring_rule_test_hu_standard_000_g6 ............***Failed    1.24 sec
          Start 1783: tailoring_rule_test_hy_standard_000_g6
1780/1970 Test #1783: tailoring_rule_test_hy_standard_000_g6 ............***Failed    1.02 sec
          Start 1784: tailoring_rule_test_ig_standard_000_g6
1781/1970 Test #1784: tailoring_rule_test_ig_standard_000_g6 ............***Failed    1.07 sec
          Start 1785: tailoring_rule_test_is_search_000_g6
1782/1970 Test #1785: tailoring_rule_test_is_search_000_g6 ..............***Failed    1.25 sec
          Start 1786: tailoring_rule_test_is_standard_000_g6
1783/1970 Test #1786: tailoring_rule_test_is_standard_000_g6 ............***Failed    0.96 sec
          Start 1787: tailoring_rule_test_ja_standard_000_g6
1784/1970 Test #1787: tailoring_rule_test_ja_standard_000_g6 ............***Failed    2.83 sec
          Start 1788: tailoring_rule_test_ja_standard_001_g6
1785/1970 Test #1788: tailoring_rule_test_ja_standard_001_g6 ............***Failed    0.03 sec
          Start 1789: tailoring_rule_test_ja_standard_002_g6
1786/1970 Test #1789: tailoring_rule_test_ja_standard_002_g6 ............***Failed    0.03 sec
          Start 1790: tailoring_rule_test_ja_standard_003_g6
1787/1970 Test #1790: tailoring_rule_test_ja_standard_003_g6 ............***Failed    0.03 sec
          Start 1791: tailoring_rule_test_ja_standard_004_g6
1788/1970 Test #1791: tailoring_rule_test_ja_standard_004_g6 ............***Failed    0.03 sec
          Start 1792: tailoring_rule_test_ja_standard_005_g6
1789/1970 Test #1792: tailoring_rule_test_ja_standard_005_g6 ............***Failed    0.03 sec
          Start 1793: tailoring_rule_test_ja_standard_006_g6
1790/1970 Test #1793: tailoring_rule_test_ja_standard_006_g6 ............***Failed    0.03 sec
          Start 1794: tailoring_rule_test_ja_standard_007_g6
1791/1970 Test #1794: tailoring_rule_test_ja_standard_007_g6 ............***Failed    0.02 sec
          Start 1795: tailoring_rule_test_ja_unihan_000_g6
1792/1970 Test #1795: tailoring_rule_test_ja_unihan_000_g6 ..............***Failed    2.45 sec
          Start 1796: tailoring_rule_test_ja_unihan_001_g6
1793/1970 Test #1796: tailoring_rule_test_ja_unihan_001_g6 ..............***Failed    0.01 sec
          Start 1798: tailoring_rule_test_kk_standard_000_g6
1795/1970 Test #1798: tailoring_rule_test_kk_standard_000_g6 ............***Failed    1.01 sec
          Start 1799: tailoring_rule_test_kl_search_000_g6
1796/1970 Test #1799: tailoring_rule_test_kl_search_000_g6 ..............***Failed    1.29 sec
          Start 1800: tailoring_rule_test_kl_standard_000_g6
1797/1970 Test #1800: tailoring_rule_test_kl_standard_000_g6 ............***Failed    1.01 sec
          Start 1801: tailoring_rule_test_km_standard_000_g6
1798/1970 Test #1801: tailoring_rule_test_km_standard_000_g6 ............***Failed    1.29 sec
          Start 1802: tailoring_rule_test_kn_standard_000_g6
1799/1970 Test #1802: tailoring_rule_test_kn_standard_000_g6 ............***Failed    1.02 sec
          Start 1803: tailoring_rule_test_kn_traditional_000_g6
1800/1970 Test #1803: tailoring_rule_test_kn_traditional_000_g6 .........***Failed    1.29 sec
          Start 1804: tailoring_rule_test_kn_traditional_001_g6
1801/1970 Test #1804: tailoring_rule_test_kn_traditional_001_g6 .........***Failed    0.02 sec
          Start 1805: tailoring_rule_test_ko_search_000_g6
1802/1970 Test #1805: tailoring_rule_test_ko_search_000_g6 ..............***Failed    1.26 sec
          Start 1806: tailoring_rule_test_ko_searchjl_000_g6
1803/1970 Test #1806: tailoring_rule_test_ko_searchjl_000_g6 ............***Failed    1.26 sec
          Start 1807: tailoring_rule_test_ko_standard_000_g6
1804/1970 Test #1762: tailoring_rule_test_fa_AF_standard_000_g6 .........***Failed  214.71 sec
          Start 1808: tailoring_rule_test_ko_standard_001_g6
1805/1970 Test #1765: tailoring_rule_test_fi_standard_000_g6 ............***Failed  215.08 sec
          Start 1809: tailoring_rule_test_ko_standard_002_g6
1806/1970 Test #1807: tailoring_rule_test_ko_standard_000_g6 ............***Failed  120.80 sec
          Start 1810: tailoring_rule_test_ko_standard_003_g6
1807/1970 Test #1808: tailoring_rule_test_ko_standard_001_g6 ............***Failed  126.93 sec
          Start 1811: tailoring_rule_test_ko_standard_004_g6
1808/1970 Test #1809: tailoring_rule_test_ko_standard_002_g6 ............***Failed  124.00 sec
          Start 1812: tailoring_rule_test_ko_standard_005_g6
1809/1970 Test #1810: tailoring_rule_test_ko_standard_003_g6 ............***Failed  123.60 sec
          Start 1813: tailoring_rule_test_ko_standard_006_g6
1810/1970 Test #1780: tailoring_rule_test_hr_standard_000_g6 ............***Failed  410.89 sec
          Start 1814: tailoring_rule_test_ko_standard_007_g6
1811/1970 Test #1811: tailoring_rule_test_ko_standard_004_g6 ............***Failed  126.48 sec
          Start 1815: tailoring_rule_test_ko_unihan_000_g6
1812/1970 Test #1812: tailoring_rule_test_ko_standard_005_g6 ............***Failed  124.20 sec
          Start 1816: tailoring_rule_test_kok_standard_000_g6
1813/1970 Test #1816: tailoring_rule_test_kok_standard_000_g6 ...........***Failed    1.02 sec
          Start 1817: tailoring_rule_test_ky_standard_000_g6
1814/1970 Test #1817: tailoring_rule_test_ky_standard_000_g6 ............***Failed    1.01 sec
          Start 1818: tailoring_rule_test_lkt_standard_000_g6
1815/1970 Test #1818: tailoring_rule_test_lkt_standard_000_g6 ...........***Failed    0.99 sec
          Start 1819: tailoring_rule_test_ln_phonetic_000_g6
1816/1970 Test #1819: tailoring_rule_test_ln_phonetic_000_g6 ............***Failed    1.09 sec
          Start 1820: tailoring_rule_test_ln_standard_000_g6
1817/1970 Test #1820: tailoring_rule_test_ln_standard_000_g6 ............***Failed    0.95 sec
          Start 1822: tailoring_rule_test_lt_standard_000_g6
1819/1970 Test #1822: tailoring_rule_test_lt_standard_000_g6 ............***Failed    0.99 sec
          Start 1823: tailoring_rule_test_lv_standard_000_g6
1820/1970 Test #1823: tailoring_rule_test_lv_standard_000_g6 ............***Failed    0.96 sec
          Start 1824: tailoring_rule_test_mk_standard_000_g6
1821/1970 Test #1813: tailoring_rule_test_ko_standard_006_g6 ............***Failed  128.08 sec
          Start 1825: tailoring_rule_test_ml_standard_000_g6
1822/1970 Test #1824: tailoring_rule_test_mk_standard_000_g6 ............***Failed    1.01 sec
          Start 1827: tailoring_rule_test_mr_standard_000_g6
1824/1970 Test #1825: tailoring_rule_test_ml_standard_000_g6 ............***Failed    1.11 sec
          Start 1829: tailoring_rule_test_my_standard_000_g6
1826/1970 Test #1827: tailoring_rule_test_mr_standard_000_g6 ............***Failed    1.02 sec
          Start 1830: tailoring_rule_test_my_standard_001_g6
1827/1970 Test #1829: tailoring_rule_test_my_standard_000_g6 ............***Failed    3.81 sec
          Start 1831: tailoring_rule_test_nb_search_000_g6
1828/1970 Test #1830: tailoring_rule_test_my_standard_001_g6 ............***Failed    3.78 sec
          Start 1832: tailoring_rule_test_nb_standard_000_g6
1829/1970 Test #1831: tailoring_rule_test_nb_search_000_g6 ..............***Failed    1.29 sec
          Start 1834: tailoring_rule_test_nn_search_000_g6
1831/1970 Test #1832: tailoring_rule_test_nb_standard_000_g6 ............***Failed    1.01 sec
          Start 1835: tailoring_rule_test_nn_standard_000_g6
1832/1970 Test #1835: tailoring_rule_test_nn_standard_000_g6 ............***Failed    1.01 sec
          Start 1836: tailoring_rule_test_om_standard_000_g6
1833/1970 Test #1834: tailoring_rule_test_nn_search_000_g6 ..............***Failed    1.29 sec
          Start 1837: tailoring_rule_test_or__standard_000_g6
1834/1970 Test #1836: tailoring_rule_test_om_standard_000_g6 ............***Failed    1.03 sec
          Start 1838: tailoring_rule_test_pa_standard_000_g6
1835/1970 Test #1837: tailoring_rule_test_or__standard_000_g6 ...........***Failed    1.02 sec
          Start 1839: tailoring_rule_test_pl_standard_000_g6
1836/1970 Test #1838: tailoring_rule_test_pa_standard_000_g6 ............***Failed    1.00 sec
          Start 1840: tailoring_rule_test_ps_standard_000_g6
1837/1970 Test #1839: tailoring_rule_test_pl_standard_000_g6 ............***Failed    0.97 sec
          Start 1841: tailoring_rule_test_ro_standard_000_g6
1838/1970 Test #1841: tailoring_rule_test_ro_standard_000_g6 ............***Failed    0.96 sec
          Start 1843: tailoring_rule_test_se_search_000_g6
1840/1970 Test #1843: tailoring_rule_test_se_search_000_g6 ..............***Failed    1.24 sec
          Start 1844: tailoring_rule_test_se_standard_000_g6
1841/1970 Test #1844: tailoring_rule_test_se_standard_000_g6 ............***Failed    0.96 sec
          Start 1845: tailoring_rule_test_si_dictionary_000_g6
1842/1970 Test #1845: tailoring_rule_test_si_dictionary_000_g6 ..........***Failed    1.02 sec
          Start 1846: tailoring_rule_test_si_standard_000_g6
1843/1970 Test #1846: tailoring_rule_test_si_standard_000_g6 ............***Failed    1.01 sec
          Start 1847: tailoring_rule_test_sk_search_000_g6
1844/1970 Test #1847: tailoring_rule_test_sk_search_000_g6 ..............***Failed    1.28 sec
          Start 1848: tailoring_rule_test_sk_standard_000_g6
1845/1970 Test #1848: tailoring_rule_test_sk_standard_000_g6 ............***Failed    1.05 sec
          Start 1849: tailoring_rule_test_sl_standard_000_g6
1846/1970 Test #1849: tailoring_rule_test_sl_standard_000_g6 ............***Failed    0.96 sec
          Start 1850: tailoring_rule_test_smn_search_000_g6
1847/1970 Test #1850: tailoring_rule_test_smn_search_000_g6 .............***Failed    1.25 sec
          Start 1851: tailoring_rule_test_smn_standard_000_g6
1848/1970 Test #1851: tailoring_rule_test_smn_standard_000_g6 ...........***Failed    0.97 sec
          Start 1853: tailoring_rule_test_sr_Latn_search_000_g6
1850/1970 Test #1853: tailoring_rule_test_sr_Latn_search_000_g6 .........***Failed    1.35 sec
          Start 1854: tailoring_rule_test_sr_Latn_standard_000_g6
1851/1970 Test #1840: tailoring_rule_test_ps_standard_000_g6 ............***Failed   95.72 sec
          Start 1856: tailoring_rule_test_sv_reformed_000_g6
1853/1970 Test #1856: tailoring_rule_test_sv_reformed_000_g6 ............***Failed    1.01 sec
          Start 1857: tailoring_rule_test_sv_search_000_g6
1854/1970 Test #1857: tailoring_rule_test_sv_search_000_g6 ..............***Failed    1.28 sec
          Start 1858: tailoring_rule_test_sv_standard_000_g6
1855/1970 Test #1858: tailoring_rule_test_sv_standard_000_g6 ............***Failed    1.01 sec
          Start 1859: tailoring_rule_test_ta_standard_000_g6
1856/1970 Test #1859: tailoring_rule_test_ta_standard_000_g6 ............***Failed    1.17 sec
          Start 1860: tailoring_rule_test_te_standard_000_g6
1857/1970 Test #1814: tailoring_rule_test_ko_standard_007_g6 ............***Failed  127.04 sec
          Start 1861: tailoring_rule_test_th_standard_000_g6
1858/1970 Test #1860: tailoring_rule_test_te_standard_000_g6 ............***Failed    1.07 sec
          Start 1862: tailoring_rule_test_to_standard_000_g6
1859/1970 Test #1862: tailoring_rule_test_to_standard_000_g6 ............***Failed    1.00 sec
          Start 1863: tailoring_rule_test_tr_search_000_g6
1860/1970 Test #1863: tailoring_rule_test_tr_search_000_g6 ..............***Failed    1.30 sec
          Start 1864: tailoring_rule_test_tr_standard_000_g6
1861/1970 Test #1864: tailoring_rule_test_tr_standard_000_g6 ............***Failed    1.02 sec
          Start 1865: tailoring_rule_test_ug_standard_000_g6
1862/1970 Test #1865: tailoring_rule_test_ug_standard_000_g6 ............***Failed    1.02 sec
          Start 1866: tailoring_rule_test_uk_standard_000_g6
1863/1970 Test #1866: tailoring_rule_test_uk_standard_000_g6 ............***Failed    1.01 sec
          Start 1867: tailoring_rule_test_und_emoji_000_g6
1864/1970 Test #1854: tailoring_rule_test_sr_Latn_standard_000_g6 .......***Failed  424.89 sec
          Start 1870: tailoring_rule_test_und_eor_000_g6
1867/1970 Test #1870: tailoring_rule_test_und_eor_000_g6 ................***Failed    1.19 sec
          Start 1871: tailoring_rule_test_und_search_000_g6
1868/1970 Test #1871: tailoring_rule_test_und_search_000_g6 .............***Failed    1.22 sec
          Start 1872: tailoring_rule_test_ur_standard_000_g6
1869/1970 Test #1872: tailoring_rule_test_ur_standard_000_g6 ............***Failed    1.14 sec
          Start 1874: tailoring_rule_test_vi_standard_000_g6
1871/1970 Test #1874: tailoring_rule_test_vi_standard_000_g6 ............***Failed    0.98 sec
          Start 1875: tailoring_rule_test_vi_traditional_000_g6
1872/1970 Test #1875: tailoring_rule_test_vi_traditional_000_g6 .........***Failed    1.11 sec
          Start 1876: tailoring_rule_test_wo_standard_000_g6
1873/1970 Test #1876: tailoring_rule_test_wo_standard_000_g6 ............***Failed    0.97 sec
          Start 1877: tailoring_rule_test_yi_search_000_g6
1874/1970 Test #1877: tailoring_rule_test_yi_search_000_g6 ..............***Failed    1.23 sec
          Start 1878: tailoring_rule_test_yi_standard_000_g6
1875/1970 Test #1878: tailoring_rule_test_yi_standard_000_g6 ............***Failed    1.08 sec
          Start 1879: tailoring_rule_test_yo_standard_000_g6
1876/1970 Test #1879: tailoring_rule_test_yo_standard_000_g6 ............***Failed    0.98 sec
          Start 1880: tailoring_rule_test_zh_big5han_000_g6
1877/1970 Test #1815: tailoring_rule_test_ko_unihan_000_g6 ..............***Failed  519.07 sec
          Start 1881: tailoring_rule_test_zh_big5han_001_g6
1878/1970 Test #1861: tailoring_rule_test_th_standard_000_g6 ............***Failed  428.36 sec
          Start 1882: tailoring_rule_test_zh_big5han_002_g6
1879/1970 Test #1880: tailoring_rule_test_zh_big5han_000_g6 .............***Failed  464.48 sec
          Start 1883: tailoring_rule_test_zh_big5han_003_g6
1880/1970 Test #1881: tailoring_rule_test_zh_big5han_001_g6 .............***Failed  462.91 sec
          Start 1884: tailoring_rule_test_zh_big5han_004_g6
1881/1970 Test #1882: tailoring_rule_test_zh_big5han_002_g6 .............***Failed  462.50 sec
          Start 1885: tailoring_rule_test_zh_big5han_005_g6
1882/1970 Test #1883: tailoring_rule_test_zh_big5han_003_g6 .............***Failed  462.82 sec
          Start 1886: tailoring_rule_test_zh_big5han_006_g6
1883/1970 Test #1884: tailoring_rule_test_zh_big5han_004_g6 .............***Failed  462.81 sec
          Start 1887: tailoring_rule_test_zh_big5han_007_g6
1884/1970 Test #1885: tailoring_rule_test_zh_big5han_005_g6 .............***Failed  463.04 sec
          Start 1888: tailoring_rule_test_zh_big5han_008_g6
1885/1970 Test #1886: tailoring_rule_test_zh_big5han_006_g6 .............***Failed  462.44 sec
          Start 1889: tailoring_rule_test_zh_big5han_009_g6
1886/1970 Test #1887: tailoring_rule_test_zh_big5han_007_g6 .............***Failed  462.23 sec
          Start 1890: tailoring_rule_test_zh_big5han_010_g6
1887/1970 Test #1888: tailoring_rule_test_zh_big5han_008_g6 .............***Failed  463.03 sec
          Start 1891: tailoring_rule_test_zh_big5han_011_g6
1888/1970 Test #1889: tailoring_rule_test_zh_big5han_009_g6 .............***Failed  463.33 sec
          Start 1892: tailoring_rule_test_zh_big5han_012_g6
1889/1970 Test #1890: tailoring_rule_test_zh_big5han_010_g6 .............***Failed  463.31 sec
          Start 1893: tailoring_rule_test_zh_gb2312han_000_g6
1890/1970 Test #1893: tailoring_rule_test_zh_gb2312han_000_g6 ...........***Failed    1.51 sec
          Start 1894: tailoring_rule_test_zh_gb2312han_001_g6
1891/1970 Test #1894: tailoring_rule_test_zh_gb2312han_001_g6 ...........***Failed    0.02 sec
          Start 1895: tailoring_rule_test_zh_gb2312han_002_g6
1892/1970 Test #1895: tailoring_rule_test_zh_gb2312han_002_g6 ...........***Failed    0.02 sec
          Start 1896: tailoring_rule_test_zh_gb2312han_003_g6
1893/1970 Test #1896: tailoring_rule_test_zh_gb2312han_003_g6 ...........***Failed    0.02 sec
          Start 1897: tailoring_rule_test_zh_gb2312han_004_g6
1894/1970 Test #1897: tailoring_rule_test_zh_gb2312han_004_g6 ...........***Failed    0.01 sec
          Start 1898: tailoring_rule_test_zh_gb2312han_005_g6
1895/1970 Test #1898: tailoring_rule_test_zh_gb2312han_005_g6 ...........***Failed    0.01 sec
          Start 1899: tailoring_rule_test_zh_gb2312han_006_g6
1896/1970 Test #1899: tailoring_rule_test_zh_gb2312han_006_g6 ...........***Failed    0.01 sec
          Start 1900: tailoring_rule_test_zh_pinyin_000_g6
1897/1970 Test #1900: tailoring_rule_test_zh_pinyin_000_g6 ..............***Failed    2.71 sec
          Start 1901: tailoring_rule_test_zh_pinyin_001_g6
1898/1970 Test #1901: tailoring_rule_test_zh_pinyin_001_g6 ..............***Failed    0.02 sec
          Start 1902: tailoring_rule_test_zh_pinyin_002_g6
1899/1970 Test #1902: tailoring_rule_test_zh_pinyin_002_g6 ..............***Failed    0.02 sec
          Start 1903: tailoring_rule_test_zh_pinyin_003_g6
1900/1970 Test #1903: tailoring_rule_test_zh_pinyin_003_g6 ..............***Failed    0.01 sec
          Start 1904: tailoring_rule_test_zh_pinyin_004_g6
1901/1970 Test #1904: tailoring_rule_test_zh_pinyin_004_g6 ..............***Failed    0.02 sec
          Start 1905: tailoring_rule_test_zh_pinyin_005_g6
1902/1970 Test #1905: tailoring_rule_test_zh_pinyin_005_g6 ..............***Failed    0.02 sec
          Start 1906: tailoring_rule_test_zh_pinyin_006_g6
1903/1970 Test #1906: tailoring_rule_test_zh_pinyin_006_g6 ..............***Failed    0.01 sec
          Start 1907: tailoring_rule_test_zh_pinyin_007_g6
1904/1970 Test #1907: tailoring_rule_test_zh_pinyin_007_g6 ..............***Failed    0.01 sec
          Start 1908: tailoring_rule_test_zh_pinyin_008_g6
1905/1970 Test #1908: tailoring_rule_test_zh_pinyin_008_g6 ..............***Failed    0.01 sec
          Start 1909: tailoring_rule_test_zh_pinyin_009_g6
1906/1970 Test #1909: tailoring_rule_test_zh_pinyin_009_g6 ..............***Failed    0.01 sec
          Start 1910: tailoring_rule_test_zh_pinyin_010_g6
1907/1970 Test #1910: tailoring_rule_test_zh_pinyin_010_g6 ..............***Failed    0.02 sec
          Start 1911: tailoring_rule_test_zh_pinyin_011_g6
1908/1970 Test #1911: tailoring_rule_test_zh_pinyin_011_g6 ..............***Failed    0.01 sec
          Start 1912: tailoring_rule_test_zh_pinyin_012_g6
1909/1970 Test #1912: tailoring_rule_test_zh_pinyin_012_g6 ..............***Failed    0.02 sec
          Start 1913: tailoring_rule_test_zh_pinyin_013_g6
1910/1970 Test #1913: tailoring_rule_test_zh_pinyin_013_g6 ..............***Failed    0.02 sec
          Start 1914: tailoring_rule_test_zh_pinyin_014_g6
1911/1970 Test #1914: tailoring_rule_test_zh_pinyin_014_g6 ..............***Failed    0.01 sec
          Start 1915: tailoring_rule_test_zh_pinyin_015_g6
1912/1970 Test #1915: tailoring_rule_test_zh_pinyin_015_g6 ..............***Failed    0.01 sec
          Start 1916: tailoring_rule_test_zh_pinyin_016_g6
1913/1970 Test #1916: tailoring_rule_test_zh_pinyin_016_g6 ..............***Failed    0.02 sec
          Start 1917: tailoring_rule_test_zh_pinyin_017_g6
1914/1970 Test #1917: tailoring_rule_test_zh_pinyin_017_g6 ..............***Failed    0.01 sec
          Start 1918: tailoring_rule_test_zh_pinyin_018_g6
1915/1970 Test #1918: tailoring_rule_test_zh_pinyin_018_g6 ..............***Failed    0.01 sec
          Start 1919: tailoring_rule_test_zh_pinyin_019_g6
1916/1970 Test #1919: tailoring_rule_test_zh_pinyin_019_g6 ..............***Failed    0.01 sec
          Start 1920: tailoring_rule_test_zh_pinyin_020_g6
1917/1970 Test #1920: tailoring_rule_test_zh_pinyin_020_g6 ..............***Failed    0.02 sec
          Start 1921: tailoring_rule_test_zh_pinyin_021_g6
1918/1970 Test #1921: tailoring_rule_test_zh_pinyin_021_g6 ..............***Failed    0.01 sec
          Start 1922: tailoring_rule_test_zh_stroke_000_g6
1919/1970 Test #1891: tailoring_rule_test_zh_big5han_011_g6 .............***Failed  464.09 sec
          Start 1923: tailoring_rule_test_zh_stroke_001_g6
1920/1970 Test #1922: tailoring_rule_test_zh_stroke_000_g6 ..............***Failed  381.87 sec
          Start 1924: tailoring_rule_test_zh_stroke_002_g6
1921/1970 Test #1892: tailoring_rule_test_zh_big5han_012_g6 .............***Failed  443.63 sec
          Start 1925: tailoring_rule_test_zh_stroke_003_g6
1922/1970 Test #1923: tailoring_rule_test_zh_stroke_001_g6 ..............***Failed  365.63 sec
          Start 1926: tailoring_rule_test_zh_stroke_004_g6
1923/1970 Test #1924: tailoring_rule_test_zh_stroke_002_g6 ..............***Failed  346.05 sec
          Start 1927: tailoring_rule_test_zh_stroke_005_g6
1924/1970 Test #1925: tailoring_rule_test_zh_stroke_003_g6 ..............***Failed  343.46 sec
          Start 1928: tailoring_rule_test_zh_stroke_006_g6
1925/1970 Test #1926: tailoring_rule_test_zh_stroke_004_g6 ..............***Failed  341.17 sec
          Start 1929: tailoring_rule_test_zh_stroke_007_g6
1926/1970 Test #1927: tailoring_rule_test_zh_stroke_005_g6 ..............***Failed  345.65 sec
          Start 1930: tailoring_rule_test_zh_stroke_008_g6
1927/1970 Test #1928: tailoring_rule_test_zh_stroke_006_g6 ..............***Failed  341.38 sec
          Start 1931: tailoring_rule_test_zh_stroke_009_g6
1928/1970 Test #1929: tailoring_rule_test_zh_stroke_007_g6 ..............***Failed  355.63 sec
          Start 1932: tailoring_rule_test_zh_stroke_010_g6
1929/1970 Test #1930: tailoring_rule_test_zh_stroke_008_g6 ..............***Failed  342.77 sec
          Start 1933: tailoring_rule_test_zh_stroke_011_g6
1930/1970 Test #1931: tailoring_rule_test_zh_stroke_009_g6 ..............***Failed  345.43 sec
          Start 1934: tailoring_rule_test_zh_stroke_012_g6
1931/1970 Test #1932: tailoring_rule_test_zh_stroke_010_g6 ..............***Failed  340.84 sec
          Start 1935: tailoring_rule_test_zh_stroke_013_g6
1932/1970 Test #1934: tailoring_rule_test_zh_stroke_012_g6 ..............***Failed  341.19 sec
          Start 1936: tailoring_rule_test_zh_stroke_014_g6
1933/1970 Test #1933: tailoring_rule_test_zh_stroke_011_g6 ..............***Failed  346.25 sec
          Start 1937: tailoring_rule_test_zh_stroke_015_g6
1934/1970 Test #1935: tailoring_rule_test_zh_stroke_013_g6 ..............***Failed  344.46 sec
          Start 1938: tailoring_rule_test_zh_stroke_016_g6
1935/1970 Test #1936: tailoring_rule_test_zh_stroke_014_g6 ..............***Failed  341.60 sec
          Start 1939: tailoring_rule_test_zh_stroke_017_g6
1936/1970 Test #1937: tailoring_rule_test_zh_stroke_015_g6 ..............***Failed  345.10 sec
          Start 1940: tailoring_rule_test_zh_stroke_018_g6
1937/1970 Test #1938: tailoring_rule_test_zh_stroke_016_g6 ..............***Failed  341.36 sec
          Start 1941: tailoring_rule_test_zh_stroke_019_g6
1938/1970 Test #1939: tailoring_rule_test_zh_stroke_017_g6 ..............***Failed  341.32 sec
          Start 1942: tailoring_rule_test_zh_stroke_020_g6
1939/1970 Test #1940: tailoring_rule_test_zh_stroke_018_g6 ..............***Failed  343.97 sec
          Start 1943: tailoring_rule_test_zh_stroke_021_g6
1940/1970 Test #1941: tailoring_rule_test_zh_stroke_019_g6 ..............***Failed  364.72 sec
          Start 1944: tailoring_rule_test_zh_stroke_022_g6
1941/1970 Test #1942: tailoring_rule_test_zh_stroke_020_g6 ..............***Failed  333.70 sec
          Start 1945: tailoring_rule_test_zh_stroke_023_g6
1942/1970 Test #1943: tailoring_rule_test_zh_stroke_021_g6 ..............***Failed  351.66 sec
          Start 1946: tailoring_rule_test_zh_stroke_024_g6
1943/1970 Test #1944: tailoring_rule_test_zh_stroke_022_g6 ..............***Failed  344.15 sec
          Start 1947: tailoring_rule_test_zh_unihan_000_g6
1944/1970 Test #1947: tailoring_rule_test_zh_unihan_000_g6 ..............***Failed    1.12 sec
          Start 1948: tailoring_rule_test_zh_zhuyin_000_g6
1945/1970 Test #1948: tailoring_rule_test_zh_zhuyin_000_g6 ..............***Failed  114.00 sec
          Start 1949: tailoring_rule_test_zh_zhuyin_001_g6
1946/1970 Test #1949: tailoring_rule_test_zh_zhuyin_001_g6 ..............***Failed  116.55 sec
          Start 1950: tailoring_rule_test_zh_zhuyin_002_g6
1947/1970 Test #1945: tailoring_rule_test_zh_stroke_023_g6 ..............***Failed  348.35 sec
          Start 1951: tailoring_rule_test_zh_zhuyin_003_g6
1948/1970 Test #1946: tailoring_rule_test_zh_stroke_024_g6 ..............***Failed  353.89 sec
          Start 1952: tailoring_rule_test_zh_zhuyin_004_g6
1949/1970 Test #1950: tailoring_rule_test_zh_zhuyin_002_g6 ..............***Failed  113.89 sec
          Start 1953: tailoring_rule_test_zh_zhuyin_005_g6
1950/1970 Test #1951: tailoring_rule_test_zh_zhuyin_003_g6 ..............***Failed  113.58 sec
          Start 1954: tailoring_rule_test_zh_zhuyin_006_g6
1951/1970 Test #1952: tailoring_rule_test_zh_zhuyin_004_g6 ..............***Failed  113.67 sec
          Start 1955: tailoring_rule_test_zh_zhuyin_007_g6
1952/1970 Test #1953: tailoring_rule_test_zh_zhuyin_005_g6 ..............***Failed  112.07 sec
          Start 1956: tailoring_rule_test_zh_zhuyin_008_g6
1953/1970 Test #1954: tailoring_rule_test_zh_zhuyin_006_g6 ..............***Failed  114.08 sec
          Start 1957: tailoring_rule_test_zh_zhuyin_009_g6
1954/1970 Test #1955: tailoring_rule_test_zh_zhuyin_007_g6 ..............***Failed  114.27 sec
          Start 1958: tailoring_rule_test_zh_zhuyin_010_g6
1955/1970 Test #1956: tailoring_rule_test_zh_zhuyin_008_g6 ..............***Failed  116.04 sec
          Start 1959: tailoring_rule_test_zh_zhuyin_011_g6
1956/1970 Test #1957: tailoring_rule_test_zh_zhuyin_009_g6 ..............***Failed  120.69 sec
          Start 1960: tailoring_rule_test_zh_zhuyin_012_g6
1957/1970 Test #1958: tailoring_rule_test_zh_zhuyin_010_g6 ..............***Failed  114.51 sec
          Start 1961: tailoring_rule_test_zh_zhuyin_013_g6
1958/1970 Test #1959: tailoring_rule_test_zh_zhuyin_011_g6 ..............***Failed  118.10 sec
          Start 1962: tailoring_rule_test_zh_zhuyin_014_g6
1959/1970 Test #1960: tailoring_rule_test_zh_zhuyin_012_g6 ..............***Failed  113.95 sec
          Start 1963: tailoring_rule_test_zh_zhuyin_015_g6
1960/1970 Test #1961: tailoring_rule_test_zh_zhuyin_013_g6 ..............***Failed  117.50 sec
          Start 1964: tailoring_rule_test_zh_zhuyin_016_g6
1961/1970 Test #1962: tailoring_rule_test_zh_zhuyin_014_g6 ..............***Failed  114.37 sec
          Start 1965: tailoring_rule_test_zh_zhuyin_017_g6
1962/1970 Test #1963: tailoring_rule_test_zh_zhuyin_015_g6 ..............***Failed  118.45 sec
          Start 1966: tailoring_rule_test_zh_zhuyin_018_g6
1963/1970 Test #1964: tailoring_rule_test_zh_zhuyin_016_g6 ..............***Failed  119.11 sec
          Start 1967: tailoring_rule_test_zh_zhuyin_019_g6
1964/1970 Test #1965: tailoring_rule_test_zh_zhuyin_017_g6 ..............***Failed  113.67 sec
          Start 1968: tailoring_rule_test_zh_zhuyin_020_g6
1965/1970 Test #1966: tailoring_rule_test_zh_zhuyin_018_g6 ..............***Failed  113.97 sec
          Start 1969: tailoring_rule_test_zh_zhuyin_021_g6
1966/1970 Test #1967: tailoring_rule_test_zh_zhuyin_019_g6 ..............***Failed  111.47 sec
          Start 1970: tailoring_rule_test_zh_zhuyin_022_g6
1967/1970 Test #1968: tailoring_rule_test_zh_zhuyin_020_g6 ..............***Failed  113.23 sec
1968/1970 Test #1969: tailoring_rule_test_zh_zhuyin_021_g6 ..............***Failed  109.87 sec
1969/1970 Test #1970: tailoring_rule_test_zh_zhuyin_022_g6 ..............***Failed  106.39 sec

tailoring_rule_test_und_emoji_000_g6 does not seem to terminate (after 128 minutes).
*/
