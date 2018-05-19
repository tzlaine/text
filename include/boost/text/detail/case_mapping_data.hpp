#ifndef BOOST_TEXT_DETAIL_CASE_MAPPING_DATA_HPP
#define BOOST_TEXT_DETAIL_CASE_MAPPING_DATA_HPP

#include <boost/text/config.hpp>
#include <boost/text/detail/case_constants.hpp>

#include <boost/text/trie.hpp>

#include <unordered_set>


namespace boost { namespace text { namespace detail {

    struct case_cp_indices
    {
        uint16_t first_;
        uint16_t last_;
    };

    struct case_mapping
    {
        case_cp_indices from_;
        case_cp_indices to_;
        uint8_t conditions_;
    };

    struct case_elements
    {
        using iterator = uint32_t const *;

        iterator begin(uint32_t const * elements) const noexcept
        {
            return elements + first_;
        }
        iterator end(uint32_t const * elements) const noexcept
        {
            return elements + last_;
        }

        int size() const noexcept { return last_ - first_; }
        explicit operator bool() const noexcept { return first_ != last_; }

        uint16_t first_;
        uint16_t last_;
        uint8_t conditions_;
    };

    inline bool operator==(case_elements lhs, case_elements rhs) noexcept
    {
        return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
    }
    inline bool operator!=(case_elements lhs, case_elements rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<int N>
    struct case_trie_key
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

        constexpr case_trie_key() noexcept : cps_(), size_(0) {}
        constexpr case_trie_key(storage_t cps, int size) noexcept :
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

        friend bool operator==(case_trie_key lhs, case_trie_key rhs)
        {
            return lhs.size_ == rhs.size_ && std::equal(
                                                 lhs.cps_.values_,
                                                 lhs.cps_.values_ + lhs.size_,
                                                 rhs.cps_.values_);
        }

        friend bool operator!=(case_trie_key lhs, case_trie_key rhs)
        {
            return !(lhs == rhs);
        }
    };

    using case_trie_t = trie::trie<case_trie_key<3>, case_elements>;
    using case_trie_match_t = case_trie_t::match_result;

    BOOST_TEXT_DECL case_trie_t make_to_lower_trie();
    BOOST_TEXT_DECL case_trie_t make_to_title_trie();
    BOOST_TEXT_DECL case_trie_t make_to_upper_trie();
    BOOST_TEXT_DECL std::vector<uint32_t> make_soft_dotted_cps();
    BOOST_TEXT_DECL std::vector<uint32_t> make_soft_dotted_cps();
    BOOST_TEXT_DECL std::unordered_set<uint32_t> make_cased_cps();
    BOOST_TEXT_DECL std::unordered_set<uint32_t> make_case_ignorable_cps();
    BOOST_TEXT_DECL extern uint32_t const * g_case_cps_first;

    inline case_trie_t const & to_lower_trie()
    {
        static case_trie_t const retval = make_to_lower_trie();
        return retval;
    }

    inline case_trie_t const & to_title_trie()
    {
        static case_trie_t const retval = make_to_title_trie();
        return retval;
    }

    inline case_trie_t const & to_upper_trie()
    {
        static case_trie_t const retval = make_to_upper_trie();
        return retval;
    }

    inline bool cased(uint32_t cp)
    {
        static std::unordered_set<uint32_t> const cps = make_cased_cps();
        return 0 < cps.count(cp);
    }

    inline bool case_ignorable(uint32_t cp)
    {
        static std::unordered_set<uint32_t> const cps =
            make_case_ignorable_cps();
        return 0 < cps.count(cp);
    }

    inline bool soft_dotted(uint32_t cp)
    {
        static std::vector<uint32_t> const cps = make_soft_dotted_cps();
        auto const it = std::lower_bound(cps.begin(), cps.end(), cp);
        return it != cps.end() && *it == cp;
    }

}}}

#endif
