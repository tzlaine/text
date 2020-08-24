// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <coroutine>


int main()
{
#if !defined(__cpp_lib_coroutine)
    static_assert(false);
#endif
}
