#include <boost/text/text.hpp>

#include <benchmark/benchmark.h>

#include <iostream>
#include <string>


namespace {

    std::string make_string (int size)
    { return std::string(size, '.'); }

    std::string strings[14] = {
        make_string(0),
        make_string(1 << 0),
        make_string(1 << 1),
        make_string(1 << 2),
        make_string(1 << 3),
        make_string(1 << 4),
        make_string(1 << 5),
        make_string(1 << 6),
        make_string(1 << 8),
        make_string(1 << 10),
        make_string(1 << 12),
        make_string(1 << 16),
        make_string(1 << 20),
        make_string(1 << 30)
    };

}

void BM_text_view_ctor_dtor (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::text_view(
                strings[state.range(0)].c_str(),
                strings[state.range(0)].size()
            )
        );
    }
}

void BM_text_view_ctor_dtor_unchecked (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::text_view(
                strings[state.range(0)].c_str(),
                strings[state.range(0)].size(),
                boost::text::utf8::unchecked
            )
        );
    }
}

#define ALL_BENCHMARK_ARGS()                    \
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)    \
    ->Arg(5)->Arg(6)->Arg(7)->Arg(8)->Arg(9)    \
    ->Arg(10)->Arg(11)->Arg(12)->Arg(13)

#ifdef FULL_PERF_TESTS
#define BENCHMARK_ARGS() ALL_BENCHMARK_ARGS()
#else
#define BENCHMARK_ARGS()                        \
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)    \
    ->Arg(5)->Arg(6)->Arg(7)->Arg(8)->Arg(9)
#endif

BENCHMARK(BM_text_view_ctor_dtor) ALL_BENCHMARK_ARGS();
BENCHMARK(BM_text_view_ctor_dtor_unchecked) ALL_BENCHMARK_ARGS();

BENCHMARK_MAIN()
