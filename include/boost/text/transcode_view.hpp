// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/dangling.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {

        // UTF-8
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf8>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf8,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf16>, Iter f_, Sentinel l)
        {
            auto f = utf_16_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(tag_t<format::utf16>, Iter f_, Iter l_)
        {
            auto f = utf_16_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_8_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf32>, Iter f_, Sentinel l)
        {
            auto f = utf_32_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(tag_t<format::utf32>, Iter f_, Iter l_)
        {
            auto f = utf_32_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_8_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }

        // UTF-16
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(tag_t<format::utf8>, Iter f_, Sentinel l)
        {
            auto f = utf_8_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(tag_t<format::utf8>, Iter f_, Iter l_)
        {
            auto f = utf_8_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_16_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(tag_t<format::utf16>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf16,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf16_range_(tag_t<format::utf32>, Iter f_, Sentinel l)
        {
            auto f = utf_32_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(tag_t<format::utf32>, Iter f_, Iter l_)
        {
            auto f = utf_32_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_16_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }

        // UTF-32
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(tag_t<format::utf8>, Iter f_, Sentinel l)
        {
            auto f = utf_8_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(tag_t<format::utf8>, Iter f_, Iter l_)
        {
            auto f = utf_8_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_32_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf32_range_(tag_t<format::utf16>, Iter f_, Sentinel l)
        {
            auto f = utf_16_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(tag_t<format::utf16>, Iter f_, Iter l_)
        {
            auto f = utf_16_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_32_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(tag_t<format::utf32>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf32,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }

#if BOOST_TEXT_USE_CONCEPTS
        template<typename ResultType, typename I1, typename I2, typename S>
        static auto make_iter(I1 first, I2 it, S last)
        {
            if constexpr (requires { ResultType(first, it, last); }) {
                return ResultType{first, it, last};
            } else {
                return it;
            }
        }
#else
        template<typename ResultType, typename Iterator, typename Sentinel>
        constexpr auto
        make_iter(Iterator first, Iterator it, Sentinel last)
            -> decltype(ResultType(first, it, last))
        {
            return ResultType(first, it, last);
        }
        template<typename ResultType>
        constexpr auto
        make_iter(ResultType first, ResultType it, ResultType last)
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Sentinel>
        constexpr auto
        make_iter(ResultType first, ResultType it, Sentinel last)
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Iterator>
        constexpr auto
        make_iter(Iterator first, ResultType it, ResultType last)
            -> decltype(ResultType(it))
        {
            return it;
        }
#endif

        template<typename I, typename S, format FromFormat, format ToFormat>
        struct transcoding_iterator;
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf8, format::utf16>
        {
            using type = utf_8_to_16_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf8, format::utf32>
        {
            using type = utf_8_to_32_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf16, format::utf8>
        {
            using type = utf_16_to_8_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf16, format::utf32>
        {
            using type = utf_16_to_32_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf32, format::utf8>
        {
            using type = utf_32_to_8_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf32, format::utf16>
        {
            using type = utf_32_to_16_iterator<I, S>;
        };
        template<typename I, typename S, format FromFormat, format ToFormat>
        using transcoding_iterator_t =
            typename transcoding_iterator<I, S, FromFormat, ToFormat>::type;

        template<format Format, typename Unpacked>
        constexpr auto make_utf_view_iter(Unpacked unpacked)
        {
            if constexpr (unpacked.format_tag == Format) {
                return unpacked.first;
            } else {
                return transcoding_iterator_t<
                    decltype(unpacked.first),
                    decltype(unpacked.last),
                    unpacked.format_tag,
                    Format>{unpacked.first, unpacked.first, unpacked.last};
            }
        }
        template<format Format, typename Unpacked>
        constexpr auto make_utf_view_sent(Unpacked unpacked)
        {
            if constexpr (
                unpacked.format_tag == Format || !std::is_same_v<
                                                     decltype(unpacked.first),
                                                     decltype(unpacked.last)>) {
                return unpacked.last;
            } else {
                return transcoding_iterator_t<
                    decltype(unpacked.first),
                    decltype(unpacked.last),
                    unpacked.format_tag,
                    Format>{unpacked.first, unpacked.last, unpacked.last};
            }
        }
    }

    namespace detail {
        template<typename V>
        struct utf_view_iter
        {
            using type = decltype(std::ranges::begin(std::declval<V>()));
        };
        template<typename V>
        struct utf_view_iter<V *>
        {
            using type = V *;
        };
        template<typename V>
        using utf_view_iter_t = typename utf_view_iter<V>::type;
        template<typename V>
        struct utf_view_sent
        {
            using type = decltype(std::ranges::end(std::declval<V>()));
        };
        template<typename V>
        struct utf_view_sent<V *>
        {
            using type = null_sentinel_t;
        };
        template<typename V>
        using utf_view_sent_t = typename utf_view_sent<V>::type;
    }

    template<format Format, utf_range_like V>
        requires std::ranges::view<V> || utf_pointer<V>
    struct utf_view : stl_interfaces::view_interface<utf_view<Format, V>>
    {
        using from_iterator = detail::utf_view_iter_t<V>;
        using from_sentinel = detail::utf_view_sent_t<V>;

        using iterator = decltype(detail::make_utf_view_iter<Format>(
            boost::text::unpack_iterator_and_sentinel(
                std::declval<from_iterator>(), std::declval<from_sentinel>())));
        using sentinel = decltype(detail::make_utf_view_sent<Format>(
            boost::text::unpack_iterator_and_sentinel(
                std::declval<from_iterator>(), std::declval<from_sentinel>())));

        constexpr utf_view() {}
        constexpr utf_view(V base)
        {
            if constexpr (std::is_pointer_v<V>) {
                auto r = boost::text::unpack_iterator_and_sentinel(
                    base, null_sentinel);
                first_ = detail::make_utf_view_iter<Format>(r);
                last_ = detail::make_utf_view_sent<Format>(r);
            } else {
                auto r = boost::text::unpack_iterator_and_sentinel(
                    std::ranges::begin(base), std::ranges::end(base));
                first_ = detail::make_utf_view_iter<Format>(r);
                last_ = detail::make_utf_view_sent<Format>(r);
            }
        }

        constexpr iterator begin() const { return first_; }
        constexpr sentinel end() const { return last_; }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, utf_view v)
        {
            if constexpr (Format == format::utf8) {
                auto out = std::ostreambuf_iterator<char>(os);
                for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                    *out = *it;
                }
            } else {
                boost::text::transcode_to_utf8(
                    v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, utf_view v)
        {
            if constexpr (Format == format::utf16) {
                auto out = std::ostreambuf_iterator<wchar_t>(os);
                for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                    *out = *it;
                }
            } else {
                boost::text::transcode_to_utf16(
                    v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            }
            return os;
        }
#endif

    private:
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf8_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf8_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf8_impl : range_adaptor_closure<as_utf8_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked =
                    text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf8_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf8, decltype(subrange)>(subrange);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf8_dispatch<as_utf8_impl, Range &&>::call(
                    (Range &&) r))
            {
                return dtl::as_utf8_dispatch<as_utf8_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf8_impl as_utf8;
#else
    namespace {
        constexpr dtl::as_utf8_impl as_utf8;
    }
#endif

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf16_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf16_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf16_impl : range_adaptor_closure<as_utf16_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked = text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf16_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf16, decltype(subrange)>(subrange);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf16_dispatch<as_utf16_impl, Range &&>::
                                call((Range &&) r))
            {
                return dtl::as_utf16_dispatch<as_utf16_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf16_impl as_utf16;
#else
    namespace {
        constexpr dtl::as_utf16_impl as_utf16;
    }
#endif

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf32_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf32_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf32_impl : range_adaptor_closure<as_utf32_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked =
                    text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf32, decltype(subrange)>(subrange);
           }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf32_dispatch<as_utf32_impl, Range &&>::
                                call((Range &&) r))
            {
                return dtl::as_utf32_dispatch<as_utf32_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf32_impl as_utf32;
#else
    namespace {
        constexpr dtl::as_utf32_impl as_utf32;
    }
#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** A view adaptor that produces a UTF-8 view of the given view. */
    constexpr detail::unspecified as_utf8;

    /** A view adaptor that produces a UTF-16 view of the given view. */
    constexpr detail::unspecified as_utf16;

    /** A view adaptor that produces a UTF-32 view of the given view. */
    constexpr detail::unspecified as_utf32;

#endif

    namespace dtl {
        template<typename T>
        struct is_empty_view : std::false_type
        {};
        template<typename T>
        struct is_empty_view<std::ranges::empty_view<T>> : std::true_type
        {};
        template<typename T>
        constexpr bool is_empty_view_v = is_empty_view<T>::value;

        template<format Format>
        struct as_utf_impl : range_adaptor_closure<as_utf_impl<Format>>
        {
            template<utf_range_like R>
            constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<decltype((r))>;
                if constexpr (std::ranges::range<T>) {
                    if constexpr (
                        dtl::is_empty_view_v<T> ||
                        code_unit_iter<std::ranges::iterator_t<T>, Format>) {
                        return r;
                    } else {
                        return utf_view<Format, T>((R &&) r);
                    }
                } else {
                    if constexpr (code_unit_pointer<T, Format>) {
                        return std::ranges::subrange(r, null_sentinel);
                    } else {
                        return utf_view<Format, T>((R &&) r);
                    }
                }
            }
        };
    }

    inline constexpr dtl::as_utf_impl<format::utf8> as_utf8;

    inline constexpr dtl::as_utf_impl<format::utf16> as_utf16;

    inline constexpr dtl::as_utf_impl<format::utf32> as_utf32;
}}}

namespace std::ranges {
template<boost::text::format Format, typename V>
inline constexpr bool enable_borrowed_range<boost::text::utf_view<Format, V>> =
    true;
}

#endif

#endif
