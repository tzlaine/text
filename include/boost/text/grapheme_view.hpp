#ifndef BOOST_TEXT_GRAPHEME_VIEW_HPP
#define BOOST_TEXT_GRAPHEME_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/transcode_view.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {
        template<typename CPIter, typename Sentinel>
        auto make_gr_view_last(CPIter first, Sentinel last)
        {
            return last;
        }
        template<typename CPIter>
        auto make_gr_view_last(CPIter first, CPIter last)
        {
            return grapheme_iterator<CPIter>(first, last, last);
        }
        template<typename CPIter, typename Sentinel>
        using gr_view_sentinel_t = std::conditional_t<
            std::is_same<CPIter, Sentinel>::value,
            grapheme_iterator<CPIter, Sentinel>,
            Sentinel>;
    }

    /** A view over graphemes that occur in an underlying sequence of code
        points. */
    template<typename CPIter, typename Sentinel = CPIter>
    struct grapheme_view
        : stl_interfaces::view_interface<grapheme_view<CPIter, Sentinel>>
    {
        using iterator = grapheme_iterator<CPIter, Sentinel>;
        using sentinel = detail::gr_view_sentinel_t<CPIter, Sentinel>;

        constexpr grapheme_view() : first_(), last_() {}

        /** Construct a grapheme view that covers the entirety of the view
            of graphemes that `begin()` and `end()` lie within. */
        constexpr grapheme_view(iterator first, sentinel last) :
            first_(first),
            last_(last)
        {}

        /** Construct a grapheme view that covers the entirety of the view
            of graphemes that `begin()` and `end()` lie within. */
        constexpr grapheme_view(CPIter first, Sentinel last) :
            first_(first, first, last),
            last_(detail::make_gr_view_last(first, last))
        {}

        /** Construct a view covering a subset of the view of graphemes that
            `begin()` and `end()` lie within. */
        template<
            typename I = CPIter,
            typename S = Sentinel,
            typename Enable = std::enable_if_t<std::is_same<I, S>::value>>
        constexpr grapheme_view(I first, I view_first, I view_last, I last) :
            first_(first, view_first, last),
            last_(first, view_last, last)
        {}

        constexpr iterator begin() const noexcept { return first_; }
        constexpr sentinel end() const noexcept { return last_; }

        friend constexpr bool operator==(grapheme_view lhs, grapheme_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(grapheme_view lhs, grapheme_view rhs)
        {
            return !(lhs == rhs);
        }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Returns a `grapheme_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_graphemes(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return grapheme_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace detail {
        template<
            typename Range,
            bool Pointer = char_ptr<Range>::value || _16_ptr<Range>::value ||
                           cp_ptr<Range>::value>
        struct as_graphemes_dispatch
        {
            static constexpr auto call(Range const & r_) noexcept
            {
                auto r = as_utf32(r_);
                return grapheme_view<decltype(r.begin()), decltype(r.end())>(
                    r.begin(), r.end());
            }
        };

        template<typename Ptr>
        struct as_graphemes_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
            {
                auto r = as_utf32(p);
                return grapheme_view<decltype(r.begin()), null_sentinel>(
                    r.begin(), null_sentinel{});
            }
        };
    }

    /** Returns a `grapheme_view` over the data in `r`, transcoding the data
        if necessary. */
    template<typename Range>
    constexpr auto as_graphemes(Range const & r) noexcept
        -> decltype(detail::as_graphemes_dispatch<Range>::call(r))
    {
        return detail::as_graphemes_dispatch<Range>::call(r);
    }

}}

#endif
