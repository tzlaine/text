#ifndef BOOST_TEXT_COLLATION_DATA_HPP
#define BOOST_TEXT_COLLATION_DATA_HPP

#include <boost/text/normalization_data.hpp>
#include <boost/text/collation_weights.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <cstdint>


namespace boost { namespace text {

    /** */
    struct compressed_collation_element
    {
        uint16_t l1() const noexcept { return l1_; }
        uint8_t l2() const noexcept { return uint8_t(l2_bias + biased_l2_); }
        uint8_t l3() const noexcept { return l3_; }

        static constexpr uint8_t l2_bias =
            static_cast<uint8_t>(static_cast<int>(collation_weights::min_l2));

        uint16_t l1_;
        uint8_t biased_l2_;
        uint8_t l3_;
    };

    inline bool operator==(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return lhs.l1_ == rhs.l1_ && lhs.biased_l2_ == rhs.biased_l2_ &&
               lhs.l3_ == rhs.l3_;
    }
    inline bool operator!=(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return !(lhs == rhs);
    }

    static_assert(
        sizeof(compressed_collation_element) == 4,
        "Oops!  compressed_collation_element should be 32 bits.");

    /** */
    struct collation_element
    {
        uint16_t l1_;
        uint16_t l2_;
        uint16_t l3_;
        uint16_t l4_;
        uint32_t identical_;
    };

    inline collation_element
    to_collation_element(compressed_collation_element ce)
    {
        return collation_element{ce.l1(), ce.l2(), ce.l3()};
    }

    inline collation_element to_collation_element(
        compressed_collation_element ce, uint16_t l4, uint32_t cp = 0)
    {
        return collation_element{ce.l1(), ce.l2(), ce.l3(), l4, cp};
    }

    namespace detail {
        extern compressed_collation_element const * g_collation_elements_first;
    }

    struct compressed_collation_elements
    {
        using iterator = compressed_collation_element const *;

        iterator begin() const noexcept
        {
            return detail::g_collation_elements_first + first_;
        }
        iterator end() const noexcept
        {
            return detail::g_collation_elements_first + last_;
        }

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

    namespace detail {
        struct collation_trie_node;

        extern collation_trie_node const * g_collation_trie_nodes;

        struct collation_trie_node
        {
            using iterator = collation_trie_node const *;

            iterator begin() const noexcept
            {
                return g_collation_trie_nodes + first_child_;
            }
            iterator end() const noexcept
            {
                return g_collation_trie_nodes + last_child_;
            }

            bool match() const noexcept
            {
                return static_cast<bool>(collation_elements_);
            }

            bool leaf() const noexcept { return first_child_ == last_child_; }

            uint32_t cp_;

            uint16_t first_child_;
            uint16_t last_child_;

            // Only nonempty when this is the end of a match.
            compressed_collation_elements collation_elements_;
        };

        inline bool operator==(collation_trie_node lhs, collation_trie_node rhs)
        {
            return lhs.cp_ == rhs.cp_;
        }
        inline bool operator!=(collation_trie_node lhs, collation_trie_node rhs)
        {
            return !(lhs == rhs);
        }
    }
}}

namespace std {
    template<>
    struct hash<boost::text::detail::collation_trie_node>
    {
        typedef boost::text::detail::collation_trie_node argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const & n) const noexcept
        {
            return n.cp_;
        }
    };
}

namespace boost { namespace text {

    /** TODO */
    struct longest_collation_t
    {
        detail::collation_trie_node node_;
        int match_length_;

        static const uint16_t invalid_trie_node_index = 0xffff;
    };

    namespace detail {

        extern std::unordered_set<detail::collation_trie_node> const
            g_collation_initial_nodes;

        inline collation_trie_node const * find_trie_node(
            collation_trie_node const * first,
            collation_trie_node const * last,
            uint32_t cp) noexcept
        {
            collation_trie_node const to_find{cp};
            auto it = std::lower_bound(
                first,
                last,
                to_find,
                [](collation_trie_node const & lhs,
                   collation_trie_node const & rhs) {
                    return lhs.cp_ < rhs.cp_;
                });
            if (it != last && it->cp_ != cp)
                it = last;
            return it;
        }

        inline longest_collation_t
        extend_collation(longest_collation_t prev, uint32_t cp) noexcept
        {
            auto first = g_collation_trie_nodes + prev.node_.first_child_;
            auto last = g_collation_trie_nodes + prev.node_.last_child_;
            auto const node_it = find_trie_node(first, last, cp);
            if (node_it == last || !node_it->match())
                return prev;
            return longest_collation_t{*node_it, prev.match_length_ + 1};
        }
    }

    /** TODO */
    template<typename Iter>
    longest_collation_t longest_collation(Iter first, Iter last) noexcept
    {
        auto it = first;
        auto const starter = *it++;
        detail::collation_trie_node node{starter};
        auto hash_it = detail::g_collation_initial_nodes.find(node);
        if (hash_it == detail::g_collation_initial_nodes.end())
            return longest_collation_t{{}, 0};
        node = *hash_it;

        longest_collation_t retval{node, node.collation_elements_ ? 1 : 0};
        while (it != last && !node.leaf()) {
            auto const node_it = detail::find_trie_node(
                detail::g_collation_trie_nodes + node.first_child_,
                detail::g_collation_trie_nodes + node.last_child_,
                *it++);
            if (node_it ==
                detail::g_collation_trie_nodes + node.last_child_)
                break;
            node = *node_it;
            if (node.match()) {
                retval.node_ = node;
                retval.match_length_ = it - first;
            }
        }
        return retval;
    }

}}

#endif
