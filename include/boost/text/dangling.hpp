// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DANGLING_HPP
#define BOOST_TEXT_DANGLING_HPP

#include <boost/text/config.hpp>


#if BOOST_TEXT_USE_CONCEPTS

namespace boost::text {
    /** An alias template analogous to `std::ranges::borrowed_subrange_t`. */
    template<std::ranges::range R, std::ranges::view V>
    using borrowed_view_t = std::
        conditional_t<std::ranges::borrowed_range<R>, V, std::ranges::dangling>;
}

#endif

#endif
