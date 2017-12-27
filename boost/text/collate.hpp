#ifndef BOOST_TEXT_COLLATE_HPP
#define BOOST_TEXT_COLLATE_HPP

#include <boost/text/collation_data.hpp>
#include <boost/text/string.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/algorithm/cxx14/mismatch.hpp>


namespace boost { namespace text {

    /** TODO */
    enum class variable_weighting { non_ignorable, blanked, shifted };

    /** TODO */
    struct text_sort_key
    {
        using iterator = std::vector<uint32_t>::const_iterator;

        explicit text_sort_key(std::vector<compressed_collation_element> const &
                                   collation_elements)
        {
            // TODO: Totally wrong!
            storage_.resize(collation_elements.size());
            std::transform(
                collation_elements.begin(),
                collation_elements.end(),
                storage_.begin(),
                [](compressed_collation_element e) {
                    return uint32_t(e.l1()) << 16 | uint32_t(e.l2()) << 8 |
                           uint32_t(e.l3());
                });
        }

        iterator begin() const noexcept { return sotrage_.begin(); }
        iterator end() const noexcept { return sotrage_.end(); }

        int compare(sort_key const & rhs) const noexcept
        {
            auto const pair = boost::algorithm::mismatch(
                begin(), end(), rhs.begin(), rhs.end());
            if (pair.first == end()) {
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

    private:
        std::vector<uint32_t> storage_;
    };

    // The code in this file implements the UCA as described in
    // http://www.unicode.org/reports/tr10/#Main_Algorithm .  The numbering
    // and some variable naming comes from there.
    namespace detail {

        // http://www.unicode.org/reports/tr10/#Derived_Collation_Elements
        template<typename Iter>
        void add_derived_elements(
            uint32_t cp, std::vector<compressed_collation_element> & ces)
        {
            // TODO: if (hangul_syllable(cp)) {
            //     auto const decomp = decompose_hangul_syllable(cp);
            //     ...
            //}

            // Tangut and Tangut Components
            if (0x17000 <= cp && cp <= 0x18AFF) {
                ces.push_back(
                    compressed_collation_element{0xFB00, 0x0020, 0x0020});
                ces.push_back(compressed_collation_element{
                    (cp - 0x17000) | 0x8000, 0x0000, 0x0000});
            }

            // Nushu
            if (0x1B170 <= cp && cp <= 0x1B2FF) {
                ces.push_back(
                    compressed_collation_element{0xFB00, 0x0020, 0x0020});
                ces.push_back(compressed_collation_element{
                    (cp - 0x1B170) | 0x8000, 0x0000, 0x0000});
            }

            compressed_collation_element const BBBB{
                (cp & 0x7FFF) | 0x8000, 0x0000, 0x0000};

            // Core Han Unified Ideographs
            std::array<uint32_t, 12> const CJK_Compatibility_Ideographs = {
                0xFA0E,
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
                0xFA29};

            if (0x4E00 <= cp && cp <= 0x9FEA ||
                ((cp & detail::OR_CJK_Compatibility_Ideographs) &&
                 std::binary_search(
                     CJK_Compatibility_Ideographs.begin(),
                     CJK_Compatibility_Ideographs.end(),
                     cp))) {
                ces.push_back(compressed_collation_element{
                    0xFB40 + (cp >> 15), 0x0020, 0x0020});
                ces.push_back(BBBB);
            }

            std::array<uint32_t, 222> const CJK_Unified_Ideographs_Extension_D =
                {0x2B740, 0x2B741, 0x2B742, 0x2B743, 0x2B744, 0x2B745, 0x2B746,
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
                 0x2B819, 0x2B81A, 0x2B81B, 0x2B81C, 0x2B81D};

            // All other Han Unified Ideographs
            if ((0x3400 <= cp && cp <= 0x4DB5) ||
                (0x20000 <= cp && cp <= 0x2A6D6) ||
                (0x2A700 <= cp && cp <= 0x2B734) ||
                (0x2B820 <= cp && cp <= 0x2CEA1) ||
                (0x2CEB0 <= cp && cp <= 0x2EBE0) ||
                ((cp & detail::OR_CJK_Unified_Ideographs_Extension_D) &&
                 std::binary_search(
                     CJK_Unified_Ideographs_Extension_D.begin(),
                     CJK_Unified_Ideographs_Extension_D.end(),
                     cp))) {
                ces.push_back(compressed_collation_element{
                    0xFB80 + (cp >> 15), 0x0020, 0x0020});
                ces.push_back(BBBB);
            }

            // Everything else (except Hangul; sigh).
            ces.push_back(compressed_collation_element{
                0xFBC0 + (cp >> 15), 0x0020, 0x0020});
            ces.push_back(BBBB);
        }

        inline bool ignorable(compressed_collation_element ce) noexcept
        {
            return ce.l1() == 0;
        }

        // http://www.unicode.org/reports/tr10/#Variable_Weighting
        void s2_3(
            std::vector<compressed_collation_element> & ces,
            variable_weighting option)
        {
            if (option == variable_weighting::non_ignorable)
                return;

            if (option == variable_weighting::blanked) {
                // TODO
            } else {
                // shifted
                // TODO
            }
        }

        // Does all of S2 except for S2.3, which is handled when building
        // the sort key.
        template<typename Iter>
        void
        s2(Iter first,
           Iter last,
           std::vector<compressed_collation_element> & ces)
        {
            // S2.1 Find longest prefix that results in a collation table
            // match.
            auto collation_ = longest_collation(first, last);
            if (collation.match_length_ == 0) {
                while (first != last) {
                    // TODO: This should not go past next starter.
                    add_derived_elements(*first++);
                }
                return;
            }
            auto S_last = first + collation_.match_length_;

            // S2.1.1 Process any nonstarters following S.
            auto nonstarter_last = std::find_if(
                first, last, [](uint32_t cp) { return ccc(cp) == 0; });

            auto nonstarter_first = S_end;
            while (!collation.node_.leaf() &&
                   nonstarter_first != nonstarter_last) {
                auto coll =
                    detail::extend_collation(collation_, *nonstarter_first);
                if (collation_.match_length_ < coll.match_length_) {
                    S_end = std::rotate(
                        S_end, nonstarter_first, nonstarter_first + 1);
                    collation_ = coll;
                }
                ++nonstarter_first;
            }

            std::copy(
                collation_.node_.collation_elements_.begin(),
                collation_.node_.collation_elements_.end(),
                std::back_inserter(ces));
        }

        inline text_sort_key collation_sort_key(string const & s)
        {
            std::vector<compressed_collation_element> ces;
            utf32_range as_utf32(s);
            // TODO: Try tuning this buffer size for perf.
            std::array<uint32_t, 256> buffer;
            while (!as_utf32.empty()) {
                auto it = as_utf32.begin();
                auto out_it = buffer.begin();
                while (it != as_utf32.end() && out_it != buffer.end()) {
                    *out_it++ = *it;
                    ++it;
                }
                // TODO: Only pass the portion of buffer ending at the last
                // ccc == 0.
                s2(buffer.begin(), out_it, ces);
                as_utf32 = utf32_range(it, as_utf32.end());
            }
            return text_sort_key(ces);
        }

        inline int collate(string const & lhs, string const & rhs)
        {
            // TODO: Do this incrementally, and bail early once the answer
            // is certain.
            // TODO: Do this into stack buffers to avoid allocation for
            // small enough strings.
            text_sort_key const lhs_sk = collation_sort_key(lhs);
            text_sort_key const rhs_sk = collation_sort_key(rhs);
            return lhs_sk.compare(rhs.sk);
        }
    }

    /** TODO
        TODO: string -> text
    */
    inline text_sort_key collation_sort_key(string const & s)
    {
        return detail::collation_sort_key(s);
    }

    /** TODO
        TODO: string -> text
    */
    inline int collate(string const & lhs, string const & rhs)
    {
        return detail::collate(lhs, rhs);
    }

    // TODO: Tailored collation.
}}

#endif
