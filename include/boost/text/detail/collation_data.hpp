#ifndef BOOST_TEXT_DETAIL_COLLATION_DATA_HPP
#define BOOST_TEXT_DETAIL_COLLATION_DATA_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/trie_map.hpp>
#include <boost/text/detail/collation_constants.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/optional.hpp>

#include <algorithm>

#include <cstdint>

#ifndef BOOST_TEXT_COLLATION_DATA_INSTRUMENTATION
#define BOOST_TEXT_COLLATION_DATA_INSTRUMENTATION 0
#endif
#if BOOST_TEXT_COLLATION_DATA_INSTRUMENTATION
#include <boost/container/small_vector.hpp>
#endif


namespace boost { namespace text { namespace detail {

    struct collation_element
    {
        uint32_t l1_;
        uint16_t l2_;
        uint16_t l3_;
        uint32_t l4_;
    };

#if BOOST_TEXT_COLLATION_DATA_INSTRUMENTATION
    inline std::ostream & operator<<(std::ostream & os, collation_element ce)
    {
        return os << std::hex << "[" << ce.l1_ << " " << ce.l2_ << " " << ce.l3_
                  << " " << ce.l4_ << "]" << std::dec;
    }
    inline std::ostream & operator<<(
        std::ostream & os,
        container::small_vector<detail::collation_element, 1024> const & ces)
    {
        for (auto ce : ces) {
            os << ce << " ";
        }
        return os;
    }
#endif

    inline bool
    operator==(collation_element lhs, collation_element rhs) noexcept
    {
        return lhs.l1_ == rhs.l1_ && lhs.l2_ == rhs.l2_ && lhs.l3_ == rhs.l3_ &&
               lhs.l4_ == rhs.l4_;
    }
    inline bool
    operator!=(collation_element lhs, collation_element rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool operator<(collation_element lhs, collation_element rhs) noexcept
    {
        if (rhs.l1_ < lhs.l1_)
            return false;
        if (lhs.l1_ < rhs.l1_)
            return true;

        if (rhs.l2_ < lhs.l2_)
            return false;
        if (lhs.l2_ < rhs.l2_)
            return true;

        if (rhs.l3_ < lhs.l3_)
            return false;
        if (lhs.l3_ < rhs.l3_)
            return true;

        return lhs.l4_ < rhs.l4_;
    }
    inline bool
    operator<=(collation_element lhs, collation_element rhs) noexcept
    {
        return lhs == rhs || lhs < rhs;
    }

    inline collation_strength ce_strength(collation_element ce) noexcept
    {
        if (ce.l1_)
            return collation_strength::primary;
        if (ce.l2_)
            return collation_strength::secondary;
        if (ce.l3_)
            return collation_strength::tertiary;
        if (ce.l4_)
            return collation_strength::quaternary;
        return collation_strength::identical;
    }

    BOOST_TEXT_DECL extern std::array<collation_element, 39258>
    make_collation_elements();

    inline std::array<collation_element, 39258> const & collation_elements_()
    {
        static auto const retval = make_collation_elements();
        return retval;
    }

    inline collation_element const * collation_elements_ptr()
    {
        return &collation_elements_()[0];
    }

    struct collation_elements
    {
        using iterator = collation_element const *;

        iterator begin(collation_element const * elements) const noexcept
        {
            return elements + first_;
        }
        iterator end(collation_element const * elements) const noexcept
        {
            return elements + last_;
        }

        int size() const noexcept { return last_ - first_; }
        explicit operator bool() const noexcept { return first_ != last_; }

        uint16_t first_;
        uint16_t last_;
    };

    inline bool
    operator==(collation_elements lhs, collation_elements rhs) noexcept
    {
        return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
    }
    inline bool
    operator!=(collation_elements lhs, collation_elements rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<int N>
    struct collation_trie_key
    {
        using iterator = uint32_t *;
        using const_iterator = uint32_t const *;
        using value_type = uint32_t;

        struct storage_t
        {
            constexpr storage_t() noexcept : values_{} {}
            constexpr storage_t(uint32_t x) noexcept : values_{x} {}
            constexpr storage_t(uint32_t x, uint32_t y) noexcept : values_{x, y}
            {}
            constexpr storage_t(uint32_t x, uint32_t y, uint32_t z) noexcept :
                values_{x, y, z}
            {}
            uint32_t values_[N];
        };

        constexpr collation_trie_key() noexcept : cps_(), size_(0) {}
        constexpr collation_trie_key(storage_t cps, int size) noexcept :
            cps_(cps),
            size_(size)
        {}

        const_iterator begin() const noexcept { return cps_.values_; }
        const_iterator end() const noexcept { return begin() + size_; }

        iterator begin() noexcept { return cps_.values_; }
        iterator end() noexcept { return begin() + size_; }

        iterator insert(iterator at, uint32_t cp) noexcept
        {
            assert(at == end());
            assert(size_ < N);
            *at = cp;
            ++size_;
            return at;
        }

        storage_t cps_;
        int size_;

        friend bool operator==(collation_trie_key lhs, collation_trie_key rhs)
        {
            return lhs.size_ == rhs.size_ && std::equal(
                                                 lhs.cps_.values_,
                                                 lhs.cps_.values_ + lhs.size_,
                                                 rhs.cps_.values_);
        }

        friend bool operator!=(collation_trie_key lhs, collation_trie_key rhs)
        {
            return !(lhs == rhs);
        }
    };

    using collation_trie_t =
        trie::trie_map<collation_trie_key<32>, collation_elements>;
    using trie_match_t = collation_trie_t::match_result;
    using trie_iterator_t = collation_trie_t::iterator;
    using const_trie_iterator_t = collation_trie_t::const_iterator;

    BOOST_TEXT_DECL extern std::array<collation_trie_key<3>, 38593>
    make_trie_keys();
    BOOST_TEXT_DECL extern std::array<collation_elements, 38593>
    make_trie_values();

    inline std::array<collation_trie_key<3>, 38593> const & trie_keys()
    {
        static auto const retval = make_trie_keys();
        return retval;
    }
    inline std::array<collation_elements, 38593> const & trie_values()
    {
        static auto const retval = make_trie_values();
        return retval;
    }

    struct reorder_group
    {
        string_view name_;
        collation_element first_;
        collation_element last_;
        bool simple_;
        bool compressible_;
    };

    inline bool operator==(reorder_group lhs, reorder_group rhs) noexcept
    {
        return lhs.name_ == rhs.name_ && lhs.first_ == rhs.first_ &&
               lhs.last_ == rhs.last_ && lhs.simple_ == rhs.simple_ &&
               lhs.compressible_ == rhs.compressible_;
    }

    BOOST_TEXT_DECL extern std::array<reorder_group, 140> make_reorder_groups();

    inline std::array<reorder_group, 140> const & reorder_groups()
    {
        static auto const retval = make_reorder_groups();
        return retval;
    }

    inline optional<reorder_group> find_reorder_group(string_view name) noexcept
    {
        if (name == "Hrkt")
            name = "Hira";
        if (name == "Kana")
            name = "Hira";
        if (name == "Hans")
            name = "Hani";
        if (name == "Hant")
            name = "Hani";
        for (auto group : reorder_groups()) {
            if (group.name_ == name)
                return group;
        }
        return {};
    }

}}}

#endif
