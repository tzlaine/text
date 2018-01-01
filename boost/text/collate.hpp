#ifndef BOOST_TEXT_COLLATE_HPP
#define BOOST_TEXT_COLLATE_HPP

#include <boost/text/normalize.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/algorithm/cxx14/mismatch.hpp>

#include <iomanip>
#include <ostream>


namespace boost { namespace text {

    /** TODO */
    enum class collation_strength {
        primary,
        secondary,
        tertiary,
        quaternary,
        identical
    };

    /** TODO */
    enum class variable_weighting { non_ignorable, shifted };

    /** TODO */
    enum class l2_weight_order { forward, backward };

    /** TODO */
    struct text_sort_key
    {
        using iterator = std::vector<uint32_t>::const_iterator;

        text_sort_key() {}
        explicit text_sort_key(std::vector<uint32_t> bytes) :
            storage_(std::move(bytes))
        {}

        iterator begin() const noexcept { return storage_.begin(); }
        iterator end() const noexcept { return storage_.end(); }

        int compare(text_sort_key const & rhs) const noexcept
        {
            auto const pair = algorithm::mismatch(
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

#ifndef NDEBUG
        friend std::ostream &
        operator<<(std::ostream & os, text_sort_key const & k)
        {
            os << std::hex;
            bool first = true;
            for (auto x : k.storage_) {
                if (!first)
                    os << ", ";
                os << "0x" << std::setfill('0') << std::setw(4) << x;
                first = false;
            }
            os << std::dec;
            return os;
        }
#endif

    private:
        std::vector<uint32_t> storage_;
    };

    // The code in this file implements the UCA as described in
    // http://www.unicode.org/reports/tr10/#Main_Algorithm .  The numbering
    // and some variable naming comes from there.
    namespace detail {

        template<typename Iter>
        void
        s2(Iter first,
           Iter last,
           variable_weighting weighting,
           container::small_vector<collation_element, 1024> & ces);

        // http://www.unicode.org/reports/tr10/#Derived_Collation_Elements
        template<typename OutIter>
        inline OutIter add_derived_elements(
            uint32_t cp, variable_weighting weighting, OutIter out)
        {
            if (hangul_syllable(cp)) {
                auto cps = decompose_hangul_syllable<3>(cp);
                container::small_vector<collation_element, 1024> ces;
                s2(cps.begin(), cps.end(), weighting, ces);
                return std::copy(ces.begin(), ces.end(), out);
            }

            // Tangut and Tangut Components
            if (0x17000 <= cp && cp <= 0x18AFF) {
                *out++ = collation_element{0xFB00, 0x0020, 0x0002};
                *out++ = collation_element{
                    uint16_t((cp - 0x17000) | 0x8000), 0x0000, 0x0000};
                return out;
            }

            // Nushu
            if (0x1B170 <= cp && cp <= 0x1B2FF) {
                *out++ = collation_element{0xFB01, 0x0020, 0x0002};
                *out++ = collation_element{
                    uint16_t((cp - 0x1B170) | 0x8000), 0x0000, 0x0000};
                return out;
            }

            collation_element const BBBB{
                uint16_t((cp & 0x7FFF) | 0x8000), 0x0000, 0x0000};

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

            if ((0x4E00 <= cp && cp <= 0x9FEA) ||
                ((cp & detail::OR_CJK_Compatibility_Ideographs) &&
                 std::binary_search(
                     CJK_Compatibility_Ideographs.begin(),
                     CJK_Compatibility_Ideographs.end(),
                     cp))) {
                *out++ = collation_element{
                    uint16_t(0xFB40 + (cp >> 15)), 0x0020, 0x0002};
                *out++ = BBBB;
                return out;
            }

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
                *out++ = collation_element{
                    uint16_t(0xFB80 + (cp >> 15)), 0x0020, 0x0002};
                *out++ = BBBB;
                return out;
            }

            *out++ = collation_element{
                uint16_t(0xFBC0 + (cp >> 15)), 0x0020, 0x0002};
            *out++ = BBBB;
            return out;
        }

        inline bool variable(collation_element ce) noexcept
        {
            auto const lo = static_cast<int>(collation_weights::min_variable);
            auto const hi = static_cast<int>(collation_weights::max_variable);
            return lo <= ce.l1_ && ce.l1_ <= hi;
        }

        inline bool ignorable(collation_element ce) noexcept
        {
            return ce.l1_ == 0;
        }

        // http://www.unicode.org/reports/tr10/#Variable_Weighting
        inline bool s2_3(
            collation_element * first,
            collation_element * last,
            variable_weighting weighting,
            bool after_variable)
        {
            if (weighting == variable_weighting::non_ignorable)
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

            auto it = first;
            while (it != last) {
                auto & ce = *it++;
                auto const l1 = ce.l1_;
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
                            ce.l4_ = 0xffff;
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
                        ce.l4_ = 0xffff;
                    after_variable = false;
                }
                if (second_of_implicit_weight_pair) {
                    ce.l4_ = 0;
                    second_of_implicit_weight_pair = false;
                }
                second_of_implicit_weight_pair =
                    l1 == 0xfb00 || l1 == 0xfb01 ||
                    (l1 == 0xfb40 || l1 == 0xfb41) ||
                    (l1 == 0xfb80 || l1 == 0xfb84 || l1 == 0xfb85) ||
                    (0xfbc0 <= l1 && l1 <= 0xfbe1);
            }

            return after_variable;
        }

