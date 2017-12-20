#ifndef BOOST_TEXT_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_NORMALIZATION_DATA_HPP

#include <boost/text/utf8.hpp>

#include <array>


namespace boost { namespace text {

    // TODO: Experiment with changing these *_decomposition types to be a pair
    // of iterators and a size as an optimization.

    /** */
    struct canonical_decomposition
    {
        using storage_type = std::array<uint32_t, 4>;
        using iterator = storage_type::const_iterator;

        iterator begin() const { return storage_.begin(); }
        iterator end() const { return storage_.begin() + size_; }

        storage_type storage_;
        int size_;
    };

    /** TODO */
    canonical_decomposition canonical_decompose(uint32_t cp) noexcept;

    /** TODO

        See http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings
        for the source of the "18".
    */
    struct compatible_decomposition
    {
        using storage_type = std::array<uint32_t, 18>;
        using iterator = storage_type::const_iterator;

        iterator begin() const { return storage_.begin(); }
        iterator end() const { return storage_.begin() + size_; }

        storage_type storage_;
        int size_;
    };

    /** TODO */
    compatible_decomposition compatible_decompose(uint32_t cp) noexcept;

    /** TODO */
    inline uint32_t
    compose_hangul(uint32_t cp0, uint32_t cp1, uint32_t cp2 = 0) noexcept
    {
        uint32_t const SBase = 0xAC00;
        uint32_t const LBase = 0x1100;
        uint32_t const VBase = 0x1161;
        uint32_t const TBase = 0x11A7;
        // uint32_t const LCount = 19;
        uint32_t const VCount = 21;
        uint32_t const TCount = 28;
        uint32_t const NCount = VCount * TCount; // 588
        // uint32_t const SCount = LCount * NCount; // 11172

        auto const LIndex = cp0 - LBase;
        auto const VIndex = cp1 - VBase;

        auto const LVIndex = LIndex * NCount + VIndex * TCount;
        if (cp2) {
            auto const TIndex = cp2 - TBase;
            return SBase + LVIndex + TIndex;
        } else {
            return SBase + LVIndex;
        }
    }

    /** TODO */
    uint32_t compose_unblocked(uint32_t cp0, uint32_t cp1) noexcept;

    /** Returns the Canonical Combining Class for code point cp. */
    int ccc(uint32_t cp) noexcept;

    /** The possible results returned by the single code point quick check
        functions.  A result of maybe indicates that a quick check is not
        possible and a full check must be performed. */
    enum class quick_check { yes, no, maybe };

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFD. */
    quick_check quick_check_nfd_code_point(uint32_t cp) noexcept;

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKD. */
    quick_check quick_check_nfkd_code_point(uint32_t cp) noexcept;

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFC. */
    quick_check quick_check_nfc_code_point(uint32_t cp) noexcept;

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKC. */
    quick_check quick_check_nfkc_code_point(uint32_t cp) noexcept;

}}

#endif
