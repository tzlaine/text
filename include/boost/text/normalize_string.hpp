#ifndef BOOST_TEXT_NORMALIZE_STRING_HPP
#define BOOST_TEXT_NORMALIZE_STRING_HPP

#include <boost/text/normalize.hpp>
#include <boost/text/string.hpp>


namespace boost { namespace text {

    /** Puts the contents of <code>s</code> in Unicode normalization form NFD.
        Normalization is not performed if <code>s</code> passes a normalization
        quick-check. */
    inline void normalize_to_nfd(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfd(
            as_utf32.begin(),
            as_utf32.end(),
            utf8::from_utf32_inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of <code>s</code> in Unicode normalization form NFKD.
        Normalization is not performed if <code>s</code> passes a normalization
        quick-check. */
    inline void normalize_to_nfkd(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfkd(
            as_utf32.begin(),
            as_utf32.end(),
            utf8::from_utf32_inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of <code>s</code> in Unicode normalization form NFC.
        Normalization is not performed if <code>s</code> passes a normalization
        quick-check. */
    inline void normalize_to_nfc(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(temp.size());

        normalize_to_nfc(
            as_utf32.begin(),
            as_utf32.end(),
            utf8::from_utf32_inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of <code>s</code> in Unicode normalization form NFKC.
        Normalization is not performed if <code>s</code> passes a normalization
        quick-check. */
    inline void normalize_to_nfkc(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfkc(
            as_utf32.begin(),
            as_utf32.end(),
            utf8::from_utf32_inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of <code>s</code> in normalization form FCC.
       Normalization is not performed if <code>s</code> passes a normalization
       quick-check. */
    inline void normalize_to_fcc(string & s)
    {
        // http://www.unicode.org/notes/tn5/#FCC
        utf32_range as_utf32(s);
        if (fcd_form(as_utf32.begin(), as_utf32.end()))
            return;

        string temp;
        temp.reserve(s.size());

        normalize_to_fcc(
            as_utf32.begin(),
            as_utf32.end(),
            utf8::from_utf32_inserter(temp, temp.end()));

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

}}

#endif
