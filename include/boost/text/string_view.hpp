// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_STRING_VIEW_HPP
#define BOOST_TEXT_STRING_VIEW_HPP

#include <boost/text/config.hpp>

#if !defined(__cpp_lib_string_view)
#include <boost/utility/string_view.hpp>
#else
#include <string_view>
#endif

#include <string>


namespace boost { namespace text {

#if defined(__cpp_lib_string_view)
    using string_view = std::string_view;
#else
    using string_view = boost::string_view;
#endif

    namespace detail {
        inline string_view substring(string_view sv, int lo, int hi)
        {
            if (lo < 0)
                lo = sv.size() + lo;
            if (hi < 0)
                hi = sv.size() + hi;
            return string_view(sv.data() + lo, hi - lo);
        }
        inline string_view substring(std::string const & s, int lo, int hi)
        {
            if (lo < 0)
                lo = s.size() + lo;
            if (hi < 0)
                hi = s.size() + hi;
            return string_view(s.data() + lo, hi - lo);
        }
    }

}}

#endif
