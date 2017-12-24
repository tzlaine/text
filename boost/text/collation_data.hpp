#ifndef BOOST_TEXT_COLLATION_DATA_HPP
#define BOOST_TEXT_COLLATION_DATA_HPP

#include <boost/text/normalization_data.hpp>
#include <boost/text/collation_weights.hpp>

#include <cstdint>


// TODO: All this stuff should go in detail.
namespace boost { namespace text {

    /** */
    struct collation_element
    {
        constexpr collation_element() noexcept : l1_(), biased_l2_(), l3_() {}
        constexpr collation_element(
            uint16_t l1, uint8_t l2, uint8_t l3) noexcept :
            l1_(l1),
            biased_l2_(l2 - l2_bias),
            l3_(l3)
        {}

        constexpr uint16_t l1() const noexcept { return l1_; }
        constexpr uint8_t l2() const noexcept { return l2_bias + biased_l2_; }
        constexpr uint8_t l3() const noexcept { return l3_; }

    private:
        static constexpr uint8_t l2_bias =
            static_cast<uint8_t>(static_cast<int>(collation_weights::min_l2));
        uint16_t l1_;
        uint8_t biased_l2_;
        uint8_t l3_;
    };

    static_assert(
        sizeof(collation_element) == 4,
        "Oops!  collation_element should be 32 bits.");

#if 0 // TODO
    struct collation_element_range
    {
        collation_element_range(
            collation_element const * f, collation_element const * l) noexcept :
            first_(f),
            last_(l)
        {}

        collation_element const * begin() const noexcept { return first_; }
        collation_element const * end() const noexcept { return last_; }

    private:
        collation_element const * first_;
        collation_element const * last_;
    };
#endif

    struct collation_elements
    {
        using storage_type = std::array<collation_element, 18>;
        using iterator = storage_type::const_iterator;

        iterator begin() const noexcept { return storage_.begin(); }
        iterator end() const noexcept { return storage_.begin() + size_; }

        explicit operator bool() const noexcept { return size_ != 0; }

        storage_type storage_;
        int size_;
    };

    namespace detail {
        collation_elements collation_impl_singleton_1(uint32_t cp) noexcept;
        collation_elements collation_impl_singleton_2(uint32_t cp) noexcept;
        collation_elements
            collation_impl_multiple_cp_key(code_points<4> cps) noexcept;
    }

    /** Returns the collation elements for code point cp0. */
    collation_elements collation(uint32_t cp) noexcept
    {
        if (cp < static_cast<int>(
                     collation_weights::median_sigleton_collation_key)) {
            return detail::collation_impl_singleton_1(cp);
        } else {
            return detail::collation_impl_singleton_2(cp);
        }
    }

    /** Returns the collation elements for code points <cp0, cp1>. */
    collation_elements collation(uint32_t cp0, uint32_t cp1) noexcept
    {
        return detail::collation_impl_multiple_cp_key(
            code_points<4>{{{cp0, cp1}}, 2});
    }

    /** Returns the collation elements for code points <cp0, cp1, cp2>. */
    collation_elements
    collation(uint32_t cp0, uint32_t cp1, uint32_t cp2) noexcept
    {
        return detail::collation_impl_multiple_cp_key(
            code_points<4>{{{cp0, cp1, cp2}}, 3});
    }

    /** Returns the collation elements for code points <cp0, cp1, cp2, cp3>. */
    collation_elements
    collation(uint32_t cp0, uint32_t cp1, uint32_t cp2, uint32_t cp3) noexcept
    {
        return detail::collation_impl_multiple_cp_key(
            code_points<4>{{{cp0, cp1, cp2, cp3}}, 4});
    }

}}

#endif
