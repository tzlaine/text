#ifndef BOOST_TEXT_IN_PLACE_CASE_MAPPING_HPP
#define BOOST_TEXT_IN_PLACE_CASE_MAPPING_HPP

#include <boost/text/text.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/case_mapping.hpp>


namespace boost { namespace text {

    /** Changes the case of t to lower-case, using language-specific handling
        as indicated by lang. */
    inline void in_place_to_lower(
        text & t, case_language lang = case_language::other) noexcept
    {
        string s;
        to_lower(
            t.begin().base(),
            t.begin().base(),
            t.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang);
        if (s.size() < t.storage_bytes()) {
            t = s;
        } else {
            normalize_to_fcc(s);
            t.replace(std::move(s));
        }
    }

    /** Changes the case of r to lower-case, using language-specific handling
        as indicated by lang. */
    inline void in_place_to_lower(
        rope & r, case_language lang = case_language::other) noexcept
    {
        string s;
        to_lower(
            r.begin().base(),
            r.begin().base(),
            r.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang);
        r = s;
    }

    /** Changes the case of t to title-case, using language-specific handling
        as indicated by lang. */
    template<typename NextWordBreakFunc = next_word_break_callable>
    void in_place_to_title(
        text & t,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
    {
        string s;
        to_title(
            t.begin().base(),
            t.begin().base(),
            t.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang,
            next_word_break);
        if (s.size() < t.storage_bytes()) {
            t = s;
        } else {
            normalize_to_fcc(s);
            t.replace(std::move(s));
        }
    }

    /** Changes the case of r to title-case, using language-specific handling
        as indicated by lang. */
    template<typename NextWordBreakFunc = next_word_break_callable>
    void in_place_to_title(
        rope & r,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
    {
        string s;
        to_title(
            r.begin().base(),
            r.begin().base(),
            r.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang,
            next_word_break);
        r = s;
    }

    /** Changes the case of t to upper-case, using language-specific handling
        as indicated by lang. */
    inline void in_place_to_upper(
        text & t, case_language lang = case_language::other) noexcept
    {
        string s;
        to_upper(
            t.begin().base(),
            t.begin().base(),
            t.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang);
        if (s.size() < t.storage_bytes()) {
            t = s;
        } else {
            normalize_to_fcc(s);
            t.replace(std::move(s));
        }
    }

    /** Changes the case of r to upper-case, using language-specific handling
        as indicated by lang. */
    inline void in_place_to_upper(
        rope & r, case_language lang = case_language::other) noexcept
    {
        string s;
        to_upper(
            r.begin().base(),
            r.begin().base(),
            r.end().base(),
            utf8::from_utf32_inserter(s, s.end()),
            lang);
        r = s;
    }

}}

#endif
