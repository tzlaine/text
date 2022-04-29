// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_BREAK_VIEW_HPP
#define BOOST_TEXT_BREAK_VIEW_HPP

#include <boost/text/transcode_view.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>


namespace boost { namespace text {

    namespace detail {
        template<
            typename CPIter,
            typename CPSentinel,
            typename PrevFunc,
            typename NextFunc,
            typename CPRange>
        struct break_iterator : stl_interfaces::proxy_iterator_interface<
                                    break_iterator<
                                        CPIter,
                                        CPSentinel,
                                        PrevFunc,
                                        NextFunc,
                                        CPRange>,
                                    std::bidirectional_iterator_tag,
                                    CPRange>
        {
            break_iterator() noexcept = default;

            // begin
            break_iterator(
                CPIter first,
                CPSentinel last,
                PrevFunc * prev_func,
                NextFunc * next_func) noexcept :
                first_(first),
                seg_(first, first),
                last_(last),
                prev_func_(prev_func),
                next_func_(next_func)
            {
                seg_.second = (*next_func_)(seg_.first, last_);
            }

            // end
            break_iterator(
                CPIter first,
                CPIter it,
                CPSentinel last,
                PrevFunc * prev_func,
                NextFunc * next_func) noexcept :
                first_(first),
                seg_(it, it),
                last_(last),
                prev_func_(prev_func),
                next_func_(next_func)
            {}

            CPRange operator*() const noexcept
            {
                return CPRange{seg_.first, seg_.second};
            }

            break_iterator & operator++() noexcept
            {
                auto const next_it = (*next_func_)(seg_.second, last_);
                seg_.first = seg_.second;
                seg_.second = next_it;
                return *this;
            }

            break_iterator & operator--() noexcept
            {
                if (seg_.first == first_) {
                    seg_.second = first_;
                    return *this;
                }

                auto const prev_it =
                    (*prev_func_)(first_, std::prev(seg_.first), seg_.second);
                seg_.second = seg_.first;
                seg_.first = prev_it;
                return *this;
            }

            friend bool
            operator==(break_iterator lhs, break_iterator rhs) noexcept
            {
                return lhs.seg_ == rhs.seg_;
            }

            template<typename Sentinel>
            friend std::enable_if_t<
                !std::is_same<CPIter, CPSentinel>::value &&
                    std::is_same<Sentinel, CPSentinel>::value,
                bool>
            operator==(break_iterator lhs, Sentinel rhs) noexcept
            {
                return lhs.seg_.first == rhs;
            }

            using base_type = stl_interfaces::proxy_iterator_interface<
                break_iterator<CPIter, CPSentinel, PrevFunc, NextFunc, CPRange>,
                std::bidirectional_iterator_tag,
                CPRange>;
            using base_type::operator++;
            using base_type::operator--;

        private:
            CPIter first_ = {};
            std::pair<CPIter, CPIter> seg_ = {};
            CPSentinel last_ = {};
            PrevFunc * prev_func_ = nullptr;
            NextFunc * next_func_ = nullptr;
        };

        template<
            typename T,
            typename CPIter,
            typename CPSentinel,
            typename PrevFunc,
            typename NextFunc>
        struct make_break_iter_last
        {
            using type = CPSentinel;
            static auto call(
                CPIter first,
                CPSentinel last,
                PrevFunc * prev_func,
                NextFunc * next_func)
            {
                return last;
            }
        };

        template<
            typename T,
            typename CPIter,
            typename PrevFunc,
            typename NextFunc>
        struct make_break_iter_last<T, CPIter, CPIter, PrevFunc, NextFunc>
        {
            using type = T;
            static auto call(
                CPIter first,
                CPIter last,
                PrevFunc * prev_func,
                NextFunc * next_func)
            {
                return T(first, last, last, prev_func, next_func);
            }
        };
    }

    /** Represents a bidirectionally-iterable range of non-overlapping code
        point subranges.  Each code point subrange represents some
        semantically significant segment, the semantics of which are
        controlled by the `PrevFunc` and `NextFunc` template parameters.  For
        instance, if `NextFunc` is next_paragraph_break, the subranges
        produced by break_view will be paragraphs.  Each subrange is lazily
        produced; an output subrange is not produced until a lazy range
        iterator is dereferenced. */
    template<
        typename CPIter,
        typename CPSentinel,
        typename PrevFunc,
        typename NextFunc,
        typename Subrange = utf32_view<CPIter>>
    struct break_view
        : stl_interfaces::view_interface<
              break_view<CPIter, CPSentinel, PrevFunc, NextFunc, Subrange>>
    {
        using iterator = detail::
            break_iterator<CPIter, CPSentinel, PrevFunc, NextFunc, Subrange>;
        using sentinel = typename detail::make_break_iter_last<
            iterator,
            CPIter,
            CPSentinel,
            PrevFunc,
            NextFunc>::type;

        break_view() noexcept {}
        break_view(
            CPIter first,
            CPSentinel last,
            PrevFunc prev_func,
            NextFunc next_func) noexcept :
            first_(first, last, &prev_func_, &next_func_),
            prev_func_(std::move(prev_func)),
            next_func_(std::move(next_func))
        {
            last_ = detail::make_break_iter_last<
                iterator,
                CPIter,
                CPSentinel,
                PrevFunc,
                NextFunc>::call(first, last, &prev_func_, &next_func_);
        }

        iterator begin() const noexcept { return first_; }
        sentinel end() const noexcept { return last_; }

        /** Moves the contained `PrevFunc` out of *this. */
        PrevFunc && prev_func() && noexcept { return std::move(prev_func_); }

        /** Moves the contained `NextFunc` out of *this. */
        NextFunc && next_func() && noexcept { return std::move(next_func_); }

    private:
        iterator first_;
        sentinel last_;
        PrevFunc prev_func_;
        NextFunc next_func_;
    };
}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<
        typename CPIter,
        typename CPSentinel,
        typename PrevFunc,
        typename NextFunc,
        typename Subrange>
    inline constexpr bool enable_borrowed_range<
        boost::text::
            break_view<CPIter, CPSentinel, PrevFunc, NextFunc, Subrange>> =
        true;
}

#endif

#endif
