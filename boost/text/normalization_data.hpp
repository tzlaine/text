#ifndef BOOST_TEXT_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_NORMALIZATION_DATA_HPP

#include <boost/text/utf8.hpp>

#include <array>
#include <unordered_map>
#include <unordered_set>


// TODO: All this stuff should go in detail.
namespace boost { namespace text {

    // TODO: Experiment with changing these *_decomposition types to be a pair
    // of iterators as an optimization.

    inline constexpr std::size_t
    hash_combine(std::size_t seed, std::size_t x) noexcept
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    inline constexpr std::size_t hash(uint32_t x) { return hash_combine(0, x); }

    inline constexpr std::size_t hash(uint32_t x, uint32_t y)
    {
        return hash_combine(x, y);
    }

    inline constexpr std::size_t
    hash(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        return hash_combine(hash_combine(x, y), z);
    }

    inline constexpr std::size_t
    hash(uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept
    {
        return hash_combine(hash_combine(hash_combine(x, y), z), w);
    }

    /** */
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

    /** */
    using canonical_decomposition = code_points<4>;

    /** TODO

        See
       http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings for
       the source of the "18".
    */
    using compatible_decomposition = code_points<18>;

    /** The possible results returned by the single code point quick check
        functions.  A result of maybe indicates that a quick check is not
        possible and a full check must be performed. */
    enum class quick_check { yes, no, maybe };

    namespace detail {

        struct cp_range
        {
            uint16_t first_;
            uint16_t last_;
        };

        extern uint32_t const * g_all_canonical_decompositions;
        extern std::unordered_map<uint32_t, cp_range> const
            g_canonical_decomposition_map;
        extern uint32_t const * g_all_compatible_decompositions;
        extern std::unordered_map<uint32_t, cp_range> const
            g_compatible_decomposition_map;
        extern std::unordered_map<uint64_t, uint32_t> const g_composition_map;
        extern std::unordered_map<uint32_t, int> const g_ccc_map;

        extern std::unordered_set<uint32_t> const g_nfd_quick_check_set;
        extern std::unordered_set<uint32_t> const g_nfkd_quick_check_set;
        extern std::unordered_map<uint32_t, quick_check> const
            g_nfc_quick_check_map;
        extern std::unordered_map<uint32_t, quick_check> const
            g_nfkc_quick_check_map;

        inline constexpr bool hangul_syllable(uint32_t cp) noexcept
        {
            return 0xAC00 <= cp && cp <= 0xD7A3;
        }

        // Hangul decomposition as described in Unicode 10.0 Section 3.12.
        template<int Capacity>
        inline code_points<Capacity>
        decompose_hangul_syllable(uint32_t cp) noexcept
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
    }

    /** TODO */
    inline canonical_decomposition canonical_decompose(uint32_t cp) noexcept
    {
        if (detail::hangul_syllable(cp))
            return detail::decompose_hangul_syllable<4>(cp);

        auto const it = detail::g_canonical_decomposition_map.find(cp);
        if (it == detail::g_canonical_decomposition_map.end())
            return canonical_decomposition{{{cp}}, 1};
        canonical_decomposition retval{{},
                                       it->second.last_ - it->second.first_};
        std::copy(
            detail::g_all_canonical_decompositions + it->second.first_,
            detail::g_all_canonical_decompositions + it->second.last_,
            retval.storage_.begin());
        return retval;
    }

    /** TODO */
    inline compatible_decomposition compatible_decompose(uint32_t cp) noexcept
    {
        if (detail::hangul_syllable(cp))
            return detail::decompose_hangul_syllable<18>(cp);

        auto const it = detail::g_compatible_decomposition_map.find(cp);
        if (it == detail::g_compatible_decomposition_map.end())
            return compatible_decomposition{{{cp}}, 1};
        compatible_decomposition retval{{},
                                        it->second.last_ - it->second.first_};
        std::copy(
            detail::g_all_compatible_decompositions + it->second.first_,
            detail::g_all_compatible_decompositions + it->second.last_,
            retval.storage_.begin());
        return retval;
    }

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
    inline uint32_t compose_unblocked(uint32_t cp0, uint32_t cp1) noexcept
    {
        auto const it = detail::g_composition_map.find(detail::key(cp0, cp1));
        if (it == detail::g_composition_map.end())
            return 0;
        return it->second;
    }

    /** Returns the Canonical Combining Class for code point cp. */
    inline int ccc(uint32_t cp) noexcept
    {
        auto const it = detail::g_ccc_map.find(cp);
        if (it == detail::g_ccc_map.end())
            return 0;
        return it->second;
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFD. */
    inline quick_check quick_check_nfd_code_point(uint32_t cp) noexcept
    {
        auto const it = detail::g_nfd_quick_check_set.find(cp);
        if (it == detail::g_nfd_quick_check_set.end())
            return quick_check::yes;
        return quick_check::no;
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKD. */
    inline quick_check quick_check_nfkd_code_point(uint32_t cp) noexcept
    {
        auto const it = detail::g_nfkd_quick_check_set.find(cp);
        if (it == detail::g_nfkd_quick_check_set.end())
            return quick_check::yes;
        return quick_check::no;
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFC. */
    inline quick_check quick_check_nfc_code_point(uint32_t cp) noexcept
    {
        auto const it = detail::g_nfc_quick_check_map.find(cp);
        if (it == detail::g_nfc_quick_check_map.end())
            return quick_check::yes;
        return it->second;
    }

    /** Returns yes, no, or maybe if the given code point indicates that the
        sequence in which it is found is normalized NFKC. */
    inline quick_check quick_check_nfkc_code_point(uint32_t cp) noexcept
    {
        auto const it = detail::g_nfkc_quick_check_map.find(cp);
        if (it == detail::g_nfkc_quick_check_map.end())
            return quick_check::yes;
        return it->second;
    }

}}

namespace std {
    template<int Size>
    struct hash<boost::text::code_points<Size>>
    {
        typedef boost::text::code_points<Size> argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const & cps) const noexcept
        {
            result_type retval = 0;
            for (int i = 0, end = cps.size_; i < end; ++i) {
                retval = boost::text::hash_combine(retval, cps.storage_[i]);
            }
            return retval;
        }
    };
}

#endif
