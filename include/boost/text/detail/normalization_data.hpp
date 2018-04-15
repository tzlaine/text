#ifndef BOOST_TEXT_DETAIL_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_DETAIL_NORMALIZATION_DATA_HPP

#include <boost/text/utf8.hpp>

#include <array>
#include <unordered_map>
#include <unordered_set>


namespace boost { namespace text { namespace detail {

    template<int Capacity>
    struct code_points
    {
        using storage_type = std::array<uint32_t, Capacity>;
        using iterator = typename storage_type::iterator;
        using const_iterator = typename storage_type::const_iterator;

        const_iterator begin() const { return storage_.begin(); }
        const_iterator end() const { return storage_.begin() + size_; }

        iterator begin() { return storage_.begin(); }
        iterator end() { return storage_.begin() + size_; }

        friend bool operator==(
            code_points<Capacity> const & lhs,
            code_points<Capacity> const & rhs)
        {
            return lhs.size_ == rhs.size_ && std::equal(
                                                 lhs.storage_.begin(),
                                                 lhs.storage_.end(),
                                                 rhs.storage_.begin());
        }
        friend bool operator!=(
            code_points<Capacity> const & lhs,
            code_points<Capacity> const & rhs)
        {
            return !(lhs == rhs);
        }

        storage_type storage_;
        int size_;
    };

    using canonical_decomposition = code_points<4>;

    /** See
       http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings for
        the source of the "18".
    */
    using compatible_decomposition = code_points<18>;

    /** The possible results returned by the single code point quick check
        functions.  A result of maybe indicates that a quick check is not
        possible and a full check must be performed. */
    enum class quick_check : uint8_t { yes, no, maybe };

    struct cp_range
    {
        uint16_t first_;
        uint16_t last_;
    };

    BOOST_TEXT_DECL extern uint32_t const * g_all_canonical_decompositions;
    BOOST_TEXT_DECL extern uint32_t const * g_all_compatible_decompositions;

    BOOST_TEXT_DECL std::unordered_map<uint64_t, uint32_t>
    make_composition_map();

    inline std::unordered_map<uint64_t, uint32_t> const & composition_map()
    {
        static std::unordered_map<uint64_t, uint32_t> const retval =
            make_composition_map();
        return retval;
    }

    struct cp_props
    {
        cp_range canonical_decomposition_;
        cp_range compatible_decomposition_;
        uint8_t ccc_;
        uint8_t nfd_quick_check_ : 4;
        uint8_t nfkd_quick_check_ : 4;
        uint8_t nfc_quick_check_ : 4;
        uint8_t nfkc_quick_check_ : 4;
    };

    static_assert(sizeof(cp_props) == 12, "");

    BOOST_TEXT_DECL std::unordered_map<uint32_t, cp_props> make_cp_props_map();

    inline std::unordered_map<uint32_t, cp_props> const & cp_props_map()
    {
        static std::unordered_map<uint32_t, cp_props> const retval =
            make_cp_props_map();
        return retval;
    }

    inline constexpr bool hangul_syllable(uint32_t cp) noexcept
    {
        return 0xAC00 <= cp && cp <= 0xD7A3;
    }

    // Hangul decomposition as described in Unicode 10.0 Section 3.12.
    template<int Capacity>
    inline code_points<Capacity> decompose_hangul_syllable(uint32_t cp) noexcept
    {
        assert(hangul_syllable(cp));

        uint32_t const SBase = 0xAC00;
        uint32_t const LBase = 0x1100;
        uint32_t const VBase = 0x1161;
        uint32_t const TBase = 0x11A7;
        // uint32_t const LCount = 19;
        uint32_t const VCount = 21;
        uint32_t const TCount = 28;
        uint32_t const NCount = VCount * TCount; // 588
        // uint32_t const SCount = LCount * NCount; // 11172

        auto const SIndex = cp - SBase;

        auto const LIndex = SIndex / NCount;
        auto const VIndex = (SIndex % NCount) / TCount;
        auto const TIndex = SIndex % TCount;
        auto const LPart = LBase + LIndex;
        auto const VPart = VBase + VIndex;
        if (TIndex == 0) {
            return code_points<Capacity>{{{LPart, VPart}}, 2};
        } else {
            auto const TPart = TBase + TIndex;
            return code_points<Capacity>{{{LPart, VPart, TPart}}, 3};
        }
    }

    inline constexpr uint64_t key(uint64_t cp0, uint32_t cp1) noexcept
    {
        return (cp0 << 32) | cp1;
    }

    inline canonical_decomposition canonical_decompose(uint32_t cp) noexcept
    {
        if (detail::hangul_syllable(cp))
            return detail::decompose_hangul_syllable<4>(cp);

        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end() || it->second.canonical_decomposition_.last_ ==
                                   it->second.canonical_decomposition_.first_) {
            return canonical_decomposition{{{cp}}, 1};
        }

        canonical_decomposition retval{
            {},
            it->second.canonical_decomposition_.last_ -
                it->second.canonical_decomposition_.first_};
        std::copy(
            detail::g_all_canonical_decompositions +
                it->second.canonical_decomposition_.first_,
            detail::g_all_canonical_decompositions +
                it->second.canonical_decomposition_.last_,
            retval.storage_.begin());

        return retval;
    }

    inline compatible_decomposition compatible_decompose(uint32_t cp) noexcept
    {
        if (detail::hangul_syllable(cp))
            return detail::decompose_hangul_syllable<18>(cp);

        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end() ||
            it->second.compatible_decomposition_.last_ ==
                it->second.compatible_decomposition_.first_) {
            return compatible_decomposition{{{cp}}, 1};
        }

        compatible_decomposition retval{
            {},
            it->second.compatible_decomposition_.last_ -
                it->second.compatible_decomposition_.first_};
        std::copy(
            detail::g_all_compatible_decompositions +
                it->second.compatible_decomposition_.first_,
            detail::g_all_compatible_decompositions +
                it->second.compatible_decomposition_.last_,
            retval.storage_.begin());

        return retval;
    }

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

    inline uint32_t compose_unblocked(uint32_t cp0, uint32_t cp1) noexcept
    {
        auto const & map = detail::composition_map();
        auto const it = map.find(detail::key(cp0, cp1));
        if (it == map.end())
            return 0;
        return it->second;
    }

    inline int ccc(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return 0;
        return it->second.ccc_;
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFD. */
    inline quick_check quick_check_nfd_code_point(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return quick_check::yes;
        return quick_check(it->second.nfd_quick_check_);
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKD. */
    inline quick_check quick_check_nfkd_code_point(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return quick_check::yes;
        return quick_check(it->second.nfkd_quick_check_);
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFC. */
    inline quick_check quick_check_nfc_code_point(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return quick_check::yes;
        return quick_check(it->second.nfc_quick_check_);
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKC. */
    inline quick_check quick_check_nfkc_code_point(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return quick_check::yes;
        return quick_check(it->second.nfkc_quick_check_);
    }

    /** Returns true iff \a cp is a stable code point under FCC normalization
        (meaning that is is ccc=0 and Quick_Check_NFC=Yes).

        \see https://www.unicode.org/reports/tr15/#Stable_Code_Points */
    inline bool stable_fcc_code_point(uint32_t cp) noexcept
    {
        auto const & map = detail::cp_props_map();
        auto const it = map.find(cp);
        if (it == map.end())
            return false;
        return it->second.ccc_ == 0 &&
               quick_check(it->second.nfc_quick_check_) == quick_check::yes;
    }

}}}

#endif
