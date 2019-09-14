#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/container/static_vector.hpp>

#if BOOST_TEXT_HAS_ICU
#include <unicode/normalizer2.h>
#endif

#include <algorithm>


namespace boost { namespace text {

    namespace detail {

        template<typename Iter, std::size_t Capacity>
        void order_canonically(
            Iter first,
            Iter last,
            container::static_vector<int, Capacity> & cccs) noexcept
        {
            BOOST_ASSERT(first != last);

            std::transform(first, last, cccs.begin(), ccc);

            --last;
            while (first != last) {
                auto it = first;
                auto new_last = first;
                auto ccc_it = cccs.begin();
                while (it != last) {
                    auto next = std::next(it);
                    auto ccc_next = std::next(ccc_it);
                    auto const ccc_a = *ccc_it;
                    auto const ccc_b = *ccc_next;
                    if (0 < ccc_b && ccc_b < ccc_a) {
                        std::iter_swap(it, next);
                        std::iter_swap(ccc_it, ccc_next);
                        new_last = it;
                    }
                    ++it;
                    ++ccc_it;
                }
                last = new_last;
            }
        }

        template<std::size_t Capacity, typename FlushFunc>
        bool flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc & flush)
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                order_canonically(buffer.begin(), buffer.end(), cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<
            typename Iter,
            typename Sentinel,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename FlushFunc>
        bool normalize_to_decomposed_impl(
            Iter first,
            Sentinel last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            FlushFunc && flush)
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                if (!ccc(decomp.storage_[0])) {
                    if (!detail::flush_buffer(buffer, flush))
                        return false;
                }
                buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                ++first;
            }
            if (!detail::flush_buffer(buffer, flush))
                return false;
            return true;
        }

        template<
            typename Iter,
            typename Sentinel,
            typename OutIter,
            typename DecomposeFunc>
        OutIter normalize_to_decomposed(
            Iter first, Sentinel last, OutIter out, DecomposeFunc && decompose)
        {
            container::static_vector<uint32_t, 64> buffer;
            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_decomposed_impl(
                first,
                last,
                buffer,
                decompose,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(first, last, out);
                    return true;
                });

