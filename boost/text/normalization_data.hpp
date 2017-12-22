#ifndef BOOST_TEXT_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_NORMALIZATION_DATA_HPP

#include <boost/text/utf8.hpp>

#include <array>


// TODO: All this stuff should go in detail.
namespace boost { namespace text {

    // TODO: Experiment with changing these *_decomposition types to be a pair
    // of iterators as an optimization.

    inline constexpr std::size_t
    hash_combine(std::size_t seed, std::size_t x) noexcept
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    inline constexpr std::size_t hash(uint32_t x)
    {
        return hash_combine(0, x);
    }

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
        using iterator = typename storage_type::const_iterator;

        iterator begin() const { return storage_.begin(); }
        iterator end() const { return storage_.begin() + size_; }

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

    /** TODO */
    canonical_decomposition canonical_decompose(uint32_t cp) noexcept;

    /** TODO

        See
       http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings for
       the source of the "18".
    */
    using compatible_decomposition = code_points<18>;

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
