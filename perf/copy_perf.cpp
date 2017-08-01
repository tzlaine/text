#include <boost/text/text.hpp>

#include "text_objects.hpp"

#include <benchmark/benchmark.h>

#include <iostream>


void BM_text_view_copy (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::text_view(text_views[state.range(0)])
        );
    }
}

void BM_text_copy (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::text(texts[state.range(0)])
        );
    }
}

void BM_rope_copy (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::rope(ropes[state.range(0)])
        );
    }
}

void BM_rope_view_copy (benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::rope_view(rope_views[state.range(0)])
        );
    }
}

BENCHMARK(BM_text_view_copy) BENCHMARK_ARGS();
BENCHMARK(BM_text_copy) BENCHMARK_ARGS();
BENCHMARK(BM_rope_copy) BENCHMARK_ARGS();
BENCHMARK(BM_rope_view_copy) BENCHMARK_ARGS();

BENCHMARK_MAIN()
