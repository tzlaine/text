#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/detail/unpack.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {

        // UTF-8
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf8_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf8_tag, Iter, Sentinel>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_16_to_8_iterator(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_16_to_8_iterator(f_, f_, l_);
            auto l = make_utf_16_to_8_iterator(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_32_to_8_iterator(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_32_to_8_iterator(f_, f_, l_);
            auto l = make_utf_32_to_8_iterator(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }

        // UTF-16
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_8_to_16_iterator(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_8_to_16_iterator(f_, f_, l_);
            auto l = make_utf_8_to_16_iterator(f_, l_, l_);
            return tagged_range<utf16_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(utf16_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf16_tag, Iter, Sentinel>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf16_range_(utf32_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_32_to_16_iterator(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_32_to_16_iterator(f_, f_, l_);
            auto l = make_utf_32_to_16_iterator(f_, l_, l_);
            return tagged_range<utf16_tag, decltype(f)>{f, l};
        }

        // UTF-32
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_8_to_32_iterator(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_8_to_32_iterator(f_, f_, l_);
            auto l = make_utf_8_to_32_iterator(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf32_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = make_utf_16_to_32_iterator(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto f = make_utf_16_to_32_iterator(f_, f_, l_);
            auto l = make_utf_16_to_32_iterator(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf32_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf32_tag, Iter, Sentinel>{f, l};
        }

        template<typename ResultType, typename Iterator, typename Sentinel>
        constexpr auto
        make_iter(Iterator first, Iterator it, Sentinel last) noexcept
            -> decltype(ResultType(first, it, last))
        {
            return ResultType(first, it, last);
        }
        template<typename ResultType>
        constexpr auto
        make_iter(ResultType first, ResultType it, ResultType last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Sentinel>
        constexpr auto
        make_iter(ResultType first, ResultType it, Sentinel last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Iterator>
        constexpr auto
        make_iter(Iterator first, ResultType it, ResultType last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
    }



    /** A view over UTF-8 code units.  The set of operations available depends
        on types `Iter` and `Sentinel`.  See [view.interface] in the C++20 or
        later standard for details. */
    template<typename Iter, typename Sentinel = Iter>
    struct utf8_view : stl_interfaces::view_interface<utf8_view<Iter, Sentinel>>
    {
        using iterator = Iter;
        using sentinel = Sentinel;

        constexpr utf8_view() noexcept {}
        constexpr utf8_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf8_view lhs, utf8_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf8_view lhs, utf8_view rhs)
        {
            return !(lhs == rhs);
        }

        friend std::ostream & operator<<(std::ostream & os, utf8_view v)
        {
            auto out = std::ostreambuf_iterator<char>(os);
            for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                *out = *it;
            }
            return os;
        }
#if defined(_MSC_VER)
        friend std::wostream & operator<<(std::wostream & os, utf8_view v)
        {
            transcode_utf_8_to_16(
                v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            return os;
        }
#endif

    private:
        using iterator_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .f_);
        using sentinel_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .l_);

        iterator_t first_;
        sentinel_t last_;
    };

    /** Returns a `utf8_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf8(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace detail {
        template<
            typename Range,
            bool Pointer = char_ptr<Range>::value || _16_ptr<Range>::value ||
                           cp_ptr<Range>::value>
        struct as_utf8_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf8(std::begin(r), std::end(r)))
            {
                return as_utf8(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf8_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf8(p, null_sentinel{}))
            {
                return as_utf8(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf8_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf8(Range const & r) noexcept
        -> decltype(detail::as_utf8_dispatch<Range>::call(r))
    {
        return detail::as_utf8_dispatch<Range>::call(r);
    }



    /** A view over UTF-16 code units.  The set of operations available
        depends on types `Iter` and `Sentinel`.  See [view.interface] in the
        C++20 or later standard for details. */
    template<typename Iter, typename Sentinel = Iter>
    struct utf16_view
        : stl_interfaces::view_interface<utf16_view<Iter, Sentinel>>
    {
        using iterator = Iter;
        using sentinel = Sentinel;

        constexpr utf16_view() noexcept {}
        constexpr utf16_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf16_view lhs, utf16_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf16_view lhs, utf16_view rhs)
        {
            return !(lhs == rhs);
        }

        friend std::ostream & operator<<(std::ostream & os, utf16_view v)
        {
            transcode_utf_16_to_8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(_MSC_VER)
        friend std::wostream & operator<<(std::wostream & os, utf16_view v)
        {
            auto out = std::ostreambuf_iterator<wchar_t>(os);
            for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                *out = *it;
            }
            return os;
        }
#endif

    private:
        using iterator_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .f_);
        using sentinel_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .l_);

        iterator_t first_;
        sentinel_t last_;
    };

    /** Returns a `utf16_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf16(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace detail {
        template<
            typename Range,
            bool Pointer = char_ptr<Range>::value || _16_ptr<Range>::value ||
                           cp_ptr<Range>::value>
        struct as_utf16_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf16(std::begin(r), std::end(r)))
            {
                return as_utf16(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf16_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf16(p, null_sentinel{}))
            {
                return as_utf16(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf16_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf16(Range const & r) noexcept
        -> decltype(detail::as_utf16_dispatch<Range>::call(r))
    {
        return detail::as_utf16_dispatch<Range>::call(r);
    }



    /** A view over UTF-32 code units.  The set of operations available
        depends on types `Iter` and `Sentinel`.  See [view.interface] in the
        C++20 or later standard for details. */
    template<typename Iter, typename Sentinel = Iter>
    struct utf32_view
        : stl_interfaces::view_interface<utf32_view<Iter, Sentinel>>
    {
        using iterator = Iter;
        using sentinel = Sentinel;

        constexpr utf32_view() noexcept {}
        constexpr utf32_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf32_view lhs, utf32_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf32_view lhs, utf32_view rhs)
        {
            return !(lhs == rhs);
        }

        friend std::ostream & operator<<(std::ostream & os, utf32_view v)
        {
            transcode_utf_32_to_8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(_MSC_VER)
        friend std::wostream & operator<<(std::wostream & os, utf32_view v)
        {
            transcode_utf_32_to_16(
                v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            return os;
        }
#endif

    private:
        using iterator_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .f_);
        using sentinel_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<Iter>(), std::declval<Sentinel>())
                         .l_);

        iterator_t first_;
        sentinel_t last_;
    };

    /** Returns a `utf32_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf32(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace detail {
        template<
            typename Range,
            bool Pointer = char_ptr<Range>::value || _16_ptr<Range>::value ||
                           cp_ptr<Range>::value>
        struct as_utf32_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf32(std::begin(r), std::end(r)))
            {
                return as_utf32(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf32_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf32(p, null_sentinel{}))
            {
                return as_utf32(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf32_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf32(Range const & r) noexcept
        -> decltype(detail::as_utf32_dispatch<Range>::call(r))
    {
        return detail::as_utf32_dispatch<Range>::call(r);
    }

}}}

#endif
