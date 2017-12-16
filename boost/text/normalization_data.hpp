#ifndef BOOST_TEXT_NORMALIZATION_DATA_HPP
#define BOOST_TEXT_NORMALIZATION_DATA_HPP


namespace boost { namespace text {

    /** Returns the Canonical Combining Class for code point cp. */
    int ccc(uint32_t cp) noexcept;

}}

#endif
