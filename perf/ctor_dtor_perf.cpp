#include <boost/text/text.hpp>

#include "strings.hpp"

#include <benchmark/benchmark.h>

#include <iostream>


void BM_text_view_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::text_view(
            strings[state.range(0)].c_str(), strings[state.range(0)].size()));
    }
}

void BM_text_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::text(strings[state.range(0)]));
    }
}

void BM_rope_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::unencoded_rope(
            boost::text::text_view(strings[state.range(0)])));
    }
}

void BM_rope_view_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::unencoded_rope_view(
            boost::text::text_view(strings[state.range(0)])));
    }
}

BENCHMARK(BM_text_view_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_text_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_rope_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_rope_view_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK_MAIN()
