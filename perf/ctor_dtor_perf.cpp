#include <boost/text/string.hpp>

#include "strings.hpp"

#include <benchmark/benchmark.h>

#include <iostream>


void BM_string_view_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::string_view(
            std_strings[state.range(0)].c_str(),
            std_strings[state.range(0)].size()));
    }
}

void BM_string_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(
            boost::text::string(std_strings[state.range(0)]));
    }
}

void BM_unencoded_rope_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::unencoded_rope(
            boost::text::string_view(std_strings[state.range(0)])));
    }
}

void BM_unencoded_rope_view_ctor_dtor(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(boost::text::unencoded_rope_view(
            boost::text::string_view(std_strings[state.range(0)])));
    }
}

BENCHMARK(BM_string_view_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_string_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_unencoded_rope_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK(BM_unencoded_rope_view_ctor_dtor) BENCHMARK_ARGS();

BENCHMARK_MAIN()
