#include <boost/text/text.hpp>

#include "text_objects.hpp"

#include <benchmark/benchmark.h>

#include <iostream>


void BM_text_erase_insert_front(benchmark::State & state)
{
    auto & text = texts[state.range(0)];
    auto const i = 0;
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(text.erase(text(i, i + 1)).insert(i, "."));
    }
}

void BM_text_erase_insert_back(benchmark::State & state)
{
    auto & text = texts[state.range(0)];
    auto const i = text.size() - 1;
    assert(0 <= i);
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(text.erase(text(i, i + 1)).insert(i, "."));
        assert(i == text.size() - 1);
    }
}

void BM_rope_erase_insert_front(benchmark::State & state)
{
    auto & rope = ropes[state.range(0)];
    auto const i = 0;
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(rope.erase(rope(i, i + 1)).insert(i, "."));
    }
}

void BM_rope_erase_insert_back(benchmark::State & state)
{
    auto & rope = ropes[state.range(0)];
    auto const i = rope.size() - 1;
    assert(0 <= i);
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(rope.erase(rope(i, i + 1)).insert(i, "."));
        assert(i == rope.size() - 1);
    }
}

BENCHMARK(BM_text_erase_insert_front) BENCHMARK_ARGS_NONEMPTY();
BENCHMARK(BM_text_erase_insert_back) BENCHMARK_ARGS_NONEMPTY();
BENCHMARK(BM_rope_erase_insert_front) BENCHMARK_ARGS_NONEMPTY();
BENCHMARK(BM_rope_erase_insert_back) BENCHMARK_ARGS_NONEMPTY();

BENCHMARK_MAIN()