            return out;
        }

        inline constexpr bool hangul_l(uint32_t cp) noexcept
        {
            return 0x1100 <= cp && cp <= 0x1112;
        }
        inline constexpr bool hangul_v(uint32_t cp) noexcept
        {
            return 0x1161 <= cp && cp <= 0x1175;
        }
        inline constexpr bool hangul_t(uint32_t cp) noexcept
        {
            return 0x11a8 <= cp && cp <= 0x11c2;
        }

        template<bool DisallowDiscontiguous, std::size_t Capacity>
        void compose(
            container::static_vector<uint32_t, Capacity> & buffer,
            container::static_vector<int, Capacity> & cccs)
        {
            BOOST_ASSERT(buffer.size() == cccs.size());
            BOOST_ASSERT(2 <= buffer.size());

            auto starter_it = buffer.begin();
            auto it = std::next(buffer.begin());
            auto ccc_it = std::next(cccs.begin());
            while (it != buffer.end()) {
                // Hangul composition as described in Unicode 11.0 Section 3.12.
                auto const hangul_cp0 = *starter_it;
                auto const hangul_cp1 = *it;
                if (it == starter_it + 1 && hangul_l(hangul_cp0) &&
                    hangul_v(hangul_cp1)) {
                    auto const cp2_it = it + 1;
                    auto const hangul_cp2 =
                        cp2_it == buffer.end() ? 0 : *cp2_it;
                    if (hangul_t(hangul_cp2)) {
                        *starter_it =
                            compose_hangul(hangul_cp0, hangul_cp1, hangul_cp2);
                        buffer.erase(it, it + 2);
                        cccs.erase(ccc_it, ccc_it + 2);
                    } else {
                        *starter_it = compose_hangul(hangul_cp0, hangul_cp1);
                        buffer.erase(it, it + 1);
                        cccs.erase(ccc_it, ccc_it + 1);
                    }
                } else {
                    auto const prev_ccc = *std::prev(ccc_it);
                    auto const ccc = *ccc_it;
                    uint32_t composition = 0;
                    if ((it == starter_it + 1 ||
                         (!DisallowDiscontiguous &&
                          (prev_ccc != 0 && prev_ccc < ccc))) &&
                        (composition = compose_unblocked(*starter_it, *it))) {
                        *starter_it = composition;
                        buffer.erase(it);
                        cccs.erase(ccc_it);
                    } else {
                        ++it;
                        ++ccc_it;
                        if (it == buffer.end() &&
                            starter_it < buffer.end() - 2) {
                            ++starter_it;
                            it = std::next(starter_it);
                            ccc_it = cccs.begin() + (it - buffer.begin());
                        }
                    }
                }
            }
        }

        template<
            bool DisallowDiscontiguous,
            std::size_t Capacity,
            typename FlushFunc>
        bool compose_and_flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc &&
                flush) noexcept(noexcept(flush(buffer.begin(), buffer.end())))
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                order_canonically(buffer.begin(), buffer.end(), cccs);
            if (2 <= buffer.size())
                compose<DisallowDiscontiguous>(buffer, cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<std::size_t Capacity>
        bool hangul_final_v(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return !buffer.empty() && hangul_l(buffer.back()) && hangul_v(cp);
        }

        template<std::size_t Capacity>
        bool hangul_final_t(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return 2 <= buffer.size() && hangul_l(buffer[buffer.size() - 2]) &&
                   hangul_v(buffer.back()) && hangul_t(cp);
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            typename Sentinel,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename QuickCheckFunc,
            typename FlushFunc>
        bool normalize_to_composed_impl(
            Iter first,
            Sentinel last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_,
            FlushFunc && flush) noexcept
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                auto const it = std::find_if(
                    decomp.begin(), decomp.end(), [&quick_check_](uint32_t cp) {
                        return !ccc(cp) && quick_check_(cp) == quick_check::yes;
                    });
                if (it != decomp.end() && !hangul_final_v(buffer, *it) &&
                    !hangul_final_t(buffer, *it)) {
                    buffer.insert(buffer.end(), decomp.begin(), it);
                    if (!detail::compose_and_flush_buffer<
                            DisallowDiscontiguous>(buffer, flush)) {
                        return false;
                    }
                    buffer.insert(buffer.end(), it, decomp.end());
                } else {
                    buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                }
                ++first;
            }
            if (!detail::compose_and_flush_buffer<DisallowDiscontiguous>(
                    buffer, flush)) {
                return false;
            }
            return true;
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            typename Sentinel,
            typename OutIter,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        OutIter normalize_to_composed(
            Iter first,
            Sentinel last,
            OutIter out,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_)
        {
            container::static_vector<uint32_t, 64> buffer;

            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            normalize_to_composed_impl<DisallowDiscontiguous>(
                first,
                last,
                buffer,
                decompose,
                quick_check_,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(first, last, out);
                    return true;
                });

            return out;
        }

#if 0
        // NOTE: The logic in
        // http://www.unicode.org/reports/tr15/tr15-45.html#Detecting_Normalization_Forms
        // seems to indicate that if a supplementary code point is encountered
        // in normalized_quick_check(), then we should proceed as normal for
        // this iteration, but then do a double increment of the loop control
        // variable.  That looks wrong, so I'm leaving that out for now.
        bool supplemantary(uint32_t cp)
        {
            return 0x10000 <= cp && cp <= 0x10FFFF;
        }
