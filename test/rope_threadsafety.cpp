#include <boost/text/rope.hpp>

#include <atomic>
#include <thread>


std::atomic<bool> done(false);

void thread_function(boost::text::rope const & r)
{
    boost::text::rope local_r = r;

    while (!done)
        ;
}

int main()
{
    std::unique_ptr<boost::text::rope> r(new boost::text::rope("some text"));

    std::thread thread_0(thread_function, *r);
    std::thread thread_1(thread_function, *r);
    std::thread thread_2(thread_function, *r);
    std::thread thread_3(thread_function, *r);

    std::thread thread_4(thread_function, *r);
    std::thread thread_5(thread_function, *r);
    std::thread thread_6(thread_function, *r);
    std::thread thread_7(thread_function, *r);

    r.release();
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
