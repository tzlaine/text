#include <boost/text/text.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

#include "text_objects.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>


void BM_text_view_compare (benchmark::State & state)
{
    auto const & current = text_views[state.range(0)];
    auto const & last = text_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_text_view_boyer_moore (benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::algorithm::boyer_moore_search(
                text_views[state.range(0)].begin(), text_views[state.range(0)].end(),
                pattern, pattern + 1
            )
        );
    }
}

void BM_text_compare (benchmark::State & state)
{
    auto const & current = text_views[state.range(0)];
    auto const & last = text_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_text_boyer_moore (benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::algorithm::boyer_moore_search(
                texts[state.range(0)].begin(), texts[state.range(0)].end(),
                pattern, pattern + 1
            )
        );
    }
}

void BM_rope_compare (benchmark::State & state)
{
    auto const & current = text_views[state.range(0)];
    auto const & last = text_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_rope_boyer_moore (benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::algorithm::boyer_moore_search(
                ropes[state.range(0)].begin(), ropes[state.range(0)].end(),
                pattern, pattern + 1
            )
        );
    }
}

void BM_rope_view_compare (benchmark::State & state)
{
    auto const & current = text_views[state.range(0)];
    auto const & last = text_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_rope_view_boyer_moore (benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::algorithm::boyer_moore_search(
                rope_views[state.range(0)].begin(), rope_views[state.range(0)].end(),
                pattern, pattern + 1
            )
        );
    }
}

BENCHMARK(BM_text_view_compare) BENCHMARK_ARGS();
BENCHMARK(BM_text_view_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_text_compare) BENCHMARK_ARGS();
BENCHMARK(BM_text_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_rope_compare) BENCHMARK_ARGS();
BENCHMARK(BM_rope_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_rope_view_compare) BENCHMARK_ARGS();
BENCHMARK(BM_rope_view_boyer_moore) BENCHMARK_ARGS();

BENCHMARK_MAIN()
