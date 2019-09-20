#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_iterator.hpp>
#include <boost/text/detail/algorithm.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {
        struct utf8_tag
        {};
        struct utf16_tag
        {};
        struct utf32_tag
        {};

        template<typename Tag, typename Iter, typename Sentinel = Iter>
        struct tagged_range
        {
            Iter f_;
            Sentinel l_;
            Tag tag_;
        };

        template<
            typename Iter,
            typename Sentinel,
            bool UTF8 = is_char_iter<Iter>::value,
            bool UTF16 = is_16_iter<Iter>::value,
            bool UTF32 = is_cp_iter<Iter>::value>
        struct unpack_iterator_and_sentinel_impl
        {
        };

        template<typename Iter, typename Sentinel>
        struct unpack_iterator_and_sentinel_impl<
            Iter,
            Sentinel,
            true,
            false,
            false>
        {
            static constexpr auto call(Iter first, Sentinel last) noexcept
            {
                return tagged_range<utf8_tag, Iter, Sentinel>{first, last};
            }
        };
        template<typename Iter, typename Sentinel>
        struct unpack_iterator_and_sentinel_impl<
            Iter,
            Sentinel,
            false,
            true,
            false>
        {
            static constexpr auto call(Iter first, Sentinel last) noexcept
            {
                return tagged_range<utf16_tag, Iter, Sentinel>{first, last};
            }
        };
        template<typename Iter, typename Sentinel>
        struct unpack_iterator_and_sentinel_impl<
            Iter,
            Sentinel,
            false,
            false,
            true>
        {
            static constexpr auto call(Iter first, Sentinel last) noexcept
            {
                return tagged_range<utf32_tag, Iter, Sentinel>{first, last};
            }
        };

        template<typename Iter, typename Sentinel>
        constexpr auto
        unpack_iterator_and_sentinel(Iter first, Sentinel last) noexcept
            -> decltype(unpack_iterator_and_sentinel_impl<Iter, Sentinel>::call(
                first, last))
        {
            return unpack_iterator_and_sentinel_impl<Iter, Sentinel>::call(
                first, last);
        }

        // 8 -> 32
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_32_iterator<Iter> first,
            utf_8_to_32_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_32_iterator<Iter, Sentinel> first, Sentinel last) noexcept;
        // 32 -> 8
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_8_iterator<Iter> first,
            utf_32_to_8_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_8_iterator<Iter, Sentinel> first, Sentinel last) noexcept;
        // 16 -> 32
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_32_iterator<Iter> first,
            utf_16_to_32_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_32_iterator<Iter, Sentinel> first,
            Sentinel last) noexcept;
        // 32 -> 16
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_16_iterator<Iter> first,
            utf_32_to_16_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_16_iterator<Iter, Sentinel> first,
            Sentinel last) noexcept;
        // 8 -> 16
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_16_iterator<Iter> first,
            utf_8_to_16_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_16_iterator<Iter, Sentinel> first, Sentinel last) noexcept;
        // 16 -> 8
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_8_iterator<Iter> first,
            utf_16_to_8_iterator<Iter> last) noexcept;
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_8_iterator<Iter, Sentinel> first, Sentinel last) noexcept;

        // 8 -> 32
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_32_iterator<Iter> first,
            utf_8_to_32_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_32_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }
        // 32 -> 8
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_8_iterator<Iter> first,
            utf_32_to_8_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_8_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }

        // 16 -> 32
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_32_iterator<Iter> first,
            utf_16_to_32_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_32_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }
        // 32 -> 16
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_16_iterator<Iter> first,
            utf_32_to_16_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_32_to_16_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }

        // 8 -> 16
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_16_iterator<Iter> first,
            utf_8_to_16_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_8_to_16_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }
        // 16 -> 8
        template<typename Iter>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_8_iterator<Iter> first,
            utf_16_to_8_iterator<Iter> last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last.base());
        }
        template<typename Iter, typename Sentinel>
        constexpr auto unpack_iterator_and_sentinel(
            utf_16_to_8_iterator<Iter, Sentinel> first, Sentinel last) noexcept
        {
            return unpack_iterator_and_sentinel(first.base(), last);
        }



        // UTF-8
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf8_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf8_tag, Iter, Sentinel>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto const f = make_utf_16_to_8_iterator(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_16_to_8_iterator(f_, f_, l_);
            auto const l = make_utf_16_to_8_iterator(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Sentinel l) noexcept
        {
            auto const f = make_utf_32_to_8_iterator(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_32_to_8_iterator(f_, f_, l_);
            auto const l = make_utf_32_to_8_iterator(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }

        // UTF-16
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto const f = make_utf_8_to_16_iterator(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_8_to_16_iterator(f_, f_, l_);
            auto const l = make_utf_8_to_16_iterator(f_, l_, l_);
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
            auto const f = make_utf_32_to_16_iterator(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_32_to_16_iterator(f_, f_, l_);
            auto const l = make_utf_32_to_16_iterator(f_, l_, l_);
            return tagged_range<utf16_tag, decltype(f)>{f, l};
        }

        // UTF-32
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto const f = make_utf_8_to_32_iterator(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_8_to_32_iterator(f_, f_, l_);
            auto const l = make_utf_8_to_32_iterator(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf32_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto const f = make_utf_16_to_32_iterator(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto const f = make_utf_16_to_32_iterator(f_, f_, l_);
            auto const l = make_utf_16_to_32_iterator(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf32_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf32_tag, Iter, Sentinel>{f, l};
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
            first_(first),
            last_(last)
        {}

        constexpr iterator begin() const noexcept { return first_; }
        constexpr sentinel end() const noexcept { return last_; }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Returns a `utf8_range` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf8(Iter first, Sentinel last) noexcept
    {
        auto const unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto const r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf8_range` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf8(Range const & r) noexcept
        -> decltype(as_utf8(std::begin(r), std::end(r)))
    {
        return as_utf8(std::begin(r), std::end(r));
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
            first_(first),
            last_(last)
        {}

        constexpr iterator begin() const noexcept { return first_; }
        constexpr sentinel end() const noexcept { return last_; }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Returns a `utf16_range` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf16(Iter first, Sentinel last) noexcept
    {
        auto const unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto const r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf16_range` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf16(Range const & r) noexcept
        -> decltype(as_utf16(std::begin(r), std::end(r)))
    {
        return as_utf16(std::begin(r), std::end(r));
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
            first_(first),
            last_(last)
        {}

        constexpr iterator begin() const noexcept { return first_; }
        constexpr sentinel end() const noexcept { return last_; }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Returns a `utf32_range` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf32(Iter first, Sentinel last) noexcept
    {
        auto const unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto const r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf32_range` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf32(Range const & r) noexcept
        -> decltype(as_utf32(std::begin(r), std::end(r)))
    {
        return as_utf32(std::begin(r), std::end(r));
    }



    /** A template alias for utf32_view. */
    template<typename Iter, typename Sentinel>
    using code_point_view = utf32_view<Iter, Sentinel>;

}}

#endif
