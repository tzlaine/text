#ifndef BOOST_TEXT_DETAIL_ICU_NORMALIZE_HPP
#define BOOST_TEXT_DETAIL_ICU_NORMALIZE_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/detail/icu/normalizer2impl.hpp>
#include <boost/text/detail/icu/norm2_nfc_data.hpp>
#include <boost/text/detail/icu/norm2_nfkc_data.hpp>


namespace boost { namespace text { namespace detail { namespace icu {

    inline Normalizer2Impl & nfc_norm()
    {
        static Normalizer2Impl retval;
        static bool once = true;
        if (once) {
            retval.init(
                norm2_nfc_data_indexes,
                &norm2_nfc_data_trie,
                norm2_nfc_data_extraData,
                norm2_nfc_data_smallFCD);
            once = false;
        }
        return retval;
    }

    inline Normalizer2Impl & nfkc_norm()
    {
        static Normalizer2Impl retval;
        static bool once = true;
        if (once) {
            retval.init(
                norm2_nfkc_data_indexes,
                &norm2_nfkc_data_trie,
                norm2_nfkc_data_extraData,
                norm2_nfkc_data_smallFCD);
            once = false;
        }
        return retval;
    }

    template<typename String>
    struct utf8_string_appender
    {
        explicit utf8_string_appender(String & s) : s_(&s) {}

        template<typename CharIter>
        char_iter_ret_t<void, CharIter> append(CharIter first, CharIter last)
        {
            s_->insert(s_->end(), first, last);
        }

    private:
        String * s_;
    };

    template<typename UTF32OutIter>
    struct utf8_to_utf32_appender
    {
        explicit utf8_to_utf32_appender(UTF32OutIter out) : out_(out) {}

        template<typename CharIter>
        char_iter_ret_t<void, CharIter> append(CharIter first, CharIter last)
        {
            out_ = transcode_utf_8_to_32(first, last, out_);
        }

        UTF32OutIter out() const { return out_; }

    private:
        UTF32OutIter out_;
    };

    struct null_appender
    {
        null_appender(bool = true) {}
        template<typename Iter>
        void append(Iter first, Iter last)
        {}
        bool out() const { return true; }
    };

}}}}

#endif
