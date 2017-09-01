#include <boost/text/text.hpp>

#include "text_objects.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>


void BM_text_view_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : text_views[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_text_view_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            text_views[state.range(0)].begin(),
            text_views[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_text_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : texts[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_text_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            texts[state.range(0)].begin(),
            texts[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_rope_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : ropes[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_rope_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            ropes[state.range(0)].begin(),
            ropes[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_rope_view_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : rope_views[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_rope_view_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            rope_views[state.range(0)].begin(),
            rope_views[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

BENCHMARK(BM_text_view_for) BENCHMARK_ARGS();
BENCHMARK(BM_text_view_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_text_for) BENCHMARK_ARGS();
BENCHMARK(BM_text_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_rope_for) BENCHMARK_ARGS();
BENCHMARK(BM_rope_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_rope_view_for) BENCHMARK_ARGS();
BENCHMARK(BM_rope_view_std_find) BENCHMARK_ARGS();

BENCHMARK_MAIN()