        template<typename Iter>
        void
        s2(Iter first,
           Iter last,
           variable_weighting weighting,
           container::small_vector<collation_element, 1024> & ces)
        {
            bool after_variable = false;
            while (first != last) {
                // S2.1 Find longest prefix that results in a collation
                // table match.
                auto collation_ = longest_collation(first, last);
                if (collation_.match_length_ == 0) {
                    // S2.2
                    collation_element cces[32];
                    auto const cces_end =
                        add_derived_elements(*first++, weighting, cces);
                    after_variable =
                        s2_3(cces, cces_end, weighting, after_variable);
                    std::copy(cces, cces_end, std::back_inserter(ces));
                    continue;
                }
                first += collation_.match_length_;

                // S2.1.1 Process any nonstarters following S.
                auto nonstarter_last = first;
                if (!collation_.node_.leaf()) {
                    nonstarter_last = std::find_if(
                        first, last, [](uint32_t cp) { return ccc(cp) == 0; });
                }

                // TODO: Optimize the ccc() calls.

                // S2.1.2
                auto nonstarter_first = first;
                while (!collation_.node_.leaf() &&
                       nonstarter_first != nonstarter_last &&
                       ccc(*(nonstarter_first - 1)) < ccc(*nonstarter_first)) {
                    bool const unblocked =
                        nonstarter_first == first + 1 ||
                        ccc(*(nonstarter_first - 1)) < ccc(*nonstarter_first);
                    if (unblocked) {
                        auto const cp = *nonstarter_first;
                        auto coll = detail::extend_collation(collation_, cp);
                        // S2.1.3
                        if (collation_.match_length_ < coll.match_length_) {
                            std::copy_backward(
                                first, nonstarter_first, nonstarter_first + 1);
                            *first++ = cp;
                            collation_ = coll;
                        }
                    }
                    ++nonstarter_first;
                }

                // S2.4
                std::transform(
                    collation_.node_.collation_elements_.begin(),
                    collation_.node_.collation_elements_.end(),
                    std::back_inserter(ces),
                    [](compressed_collation_element ce) {
                        return to_collation_element(ce);
                    });

                // S2.3
                after_variable = s2_3(
                    &*(ces.end() - collation_.node_.collation_elements_.size()),
                    &*ces.end(),
                    weighting,
                    after_variable);
            }
        }

        template<typename Iter>
        void
            s3(container::small_vector<collation_element, 1024> const & ces,
               collation_strength strength,
               l2_weight_order l2_order,
               Iter cps_first,
               Iter cps_last,
               int cps_size,
               std::vector<uint32_t> & bytes)
        {
            // TODO: Provide an API for passing in scratch space so that l[1-4]
            // are not repeatedly realloacted.  (Same with the NFD string above.)
            container::small_vector<uint32_t, 256> l1;
            container::small_vector<uint32_t, 256> l2;
            container::small_vector<uint32_t, 256> l3;
            container::small_vector<uint32_t, 256> l4;
            l1.reserve(ces.size());
            if (collation_strength::primary < strength) {
                l2.reserve(ces.size());
                if (collation_strength::secondary < strength) {
                    l3.reserve(ces.size());
                    if (collation_strength::tertiary < strength)
                        l4.reserve(ces.size());
                }
            }

            for (auto ce : ces) {
                if (ce.l1_)
                    l1.push_back(ce.l1_);
                if (collation_strength::primary < strength) {
                    if (ce.l2_)
                        l2.push_back(ce.l2_);
                    if (collation_strength::secondary < strength) {
                        if (ce.l3_)
                            l3.push_back(ce.l3_);
                        if (collation_strength::tertiary < strength) {
                            if (ce.l4_)
                                l4.push_back(ce.l4_);
                        }
                    }
                }
            }

            // TODO: Needs to change under certain compression schemes.
            int const separators = static_cast<int>(strength);

            container::small_vector<uint32_t, 256> nfd;
            if (collation_strength::quaternary < strength)
                normalize_to_nfd(cps_first, cps_last, std::back_inserter(nfd));

            int size = l1.size();
            if (collation_strength::primary < strength) {
                size += l2.size();
                if (collation_strength::secondary < strength) {
                    size += l3.size();
                    if (collation_strength::tertiary < strength) {
                        size += l4.size();
                        if (collation_strength::quaternary < strength)
                            size += nfd.size();
                    }
                }
            }
            size += separators;

            bytes.resize(bytes.size() + size);

            auto it = bytes.end() - size;
            it = std::copy(l1.begin(), l1.end(), it);
            if (collation_strength::primary < strength) {
                *it++ = 0x0000;
                if (l2_order == l2_weight_order::forward)
                    it = std::copy(l2.begin(), l2.end(), it);
                else
                    it = std::copy(l2.rbegin(), l2.rend(), it);
                if (collation_strength::secondary < strength) {
                    *it++ = 0x0000;
                    it = std::copy(l3.begin(), l3.end(), it);
                    if (collation_strength::tertiary < strength) {
                        *it++ = 0x0000;
                        it = std::copy(l4.begin(), l4.end(), it);
                        if (collation_strength::quaternary < strength) {
                            *it++ = 0x0000;
                            it = std::copy(nfd.begin(), nfd.end(), it);
                        }
                    }
                }
            }
            assert(it == bytes.end());
        }

