// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/unencoded_rope.hpp>

#include <atomic>
#include <thread>


std::atomic<bool> done(false);

void thread_function(boost::text::unencoded_rope r)
{
    boost::text::unencoded_rope local_r = r;

    while (!done)
        ;
}

int main()
{
    boost::text::unencoded_rope r("some text");

    std::thread thread_0(thread_function, r);
    std::thread thread_1(thread_function, r);
    std::thread thread_2(thread_function, r);
    std::thread thread_3(thread_function, r);

    std::thread thread_4(thread_function, r);
    std::thread thread_5(thread_function, r);
    std::thread thread_6(thread_function, r);
    std::thread thread_7(thread_function, r);

    done = true;

    thread_0.join();
    thread_1.join();
    thread_2.join();
    thread_3.join();

    thread_4.join();
    thread_5.join();
    thread_6.join();
    thread_7.join();
}
