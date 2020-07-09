// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_ILL_FORMED_HPP
#define BOOST_TEXT_ILL_FORMED_HPP

#include <boost/text/detail/algorithm.hpp>


template<template<class...> class Template, typename... Args>
using ill_formed = std::integral_constant<
    bool,
    !boost::text::detail::is_detected<Template, Args...>::value>;

#endif
