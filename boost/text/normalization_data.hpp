#ifndef BOOST_TEXT_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_NORMALIZATION_DATA_HPP

#include <boost/text/utf8.hpp>

#include <array>


namespace boost { namespace text {

    /** */
    struct canonical_decomposition
    {
        using storage_type = std::array<uint32_t, 4>;
        using iterator = storage_type::const_iterator;

        iterator begin() const { return storage_.begin(); }
        iterator end() const { return storage_.begin() + size_; }

        storage_type storage_;
        int size_;
    };

    /** TODO */
    canonical_decomposition canonical_decompose(uint32_t cp) noexcept;

    /** TODO

        See http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings
        for the source of the "18".
    */
    struct compatible_decomposition
    {
        using storage_type = std::array<uint32_t, 18>;
        using iterator = storage_type::const_iterator;

        iterator begin() const { return storage_.begin(); }
        iterator end() const { return storage_.begin() + size_; }

        storage_type storage_;
        int size_;
    };

    /** Returns the Canonical Combining Class for code point cp. */
    int ccc(uint32_t cp) noexcept;

}}

#endif
