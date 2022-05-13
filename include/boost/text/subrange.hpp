// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_SUBRANGE_HPP
#define BOOST_TEXT_SUBRANGE_HPP

#include <boost/text/config.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

#if BOOST_TEXT_USE_CONCEPTS
    template<std::forward_iterator I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct subrange : stl_interfaces::view_interface<subrange<I, S>>
    {
        constexpr subrange() = default;
        constexpr subrange(I first, S last) : first_(first), last_(last) {}
        template<typename R>
        constexpr explicit subrange(R const & r) :
            first_(detail::begin(r)), last_(detail::end(r))
        {}

        constexpr I begin() const { return first_; }
        constexpr S end() const { return last_; }

        [[nodiscard]] constexpr subrange next(std::ptrdiff_t n = 1) const
        {
            return subrange{std::next(first_), last_};
        }
        [[nodiscard]] constexpr subrange prev(std::ptrdiff_t n = 1) const
        {
            return subrange{std::prev(first_), last_};
        }

        constexpr subrange & advance(std::ptrdiff_t n)
        {
            std::advance(first_, n);
            return *this;
        }

        template<
            typename I2,
            typename S2,
            typename Enable = std::enable_if_t<
                std::is_convertible<I, I2>::value &&
                std::is_convertible<S, S2>::value>>
        constexpr operator subrange<I2, S2>() const
        {
            return {first_, last_};
        }

    private:
        I first_;
        [[no_unique_address]] S last_;
    };

}}

#endif
