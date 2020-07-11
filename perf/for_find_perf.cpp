// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include "string_objects.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>


void BM_string_view_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : string_views[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_string_view_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            string_views[state.range(0)].begin(),
            string_views[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_string_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : strings[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_string_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            strings[state.range(0)].begin(),
            strings[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_unencoded_rope_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : unencoded_ropes[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_unencoded_rope_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            unencoded_ropes[state.range(0)].begin(),
            unencoded_ropes[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

void BM_unencoded_rope_view_for(benchmark::State & state)
{
    unsigned int x = 0;
    while (state.KeepRunning()) {
        for (char const c : unencoded_rope_views[state.range(0)]) {
            x += c;
        }
    }
    if (x)
        std::cout << "";
}

void BM_unencoded_rope_view_std_find(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::find(
            unencoded_rope_views[state.range(0)].begin(),
            unencoded_rope_views[state.range(0)].end(),
            '!' // This character is not in the string.
            ));
    }
}

BENCHMARK(BM_string_view_for) BENCHMARK_ARGS();
BENCHMARK(BM_string_view_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_string_for) BENCHMARK_ARGS();
BENCHMARK(BM_string_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_for) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_std_find) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_view_for) BENCHMARK_ARGS();
BENCHMARK(BM_unencoded_rope_view_std_find) BENCHMARK_ARGS();

BENCHMARK_MAIN()