        template<typename Iter>
        text_sort_key collation_sort_key(
            Iter first,
            Iter last,
            collation_strength strength,
            variable_weighting weighting,
            l2_weight_order l2_order)
        {
            std::vector<uint32_t> bytes;
            container::small_vector<collation_element, 1024> ces;

            // TODO: Try tuning this buffer size for perf.
            std::array<uint32_t, 256> buffer;
            auto buf_it = buffer.begin();
            auto it = first;
            while (it != last) {
                for (; it != last && buf_it != buffer.end(); ++buf_it, ++it) {
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
                s2(buffer.begin(), s2_it, weighting, ces);
                s3(ces,
                   strength,
                   l2_order,
                   first,
                   end_of_raw_input,
                   s2_it - buffer.begin(),
                   bytes);
                ces.clear();
                buf_it = std::copy(s2_it, buf_it, buffer.begin());
                first = end_of_raw_input;
            }
            return text_sort_key(std::move(bytes));
        }

        template<typename Iter1, typename Iter2>
        int collate(
            Iter1 lhs_first,
            Iter1 lhs_last,
            Iter2 rhs_first,
            Iter2 rhs_last,
            collation_strength strength,
            variable_weighting weighting,
            l2_weight_order l2_order)
        {
            // TODO: Do this incrementally, and bail early once the answer
            // is certain.
            // TODO: Do this into stack buffers to avoid allocation for
            // small enough strings.
            text_sort_key const lhs_sk = detail::collation_sort_key(
                lhs_first, lhs_last, strength, weighting, l2_order);
            text_sort_key const rhs_sk = detail::collation_sort_key(
                rhs_first, rhs_last, strength, weighting, l2_order);
            return lhs_sk.compare(rhs_sk);
        }
    }

    /** TODO
        TODO: string -> text
    */
    template<typename Iter>
    text_sort_key collation_sort_key(
        Iter first,
        Iter last,
        collation_strength strength,
        variable_weighting weighting,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        return detail::collation_sort_key(
            first, last, strength, weighting, l2_order);
    }

    /** TODO
        TODO: string -> text
    */
    inline text_sort_key collation_sort_key(
        string const & s,
        collation_strength strength,
        variable_weighting weighting,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        utf32_range as_utf32(s);
        return detail::collation_sort_key(
            as_utf32.begin(), as_utf32.end(), strength, weighting, l2_order);
    }

    /** TODO
        TODO: string -> text
    */
    template<typename Iter1, typename Iter2>
    int collate(
        Iter1 lhs_first,
        Iter1 lhs_last,
        Iter2 rhs_first,
        Iter2 rhs_last,
        collation_strength strength,
        variable_weighting weighting,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        return detail::collate(
            lhs_first,
            lhs_last,
            rhs_first,
            rhs_last,
            strength,
            weighting,
            l2_order);
    }

    /** TODO
        TODO: string -> text
    */
    inline int collate(
        string const & lhs,
        string const & rhs,
        collation_strength strength,
        variable_weighting weighting,
        l2_weight_order l2_order = l2_weight_order::forward)
    {
        utf32_range lhs_as_utf32(rhs);
        utf32_range rhs_as_utf32(lhs);
        return detail::collate(
            lhs_as_utf32.begin(),
            lhs_as_utf32.end(),
            rhs_as_utf32.begin(),
            rhs_as_utf32.end(),
            strength,
            weighting,
            l2_order);
    }

    // TODO: Tailored collation.
}}

#endif
