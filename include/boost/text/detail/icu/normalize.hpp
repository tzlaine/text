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

    template<typename OutIter>
    struct utf8_iter_appender
    {
        explicit utf8_iter_appender(OutIter out) : out_(out) {}

        template<typename CharIter>
        char_iter_ret_t<void, CharIter> append(CharIter first, CharIter last)
        {
            out_ = std::copy(first, last, out_);
        }

    private:
        OutIter out_;
    };

    template<typename CharIter, typename Sentinel, typename Out>
    void utf8_normalize_to_nfc_append(CharIter first, Sentinel last, Out & out)
    {
        nfc_norm().composeUTF8<false, true>(first, last, out);
    }
#if 0
    template<typename CharIter, typename Sentinel>
    void utf8_normalized_nfc(CharIter first, Sentinel last)
    {
        TODO dummy;
        nfc_norm().composeUTF8<false, false>(first, last, dummy);
    }
#endif

    template<typename Iter, typename Sentinel, typename Out>
    void utf16_normalize_to_nfc_append(Iter first, Sentinel last, Out & out)
    {
        ReorderingBuffer<Out> buffer(nfc_norm(), out);
        nfc_norm().compose<false, true>(first, last, buffer);
    }
#if 0
    template<typename Iter, typename Sentinel>
    void utf16_normalized_nfc(Iter first, Sentinel last)
    {
        TODO dummy;
        ReorderingBuffer<TODO> buffer(nfc_norm(), dumy);
        nfc_norm().compose<false, false>(first, last, buffer);
    }
#endif

    template<typename Iter, typename Sentinel, typename Out>
    Iter utf16_normalize_to_nfd_append(Iter first, Sentinel last, Out & out)
    {
        ReorderingBuffer<Out> buffer(nfc_norm(), out);
        return nfc_norm().decompose<true>(first, last, buffer);
    }
#if 0
    template<typename Iter, typename Sentinel>
    Iter utf16_normalized_nfd(Iter first, Sentinel last)
    {
        TODO dummy;
        ReorderingBuffer<TODO> buffer(nfc_norm(), dummy);
        return nfc_norm().decompose<false>(first, last, buffer);
    }
#endif

    template<typename CharIter, typename Sentinel, typename Out>
    void utf8_normalize_to_nfkc_append(CharIter first, Sentinel last, Out & out)
    {
        nfkc_norm().composeUTF8<false, true>(first, last, out);
    }
#if 0
    template<typename CharIter, typename Sentinel>
    void utf8_normalized_nfkc(CharIter first, Sentinel last)
    {
        TODO dummy;
        nfkc_norm().composeUTF8<false, false>(first, last, dummy);
    }
#endif

    template<typename Iter, typename Sentinel, typename Out>
    void utf16_normalize_to_nfkc_append(Iter first, Sentinel last, Out & out)
    {
        ReorderingBuffer<Out> buffer(nfkc_norm(), out);
        nfkc_norm().compose<false, true>(first, last, buffer);
    }
#if 0
    template<typename Iter, typename Sentinel>
    void utf16_normalized_nfkc(Iter first, Sentinel last)
    {
        TODO dummy;
        ReorderingBuffer<TODO> buffer(nfkc_norm(), dummy);
        nfkc_norm().compose<false, false>(first, last, buffer);
    }
#endif

    template<typename Iter, typename Sentinel, typename Out>
    Iter utf16_normalize_to_nfkd_append(Iter first, Sentinel last, Out & out)
    {
        ReorderingBuffer<Out> buffer(nfkc_norm(), out);
        return nfkc_norm().decompose<true>(first, last, buffer);
    }
#if 0
    template<typename Iter, typename Sentinel>
    Iter utf16_normalized_nfkd(Iter first, Sentinel last)
    {
        TODO dummy;
        ReorderingBuffer<TODO> buffer(nfkc_norm(), dummy);
        return nfkc_norm().decompose<false>(first, last, buffer);
    }
#endif

    template<typename CharIter, typename Sentinel, typename Out>
    void utf8_normalize_to_fcc_append(CharIter first, Sentinel last, Out & out)
    {
        nfc_norm().composeUTF8<true, true>(first, last, out);
    }
#if 0
    template<typename CharIter, typename Sentinel>
    void utf8_normalized_fcc(CharIter first, Sentinel last)
    {
        TODO dummy;
        nfc_norm().composeUTF8<true, false>(first, last, dummy);
    }
#endif

    template<typename Iter, typename Sentinel, typename Out>
    void utf16_normalize_to_fcc_append(Iter first, Sentinel last, Out & out)
    {
        ReorderingBuffer<Out> buffer(nfc_norm(), out);
        nfc_norm().compose<true, true>(first, last, buffer);
    }
#if 0
    template<typename Iter, typename Sentinel>
    void utf16_normalized_fcc(Iter first, Sentinel last)
    {
        TODO dummy;
        ReorderingBuffer<TODO> buffer(nfc_norm(), dummy);
        nfc_norm().compose<true, true>(first, last, buffer);
    }
#endif

}}}}

#endif
