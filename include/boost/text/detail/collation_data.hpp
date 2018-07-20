#ifndef BOOST_TEXT_DETAIL_COLLATION_DATA_HPP
#define BOOST_TEXT_DETAIL_COLLATION_DATA_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/trie_map.hpp>
#include <boost/text/detail/collation_constants.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>

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

    BOOST_TEXT_DECL extern std::array<collation_element, 39258> const &
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
        // string_view is not constexpr constructible from a string literal in
        // C++11; C++14 constexpr support is required.
        char const * name_;
        collation_element first_;
        collation_element last_;
        bool simple_;
        bool compressible_;
    };

    inline bool operator==(reorder_group lhs, reorder_group rhs) noexcept
    {
        return !strcmp(lhs.name_, rhs.name_) && lhs.first_ == rhs.first_ &&
               lhs.last_ == rhs.last_ && lhs.simple_ == rhs.simple_ &&
               lhs.compressible_ == rhs.compressible_;
    }

    BOOST_TEXT_DECL extern std::array<reorder_group, 140> const &
    make_reorder_groups();

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

    inline uint32_t bytes_to_uint32_t(unsigned char const * chars)
    {
        return chars[0] << 24 | chars[1] << 16 | chars[2] << 8 | chars[3] << 0;
    }

    inline uint32_t bytes_to_cp(unsigned char const * chars)
    {
        return chars[0] << 16 | chars[1] << 8 | chars[2] << 0;
    }

    inline uint32_t bytes_to_uint16_t(unsigned char const * chars)
    {
        return chars[0] << 8 | chars[1] << 0;
    }

    template<typename OutIter>
    struct lzw_to_trie_key_iter
    {
        using value_type = collation_trie_key<3>;
        using difference_type = int;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::output_iterator_tag;
        using buffer_t = container::small_vector<unsigned char, 256>;

        lzw_to_trie_key_iter(OutIter out, buffer_t & buf) :
            out_(out),
            buf_(&buf),
            element_bytes_(0)
        {}

        template<typename BidiRange>
        lzw_to_trie_key_iter & operator=(BidiRange const & r)
        {
            buf_->insert(buf_->end(), r.rbegin(), r.rend());
            while (4 <= (int)buf_->size()) {
                if (!element_bytes_)
                    element_bytes_ = buf_->front() * 3 + 1;
                if ((int)buf_->size() < element_bytes_)
                    break;
                collation_trie_key<3> element;
                for (int i = 1; i != element_bytes_; i += 3) {
                    element.insert(
                        element.end(), bytes_to_cp(buf_->data() + i));
                }
                *out_++ = element;
                buf_->erase(buf_->begin(), buf_->begin() + element_bytes_);
                element_bytes_ = 0;
            }
            return *this;
        }
        lzw_to_trie_key_iter & operator*() { return *this; }
        lzw_to_trie_key_iter & operator++() { return *this; }
        lzw_to_trie_key_iter & operator++(int) { return *this; }

    private:
        OutIter out_;
        buffer_t * buf_;
        int element_bytes_;
    };
    template<typename OutIter>
    lzw_to_trie_key_iter<OutIter> make_lzw_to_trie_key_iter(
        OutIter out, container::small_vector<unsigned char, 256> & buf)
    {
        return lzw_to_trie_key_iter<OutIter>(out, buf);
    }

    template<typename OutIter>
    struct lzw_to_trie_value_iter
    {
        using value_type = collation_elements;
        using difference_type = int;
        using pointer = unsigned char *;
        using reference = unsigned char &;
        using iterator_category = std::output_iterator_tag;
        using buffer_t = container::small_vector<unsigned char, 256>;

        lzw_to_trie_value_iter(OutIter out, buffer_t & buf) :
            out_(out),
            buf_(&buf)
        {}

        template<typename BidiRange>
        lzw_to_trie_value_iter & operator=(BidiRange const & r)
        {
            buf_->insert(buf_->end(), r.rbegin(), r.rend());
            auto const element_bytes = 4;
            auto it = buf_->begin();
            for (auto end = buf_->end() - buf_->size() % element_bytes;
                 it != end;
                 it += element_bytes) {
                collation_elements element;
                element.first_ = bytes_to_uint16_t(&*it);
                element.last_ = bytes_to_uint16_t(&*it + 2);
                *out_++ = element;
            }
            buf_->erase(buf_->begin(), it);
            return *this;
        }
        lzw_to_trie_value_iter & operator*() { return *this; }
        lzw_to_trie_value_iter & operator++() { return *this; }
        lzw_to_trie_value_iter & operator++(int) { return *this; }

    private:
        OutIter out_;
        buffer_t * buf_;
    };
    template<typename OutIter>
    lzw_to_trie_value_iter<OutIter> make_lzw_to_trie_value_iter(
        OutIter out, container::small_vector<unsigned char, 256> & buf)
    {
        return lzw_to_trie_value_iter<OutIter>(out, buf);
    }

    enum : uint16_t { no_predecessor = 0xffff, no_value = 0xffff };

    struct lzw_reverse_table_element
    {
        uint16_t pred_ = no_predecessor;
        uint16_t value_ = no_value;
    };

    using lzw_reverse_table = std::vector<lzw_reverse_table_element>;

    template<typename OutIter>
    OutIter
    copy_table_entry(lzw_reverse_table const & table, uint16_t i, OutIter out)
    {
        *out++ = table[i].value_;
        while (table[i].pred_ != no_predecessor) {
            i = table[i].pred_;
            *out++ = table[i].value_;
        }
        return out;
    }

    // Hardcoded to 16 bits.  Takes 16-bit values as input and writes the
    // decoded unsigned char values to out.
    template<typename Iter, typename OutIter>
    OutIter lzw_decompress(Iter first, Iter last, OutIter out)
    {
        lzw_reverse_table reverse_table(1 << 16);
        for (uint16_t i = 0; i < 256u; ++i) {
            reverse_table[i].value_ = i;
        }

        container::small_vector<unsigned char, 256> table_entry;

        uint32_t next_table_value = 256;
        uint32_t const end_table_value = 1 << 16;

        uint16_t prev_code = *first++;
        assert(prev_code < 256);
        unsigned char c = (unsigned char)prev_code;
        table_entry.push_back(c);
        *out++ = table_entry;

        while (first != last) {
            uint16_t const code = *first++;

            table_entry.clear();
            if (reverse_table[code].value_ == no_value) {
                table_entry.push_back(c);
                copy_table_entry(
                    reverse_table,
                    prev_code,
                    std::back_inserter(table_entry));
            } else {
                copy_table_entry(
                    reverse_table,
                    code,
                    std::back_inserter(table_entry));
            }

            *out++ = table_entry;
            c = table_entry.back();

            if (next_table_value < end_table_value)
                reverse_table[next_table_value++] = {prev_code, c};

            prev_code = code;
        }

        return out;
    }

}}}

#endif