#endif

        template<typename Iter, typename Sentinel, typename QuickCheckFunc>
        quick_check normalized_quick_check(
            Iter first, Sentinel last, QuickCheckFunc && quick_check_) noexcept
        {
            quick_check retval = quick_check::yes;
            int prev_ccc = 0;
            while (first != last) {
                auto const cp = *first;
#if 0
                // See note above.
                if (supplemantary(cp))
                    ++first;
#endif
                auto const check = quick_check_(cp);
                if (check == quick_check::no)
                    return quick_check::no;
                if (check == quick_check::maybe)
                    retval = quick_check::maybe;
                auto const ccc_ = ccc(cp);
                if (ccc_ && ccc_ < prev_ccc)
                    return quick_check::no;
                prev_ccc = ccc_;
                ++first;
            }
            return retval;
        }

        template<
            typename Iter,
            typename Sentinel,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        bool normalized_decomposed(
            Iter first,
            Sentinel last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return normalize_to_decomposed_impl(
                    first,
                    last,
                    buffer,
                    decompose,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }

        template<
            typename Iter,
            typename Sentinel,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        bool normalized_composed(
            Iter first,
            Sentinel last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return normalize_to_composed_impl<false>(
                    first,
                    last,
                    buffer,
                    decompose,
                    quick_check_,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }

        template<typename Iter>
        using char_ptr = std::integral_constant<
            bool,
            std::is_pointer<Iter>::value &&
                std::is_same<
                    typename std::remove_const<
                        typename std::remove_pointer<Iter>::type>::type,
                    char>::value>;

        template<typename Iter>
        using char_ptr_base = char_ptr<decltype(std::declval<Iter>().base())>;

        template<typename Iter>
        struct char_ptr_or_base_char_ptr
            : std::integral_constant<
                  bool,
                  char_ptr<Iter>::value
                      ? true
                      : detected_or<std::false_type, char_ptr_base, Iter>::
                            value>
        {
        };

        template<typename Iter>
        using char_writable_expr = decltype(*std::declval<Iter &>() = char());

        template<typename Iter>
        using char_iterator_expr =
            std::is_same<typename std::iterator_traits<Iter>::value_type, char>;

        template<typename Iter>
        using char_container_expr =
            std::is_same<typename Iter::container_type::value_type, char>;

        template<typename Iter>
        using base_char_container_expr = std::is_same<
            typename decltype(
                std::declval<Iter>().base())::container_type::value_type,
            char>;

        template<typename Iter>
        struct char_out_iter
            : std::integral_constant<
                  bool,
                  is_detected<char_writable_expr, Iter>::value &&
                      (detected_or<std::false_type, char_iterator_expr, Iter>::
                           value ||
                       detected_or<std::false_type, char_container_expr, Iter>::
                           value ||
                       detected_or<
                           std::false_type,
                           base_char_container_expr,
                           Iter>::value)>
        {
        };

        template<typename Iter, typename Sentinel>
        struct icu_utf8_in_fast_path
            : std::integral_constant<
                  bool,
                  char_ptr_or_base_char_ptr<Iter>::value>
        {
        };

        template<typename Iter, typename Sentinel, typename OutIter>
        struct icu_utf8_inout_fast_path
            : std::integral_constant<
                  bool,
                  char_ptr_or_base_char_ptr<Iter>::value &&
                      char_out_iter<OutIter>::value>
        {
        };

#if BOOST_TEXT_HAS_ICU
        template<typename OutIter, typename OriginalIter>
        void self_assign_it_(OutIter & out, OriginalIter *& it)
        {}
        template<typename OutIter>
        void self_assign_it_(OutIter & out, OutIter *& it)
        {
            it = &out;
        }

        template<typename OutIter, typename OriginalIter>
        struct out_iter_sink : U_NAMESPACE_QUALIFIER ByteSink
        {
            explicit out_iter_sink(OutIter out) : out_(out), it_(&out_) {}
            out_iter_sink(OutIter out, OriginalIter & it) : out_(out), it_(&it)
            {}

            out_iter_sink(out_iter_sink const & other) :
                out_(other.out_),
                it_(other.it_)
            {
                self_assign_it_(out_, it_);
            }
            out_iter_sink & operator=(out_iter_sink const & other)
            {
                out_ = other.out_;
                it_ = other.it_;
                self_assign_it_(out_, it_);
            }

            virtual void Append(char const * bytes, int32_t n) override
            {
                out_ = std::copy(bytes, bytes + n, out_);
            }

            OriginalIter iter() const { return *it_; }

        private:
            OutIter out_;
            OriginalIter * it_;
        };

        template<typename OutIter>
        out_iter_sink<OutIter, OutIter> make_byte_sink(OutIter out)
        {
            return out_iter_sink<OutIter, OutIter>(out);
        }
        template<typename C>
        auto make_byte_sink(utf_32_to_8_insert_iterator<C> & out) -> decltype(
            out_iter_sink<decltype(out.base()), utf_32_to_8_insert_iterator<C>>(
                out.base(), out))
        {
            return out_iter_sink<
                decltype(out.base()),
                utf_32_to_8_insert_iterator<C>>(out.base(), out);
        }
        template<typename C>
        auto make_byte_sink(utf_32_to_8_back_insert_iterator<C> & out)
            -> decltype(out_iter_sink<
                        decltype(out.base()),
                        utf_32_to_8_back_insert_iterator<C>>(out.base(), out))
        {
            return out_iter_sink<
                decltype(out.base()),
                utf_32_to_8_back_insert_iterator<C>>(out.base(), out);
        }

        template<typename Iter>
        typename std::enable_if<
            char_ptr<Iter>::value,
            U_NAMESPACE_QUALIFIER StringPiece>::type
        make_string_piece(Iter first, Iter last)
        {
            return U_NAMESPACE_QUALIFIER StringPiece(first, last - first);
        }
        template<typename Iter, typename Sentinel>
        typename std::enable_if<
            char_ptr<Iter>::value,
            U_NAMESPACE_QUALIFIER StringPiece>::type
        make_string_piece(Iter first, Sentinel last)
        {
            return U_NAMESPACE_QUALIFIER StringPiece(first);
        }
        template<typename Iter>
        U_NAMESPACE_QUALIFIER StringPiece make_string_piece(
            utf_8_to_32_iterator<Iter> first, utf_8_to_32_iterator<Iter> last)
        {
            return U_NAMESPACE_QUALIFIER StringPiece(
                first.base(), last.base() - first.base());
        }
        template<typename Iter, typename Sentinel>
        U_NAMESPACE_QUALIFIER StringPiece make_string_piece(
            utf_8_to_32_iterator<Iter, Sentinel> first, Sentinel last)
        {
            return U_NAMESPACE_QUALIFIER StringPiece(first.base());
        }
        template<typename Iter, typename Sentinel>
        U_NAMESPACE_QUALIFIER StringPiece
        make_string_piece(utf_8_to_32_iterator<Iter> first, Sentinel last)
        {
            return U_NAMESPACE_QUALIFIER StringPiece(first.base());
        }

        template<
            typename Iter,
            typename Sentinel,
            typename OutIter,
            typename Comp>
        auto fill_trailing_noncombiners(
            Iter & it, Sentinel last, OutIter out, OutIter out_last, Comp comp)
        {
            static uint16_t const high_surrogate_base = 0xd7c0;
            static uint16_t const low_surrogate_base = 0xdc00;

            while (!comp(it, last)) {
                BOOST_ASSERT(out != out_last); // Safe stream format assumption.
                auto const cu = *it;
                if (text::high_surrogate(cu)) {
                    auto next = std::next(it);
                    if (comp(next, last)) {
                        *out++ = cu;
                        break;
                    }
                    auto const lo = *next;
                    uint32_t cp = (cu - high_surrogate_base) << 10;
                    cp += lo - low_surrogate_base;
                    if (!ccc(cp))
                        break;
                    *out++ = cu;
                    BOOST_ASSERT(out != out_last);
                    *out++ = lo;
                    it = next;
                    ++it;
                } else {
                    if (!ccc(cu))
                        break;
                    *out++ = cu;
                    ++it;
                }
            }

            return out;
        }

        template<std::ptrdiff_t N, typename Iter, typename Sentinel>
        auto fill_buffer_to_last_noncombiner(
            Iter & first, Sentinel last, char16_t * out, char16_t * out_last)
            -> _8_iter_ret_t<char16_t *, Iter>
        {
            static_assert(64 + 1 <= N, "Safe-stream format");
            std::ptrdiff_t count = 0;
            auto it = text::make_utf_8_to_16_iterator(first, first, last);
            char16_t prev_cu = 0;
            for (; it.base() != last && count < N - 64 - 1;
                 ++it, ++out, ++count) {
                prev_cu = *it;
                *out = prev_cu;
            }
            if (it.base() != last && high_surrogate(prev_cu)) {
                *out++ = *it;
                ++it;
            }
            out = fill_trailing_noncombiners(
                it, last, out, out_last, [](decltype(it) it, Sentinel last) {
                    return it.base() == last;
                });
            first = it.base();
            return out;
        }

        template<std::ptrdiff_t N, typename Iter, typename Sentinel>
        auto fill_buffer_to_last_noncombiner(
            Iter & first, Sentinel last, char16_t * out, char16_t * out_last)
            -> _16_iter_ret_t<char16_t *, Iter>
        {
            static_assert(64 + 1 <= N, "Safe-stream format");
            std::ptrdiff_t count = 0;
            char16_t prev_cu = 0;
            for (; first != last && count < N - 64 - 1;
                 ++first, ++out, ++count) {
                prev_cu = *first;
                *out = prev_cu;
            }
            if (first != last && high_surrogate(prev_cu)) {
                *out++ = *first;
                ++first;
            }
            out = fill_trailing_noncombiners(
                first, last, out, out_last, [](Iter it, Sentinel last) {
                    return it == last;
                });
            return out;
        }

        template<std::ptrdiff_t N, typename CPIter, typename Sentinel>
        auto fill_buffer_to_last_noncombiner(
            CPIter & first, Sentinel last, char16_t * out, char16_t * out_last)
            -> cp_iter_ret_t<char16_t *, CPIter>
        {
            static_assert(64 + 1 <= N, "Safe-stream format");
            std::ptrdiff_t count = 0;
            char16_t prev_cu = 0;
            auto it = text::make_utf_32_to_16_iterator(first, first, last);
            for (; it.base() != last && count < N - 64 - 1;
                 ++it, ++out, ++count) {
                prev_cu = *it;
                *out = prev_cu;
            }
            if (it.base() != last && high_surrogate(prev_cu)) {
                *out++ = *it;
                ++it;
            }
            out = fill_trailing_noncombiners(
                it, last, out, out_last, [](decltype(it) it, Sentinel last) {
                    return it.base() == last;
                });
            first = it.base();
            return out;
        }

        template<
            typename Iter,
            typename Sentinel,
            bool FastUTF8 = icu_utf8_in_fast_path<Iter, Sentinel>::value>
        struct icu_normalized
        {
            static bool call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last)
            {
                int const chunk_size = 1024 << 2;
                U_NAMESPACE_QUALIFIER UnicodeString chunk;
                while (first != last) {
                    char16_t * const chunk_first = chunk.getBuffer(chunk_size);
                    char16_t * const chunk_last =
                        fill_buffer_to_last_noncombiner<chunk_size>(
                            first, last, chunk_first, chunk_first + chunk_size);
                    chunk.releaseBuffer(chunk_last - chunk_first);
                    UErrorCode ec = U_ZERO_ERROR;
                    auto const result = norm.isNormalized(chunk, ec);
                    BOOST_ASSERT(U_SUCCESS(ec));
                    if (!result)
                        return false;
                }
                return true;
            }
        };

        template<typename Iter, typename Sentinel>
        struct icu_normalized<Iter, Sentinel, true>
        {
            static bool call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last)
            {
                UErrorCode ec = U_ZERO_ERROR;
                auto const retval = norm.isNormalizedUTF8(
                    detail::make_string_piece(first, last), ec);
                BOOST_ASSERT(U_SUCCESS(ec));
                return retval;
            }
        };

        template<
            typename Iter,
            typename Sentinel,
            typename OutIter,
            bool FastUTF8 =
                icu_utf8_inout_fast_path<Iter, Sentinel, OutIter>::value>
        struct icu_normalize
        {
            static OutIter call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last,
                OutIter out)
            {
                int const chunk_size = 1024 << 2;
                U_NAMESPACE_QUALIFIER UnicodeString chunk;
                U_NAMESPACE_QUALIFIER UnicodeString result;
                while (first != last) {
                    char16_t * const chunk_first = chunk.getBuffer(chunk_size);
                    char16_t * const chunk_last =
                        fill_buffer_to_last_noncombiner<chunk_size>(
                            first, last, chunk_first, chunk_first + chunk_size);
                    chunk.releaseBuffer(chunk_last - chunk_first);
                    UErrorCode ec = U_ZERO_ERROR;
                    norm.normalize(chunk, result, ec);
                    BOOST_ASSERT(U_SUCCESS(ec));
                    out = text::transcode_utf_16_to_32(
                        result.getBuffer(),
                        result.getBuffer() + result.length(),
                        out);
                }
                return out;
            }
        };

        template<typename Iter, typename Sentinel, typename OutIter>
        struct icu_normalize<Iter, Sentinel, OutIter, true>
        {
            static OutIter call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last,
                OutIter out)
            {
                UErrorCode ec = U_ZERO_ERROR;
                auto sink = detail::make_byte_sink(out);
                norm.normalizeUTF8(
                    0,
                    detail::make_string_piece(first, last),
                    sink,
                    nullptr,
                    ec);
                BOOST_ASSERT(U_SUCCESS(ec));
                return sink.iter();
            }
        };
