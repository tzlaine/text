#include <boost/text/string.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

#include "string_objects.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>


void BM_string_view_compare(benchmark::State & state)
{
    auto const & current = string_views[state.range(0)];
    auto const & last = string_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_string_view_boyer_moore(benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::algorithm::boyer_moore_search(
            string_views[state.range(0)].begin(),
            string_views[state.range(0)].end(),
            pattern,
            pattern + 1));
    }
}

void BM_string_compare(benchmark::State & state)
{
    auto const & current = string_views[state.range(0)];
    auto const & last = string_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_string_boyer_moore(benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::algorithm::boyer_moore_search(
            strings[state.range(0)].begin(),
            strings[state.range(0)].end(),
            pattern,
            pattern + 1));
    }
}

void BM_unencoded_rope_compare(benchmark::State & state)
{
    auto const & current = string_views[state.range(0)];
    auto const & last = string_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_unencoded_rope_boyer_moore(benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::algorithm::boyer_moore_search(
            unencoded_ropes[state.range(0)].begin(),
            unencoded_ropes[state.range(0)].end(),
            pattern,
            pattern + 1));
    }
}

void BM_unencoded_rope_view_compare(benchmark::State & state)
{
    auto const & current = string_views[state.range(0)];
    auto const & last = string_views[13];
    unsigned int x = 0;
    while (state.KeepRunning()) {
        x += current.compare(last);
    }
    if (x)
        std::cout << "";
}

void BM_unencoded_rope_view_boyer_moore(benchmark::State & state)
{
    char pattern[] = "!"; // This character is not in the string.
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::algorithm::boyer_moore_search(
            unencoded_rope_views[state.range(0)].begin(),
            unencoded_rope_views[state.range(0)].end(),
            pattern,
            pattern + 1));
    }
}

BENCHMARK(BM_string_view_compare) BENCHMARK_ARGS();
BENCHMARK(BM_string_view_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_string_compare) BENCHMARK_ARGS();
BENCHMARK(BM_string_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_compare) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_boyer_moore) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_view_compare) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_view_boyer_moore) BENCHMARK_ARGS();

BENCHMARK_MAIN()
