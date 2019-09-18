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
        template<typename Iter>
        void append(Iter first, Iter last)
        {}
    };

    template<
        bool OnlyContiguous,
        typename CharIter,
        typename Sentinel,
        typename Appender>
    void utf8_normalize_to_nfc_append(
        CharIter first, Sentinel last, Appender & appender)
    {
        nfc_norm().composeUTF8<OnlyContiguous, true>(first, last, appender);
    }
    template<bool OnlyContiguous, typename CharIter, typename Sentinel>
    bool utf8_normalized_nfc(CharIter first, Sentinel last)
    {
        null_appender dummy;
        return nfc_norm().composeUTF8<OnlyContiguous, false>(
            first, last, dummy);
    }

    template<
        bool OnlyContiguous,
        typename Iter,
        typename Sentinel,
        typename Appender>
    void utf16_normalize_to_nfc_append(
        Iter first, Sentinel last, Appender & appender)
    {
        ReorderingBuffer<Appender> buffer(nfc_norm(), appender);
        nfc_norm().compose<OnlyContiguous, true>(first, last, buffer);
    }
    template<bool OnlyContiguous, typename Iter, typename Sentinel>
    bool utf16_normalized_nfc(Iter first, Sentinel last)
    {
        null_appender dummy;
        ReorderingBuffer<null_appender> buffer(nfc_norm(), dummy);
        return nfc_norm().compose<OnlyContiguous, false>(first, last, buffer);
    }

    template<typename Iter, typename Sentinel, typename Appender>
    Iter utf16_normalize_to_nfd_append(
        Iter first, Sentinel last, Appender & appender)
    {
        ReorderingBuffer<Appender> buffer(nfc_norm(), appender);
        return nfc_norm().decompose<true>(first, last, buffer);
    }
    template<typename Iter, typename Sentinel>
    bool utf16_normalized_nfd(Iter first, Sentinel last)
    {
        null_appender dummy;
        ReorderingBuffer<null_appender> buffer(nfc_norm(), dummy);
        return nfc_norm().decompose<false>(first, last, buffer) == last;
    }

    template<typename CharIter, typename Sentinel, typename Appender>
    void utf8_normalize_to_nfkc_append(
        CharIter first, Sentinel last, Appender & appender)
    {
        nfkc_norm().composeUTF8<false, true>(first, last, appender);
    }
    template<typename CharIter, typename Sentinel>
    bool utf8_normalized_nfkc(CharIter first, Sentinel last)
    {
        null_appender dummy;
        return nfkc_norm().composeUTF8<false, false>(first, last, dummy);
    }

    template<typename Iter, typename Sentinel, typename Appender>
    void utf16_normalize_to_nfkc_append(
        Iter first, Sentinel last, Appender & appender)
    {
        ReorderingBuffer<Appender> buffer(nfkc_norm(), appender);
        nfkc_norm().compose<false, true>(first, last, buffer);
    }
    template<typename Iter, typename Sentinel>
    bool utf16_normalized_nfkc(Iter first, Sentinel last)
    {
        null_appender dummy;
        ReorderingBuffer<null_appender> buffer(nfkc_norm(), dummy);
        return nfkc_norm().compose<false, false>(first, last, buffer);
    }

    template<typename Iter, typename Sentinel, typename Appender>
    Iter utf16_normalize_to_nfkd_append(
        Iter first, Sentinel last, Appender & appender)
    {
        ReorderingBuffer<Appender> buffer(nfkc_norm(), appender);
        return nfkc_norm().decompose<true>(first, last, buffer);
    }
    template<typename Iter, typename Sentinel>
    bool utf16_normalized_nfkd(Iter first, Sentinel last)
    {
        null_appender dummy;
        ReorderingBuffer<null_appender> buffer(nfkc_norm(), dummy);
        return nfkc_norm().decompose<false>(first, last, buffer) == last;
    }

    template<typename CharIter, typename Sentinel, typename Appender>
    void utf8_normalize_to_fcc_append(
        CharIter first, Sentinel last, Appender & appender)
    {
        nfc_norm().composeUTF8<true, true>(first, last, appender);
    }
    template<typename CharIter, typename Sentinel>
    bool utf8_normalized_fcc(CharIter first, Sentinel last)
    {
        null_appender dummy;
        return nfc_norm().composeUTF8<true, false>(first, last, dummy);
    }

}}}}

#endif