#else
        template<typename Iter>
        typename std::enable_if<char_ptr<Iter>::value, string_view>::type
        make_string_view(Iter first, Iter last)
        {
            return string_view(first, last - first);
        }
        template<typename Iter, typename Sentinel>
        typename std::enable_if<char_ptr<Iter>::value, string_view>::type
        make_string_view(Iter first, Sentinel last)
        {
            return string_view(first);
        }
        template<typename Iter>
        string_view make_string_view(
            utf_8_to_32_iterator<Iter> first, utf_8_to_32_iterator<Iter> last)
        {
            return string_view(first.base(), last.base() - first.base());
        }
        template<typename Iter, typename Sentinel>
        string_view make_string_view(
            utf_8_to_32_iterator<Iter, Sentinel> first, Sentinel last)
        {
            return string_view(first.base());
        }
        template<typename Iter, typename Sentinel>
        string_view
        make_string_view(utf_8_to_32_iterator<Iter> first, Sentinel last)
        {
            return string_view(first.base());
        }
#endif
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFD to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfd(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFDInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalize<CPIter, Sentinel, OutIter>::call(
                *norm, first, last, out);
        }
#endif
        return detail::normalize_to_decomposed(
            first, last, out, [](uint32_t cp) {
                return detail::canonical_decompose(cp);
            });
    }

    /** Writes sequence `r` in Unicode normalization form NFD to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfd(CPRange const & r, OutIter out)
    {
        return normalize_to_nfd(std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFKD to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfkd(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKDInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalize<CPIter, Sentinel, OutIter>::call(
                *norm, first, last, out);
        }
#endif
        return detail::normalize_to_decomposed(
            first, last, out, [](uint32_t cp) {
                return detail::compatible_decompose(cp);
            });
    }

    /** Writes sequence `r` in Unicode normalization form NFKD to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfkd(CPRange const & r, OutIter out)
    {
        return normalize_to_nfkd(std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFC to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFCInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalize<CPIter, Sentinel, OutIter>::call(
                *norm, first, last, out);
        }
#endif
        return detail::normalize_to_composed<false>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** Writes sequence `r` in Unicode normalization form NFC to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfc(CPRange const & r, OutIter out)
    {
        return normalize_to_nfc(std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFKC to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfkc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKCInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalize<CPIter, Sentinel, OutIter>::call(
                *norm, first, last, out);
        }
#endif
        return detail::normalize_to_composed<false>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });
    }

    /** Writes sequence `r` in Unicode normalization form NFKC to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfkc(CPRange const & r, OutIter out)
    {
        return normalize_to_nfkc(std::begin(r), std::end(r), out);
    }

    /** Returns true iff the given sequence of code points is normalized NFD.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfd(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFDInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalized<CPIter, Sentinel>::call(
                *norm, first, last);
        }
#endif
        return detail::normalized_decomposed(
            first,
            last,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfd_code_point(cp); });
    }

    /** Returns true iff the given range of code points is normalized NFD. */
    template<typename CPRange>
    bool normalized_nfd(CPRange const & r) noexcept
    {
        return normalized_nfd(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFKD.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfkd(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKDInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalized<CPIter, Sentinel>::call(
                *norm, first, last);
        }
#endif
        return detail::normalized_decomposed(
            first,
            last,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkd_code_point(cp);
            });
    }

    /** Returns true iff the given range of code points is normalized NFKD. */
    template<typename CPRange>
    bool normalized_nfkd(CPRange const & r) noexcept
    {
        return normalized_nfkd(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFC.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfc(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFCInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalized<CPIter, Sentinel>::call(
                *norm, first, last);
        }
#endif
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** Returns true iff the given range of code points is normalized NFC. */
    template<typename CPRange>
    bool normalized_nfc(CPRange const & r) noexcept
    {
        return normalized_nfc(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFKC.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfkc(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKCInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalized<CPIter, Sentinel>::call(
                *norm, first, last);
        }
#endif
        return detail::normalized_composed(
            first,
            last,
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });
    }

    /** Returns true iff the given range of code points is normalized NFKC. */
    template<typename CPRange>
    bool normalized_nfkc(CPRange const & r) noexcept
    {
        return normalized_nfkc(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is in an FCD form.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto fcd_form(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        // http://www.unicode.org/notes/tn5/#FCD_Test
        int prev_ccc = 0;
        while (first != last) {
            auto const cp = *first;
            auto const decomp = detail::canonical_decompose(cp);
            auto const ccc = detail::ccc(*decomp.begin());
            if (ccc && ccc < prev_ccc)
                return false;
            prev_ccc =
                decomp.size_ == 1 ? ccc : detail::ccc(*(decomp.end() - 1));
            ++first;
        }
        return true;
    }

    /** Returns true iff the given range of code points is in an FCD form. */
    template<typename CPRange>
    bool fcd_form(CPRange const & r) noexcept
    {
        return fcd_form(std::begin(r), std::end(r));
    }

    /** Writes sequence `[first, last)` in normalization form FCC to `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_fcc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getInstance(
                    nullptr, "nfc", UNORM2_COMPOSE_CONTIGUOUS, ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            return detail::icu_normalize<CPIter, Sentinel, OutIter>::call(
                *norm, first, last, out);
        }
#endif
        return detail::normalize_to_composed<true>(
            first,
            last,
            out,
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });
    }

    /** Writes sequence `r` in normalization form FCC to `out`

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_fcc(CPRange const & r, OutIter out)
    {
        return normalize_to_fcc(std::begin(r), std::end(r), out);
    }

}}

#endif
