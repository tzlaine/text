#ifndef BOOST_TEXT_COLLATE_HPP
#define BOOST_TEXT_COLLATE_HPP

#include <boost/text/collation_data.hpp>
#include <boost/text/string.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/algorithm/cxx14/mismatch.hpp>


namespace boost { namespace text {

    /** TODO */
    struct text_sort_key
    {
        using iterator = std::vector<uint32_t>::const_iterator;

        explicit text_sort_key(
            std::vector<collation_element> const & collation_element)
        {
            // TODO: Totally wrong!
            storage_.resize(collation_elements.size());
            std::transform(
                collation_elements.begin(),
                collation_elements.end(),
                storage_.begin(),
                [](collation_element e) {
                    return uint32_t(e.l1()) << 16 | uint32_t(e.l2()) << 8 |
                           uint32_t(e.l3());
                });
        }

        iterator begin() const noexcept { return sotrage_.begin(); }
        iterator end() const noexcept { return sotrage_.end(); }

        int compare(sort_key const & rhs) const noexcept
        {
            auto const pair = boost::algorithm::mismatch(
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

    private:
        std::vector<uint32_t> storage_;
    };

    // The code in this file implements the UCA as described in
    // http://www.unicode.org/reports/tr10/#Main_Algorithm .  The numbering
    // and some variable naming comes from there.
    namespace detail {

        // http://www.unicode.org/reports/tr10/#Derived_Collation_Elements
        template<typename Iter>
        void
        add_derived_elements(uint32_t cp, std::vector<collation_element> & ces)
        {
            // TODO
        }

        // Does all of S2 except for S2.3, which is handled when building the
        // sort key.
        template<typename Iter>
        void s2(Iter first, Iter last, std::vector<collation_element> & ces)
        {
            // S2.1 Find longest prefix that results in a collation table
            // match.
            auto collation_ = longest_collation(first, last);
            if (!collation_.collation_elements_)
                collation_ = collation(cp);
            if (collation.match_length_ == 0) {
                while (first != last) {
                    add_derived_elements(*first++);
                }
                return;
            }
            auto S_last = first + collation_.match_length_;

            // S2.1.1 Process any nonstarters following S.
            auto nonstarter_last = std::find_if(
                first, last, [](uint32_t cp) { return ccc(cp) == 0; });

            auto nonstarter_first = S_end;
            while (S_last - first <
                       static_cast<int>(collation_weights::max_key_length) &&
                   nonstarter_first != nonstarter_last) {
                if (collation_.trie_node_index_ ==
                    longest_collation_t::invalid_trie_node_index) {
                    assert(collation.match_length_ == 1);
                    uint32_t S_and_C[2] = {*(S_last - 1), *nonstarter_first};
                    auto coll = longest_collation(first, last);
                    if (coll.match_length_ == 2) {
                        S_end = std::rotate(
                            S_end, nonstarter_first, nonstarter_first + 1);
                        collation_ = coll;
                    }
                } else {
                    auto coll =
                        detail::extend_collation(collation_, *nonstarter_first);
                    if (collation_.match_length_ < coll.match_length_) {
                        S_end = std::rotate(
                            S_end, nonstarter_first, nonstarter_first + 1);
                        collation_ = coll;
                    }
                }
                ++nonstarter_first;
            }

            std::copy(
                collation_.collation_elements_.begin(),
                collation_.collation_elements_.end(),
                std::back_inserter(ces));
        }

        inline text_sort_key collation_sort_key(string const & s)
        {
            std::vector<collation_element> ces;
            utf32_range as_utf32(s);
            // TODO: Try tuning this buffer size for perf.
            std::array<uint32_t, 256> buffer;
            while (!as_utf32.empty()) {
                auto it = as_utf32.begin();
                auto out_it = buffer.begin();
                while (it != as_utf32.end() && out_it != buffer.end()) {
                    *out_it++ = *it;
                    ++it;
                }
                // TODO: Only pass the portion of buffer ending at the last
                // ccc == 0.
                s2(buffer.begin(), out_it, ces);
                as_utf32 = utf32_range(it, as_utf32.end());
            }
            return text_sort_key(ces);
        }

        inline int collate(string const & lhs, string const & rhs)
        {
            // TODO: Do this incrementally, and bail early one the answer is
            // certain.
            // TODO: Do this into stack buffers to avoid allocation for small
            // enough strings.
            text_sort_key const lhs_sk = collation_sort_key(lhs);
            text_sort_key const rhs_sk = collation_sort_key(rhs);
            return lhs_sk.compare(rhs.sk);
        }
    }

    /** TODO
        TODO: string -> text
    */
    inline text_sort_key collation_sort_key(string const & s)
    {
        return detail::collation_sort_key(s);
    }

    /** TODO
        TODO: string -> text
    */
    inline int collate(string const & lhs, string const & rhs)
    {
        return detail::collate(lhs, rhs);
    }

    // TODO: Tailored collation.

}}

#endif
