// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string.hpp>

#include <benchmark/benchmark.h>

#include <iostream>
#include <random>


std::vector<int> make_noise_vec()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist;

    auto rand = [&]() { return dist(gen); };

    std::vector<int> v(10);
    std::generate(v.begin(), v.end(), rand);

    return v;
}
std::vector<int> const noise_vec = make_noise_vec();


void BM_string_insert_one_char(benchmark::State & state)
{
    boost::text::unencoded_rope unencoded_rope;
    unencoded_rope += boost::text::string();
    while (state.KeepRunning()) {
        for (int i = 0, end = state.range(0); i < end; ++i) {
            unencoded_rope.insert(unencoded_rope.size() / 2, ".");
        }
    }
    if (unencoded_rope.size() < noise_vec.back())
        std::cout << "";
}

BENCHMARK(BM_string_insert_one_char)->RangeMultiplier(2)->Range(512, 1 << 20);

int main(int argc, char ** argv)
{
    std::cout << "string_insert_max=" << BOOST_TEXT_STRING_INSERT_MAX << "\n";
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
}
