#ifndef BOOST_TEXT_DETAIL_ICU_NORMALIZE_HPP
#define BOOST_TEXT_DETAIL_ICU_NORMALIZE_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/detail/icu/normalizer2impl.hpp>
#include <boost/text/detail/icu/norm2_nfc_data.hpp>
#include <boost/text/detail/icu/norm2_nfkc_data.hpp>

#include "bytestream.hpp"


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

    // TODO: nfkc

    // TODO: Remove.
    template<typename String>
    struct string_sink : ByteSink
    {
        explicit string_sink(String & s) : s_(&s) {}

        virtual void Append(const char * bytes, int32_t n) override
        {
            s_->insert(s_->end(), bytes, bytes + n);
        }

    private:
        String * s_;
    };

    template<typename String>
    struct string_appender
    {
        explicit string_appender(String & s) : s_(&s) {}

        template<typename CharIter>
        void append(CharIter bytes, int32_t n)
        {
            s_->insert(s_->end(), bytes, bytes + n);
        }

    private:
        String * s_;
    };

    template<typename Iter>
    struct iter_appender
    {
        explicit iter_appender(Iter out) : out_(out) {}

        template<typename CharIter>
        void append(CharIter bytes, int32_t n)
        {
            out_ = std::copy(bytes, bytes + n, out_);
        }

    private:
        Iter out_;
    };

    template<typename CharIter, typename Sentinel, typename String>
    inline void
    utf8_normalize_to_nfc_append(CharIter first, Sentinel last, String & s)
    {
        UErrorCode ec = U_ZERO_ERROR;
#if 0
        nfc_norm().composeUTF8<false, true>(
            first, last, string_appender<String>(s), ec);
#else
        string_sink<String> sink(s);
        nfc_norm().composeUTF8(
            0, false, (uint8_t const *)first, (uint8_t const *)last, &sink, ec);
#endif
        BOOST_ASSERT(U_SUCCESS(ec));
    }

    template<typename CharIter, typename Sentinel, typename String>
    inline void
    utf8_normalize_to_fcc_append(CharIter first, Sentinel last, String & s)
    {
        UErrorCode ec = U_ZERO_ERROR;
#if 0
        nfc_norm().composeUTF8<true, true>(
#if 1
            first,
            last,
#else
            (uint8_t const *)first,
            (uint8_t const *)last,
#endif
            string_appender<String>(s),
            ec);
#else
        string_sink<String> sink(s);
        nfc_norm().composeUTF8(
            0, true, (uint8_t const *)first, (uint8_t const *)last, &sink, ec);
#endif
        BOOST_ASSERT(U_SUCCESS(ec));
    }

}}}}

#endif
