#ifndef BOOST_TEXT_DETAIL_COLLATION_DATA_HPP
#define BOOST_TEXT_DETAIL_COLLATION_DATA_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/trie.hpp>
#include <boost/text/detail/collation_constants.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/optional.hpp>

#include <algorithm>

#include <cstdint>


namespace boost { namespace text { namespace detail {

    struct compressed_collation_element
    {
        uint32_t l1() const noexcept { return l1_; }
        uint16_t l2() const noexcept { return l2_; }
        uint8_t l3() const noexcept { return l3_; }

        uint32_t l1_;
        uint16_t l2_;
        uint8_t l3_;
    };

    inline bool operator==(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return lhs.l1_ == rhs.l1_ && lhs.l2_ == rhs.l2_ && lhs.l3_ == rhs.l3_;
    }
    inline bool operator!=(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return !(lhs == rhs);
    }
    inline bool operator<(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        if (rhs.l1_ < lhs.l1_)
            return false;
        if (lhs.l1_ < rhs.l1_)
            return true;

        if (rhs.l2_ < lhs.l2_)
            return false;
        if (lhs.l2_ < rhs.l2_)
            return true;

        return lhs.l3_ < rhs.l3_;
    }
    inline bool operator<=(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return lhs == rhs || lhs < rhs;
    }

    static_assert(
        sizeof(compressed_collation_element) == 8,
        "Oops!  compressed_collation_element should be 64 bits.");

    struct collation_element
    {
        uint32_t l1_;
        uint16_t l2_;
        uint8_t l3_;
        uint32_t l4_;
    };

    extern compressed_collation_element const * g_collation_elements_first;

    struct compressed_collation_elements
    {
        using iterator = compressed_collation_element const *;

        iterator begin(compressed_collation_element const * elements) const
            noexcept
        {
            return elements + first_;
        }
        iterator end(compressed_collation_element const * elements) const
            noexcept
        {
            return elements + last_;
        }

        int size() const noexcept { return last_ - first_; }
        explicit operator bool() const noexcept { return first_ != last_; }

        uint16_t first_;
        uint16_t last_;
    };

    inline bool operator==(
        compressed_collation_elements lhs, compressed_collation_elements rhs)
    {
        return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
    }
    inline bool operator!=(
        compressed_collation_elements lhs, compressed_collation_elements rhs)
    {
        return !(lhs == rhs);
    }

    struct collation_trie_key
    {
        using iterator = uint32_t *;
        using const_iterator = uint32_t const *;
        using value_type = uint32_t;

        struct storage_t
        {
            constexpr storage_t() :values_{} {}
            constexpr storage_t(uint32_t x) :values_{x}{}
            constexpr storage_t(uint32_t x, uint32_t y)  :values_{x,y}{}
            constexpr storage_t(uint32_t x, uint32_t y, uint32_t z) :values_{x,y,z} {}
            uint32_t values_[3];
        };

        constexpr collation_trie_key() : cps_(), size_(0) {}
        constexpr collation_trie_key(storage_t cps, int size) : cps_(cps), size_(size) {}

        const_iterator begin() const noexcept { return cps_.values_; }
        const_iterator end() const noexcept { return begin() + size_; }

        iterator begin() noexcept { return cps_.values_; }
        iterator end() noexcept { return begin() + size_; }

        iterator insert(iterator at, uint32_t cp)
        {
            assert(at == end());
            *at = cp;
            ++size_;
            return at;
        }

        storage_t cps_;
        int size_;
    };

    using collation_trie_t =
        trie::trie<collation_trie_key, compressed_collation_elements>;
    using trie_match_t = trie::
        trie_match_result<collation_trie_key, compressed_collation_elements>;
    using const_trie_iterator_t = trie::
        const_trie_iterator<collation_trie_key, compressed_collation_elements>;

    extern collation_trie_t const g_default_collation_trie;

    struct reorder_group
    {
        string_view name_;
        compressed_collation_element first_;
        compressed_collation_element last_;
        bool simple_;
        bool compressible_;
    };

    inline bool operator==(reorder_group lhs, reorder_group rhs)
    {
        return lhs.name_ == rhs.name_ && lhs.first_ == rhs.first_ &&
               lhs.last_ == rhs.last_ && lhs.simple_ == rhs.simple_ &&
               lhs.compressible_ == rhs.compressible_;
    }

    extern std::array<reorder_group, 140> const g_reorder_groups;

    inline optional<reorder_group> find_reorder_group(string_view name)
    {
        if (name == "Hrkt")
            name = "Hira";
        if (name == "Kana")
            name = "Hira";
        if (name == "Hans")
            name = "Hani";
        if (name == "Hant")
            name = "Hani";
        for (auto group : g_reorder_groups) {
            if (group.name_ == name)
                return group;
        }
        return {};
    }

}}}

#endif
