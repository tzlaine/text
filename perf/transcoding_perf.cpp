#include <boost/text/string.hpp>
#include <boost/text/utf8.hpp>
#include <boost/text/utf16.hpp>
#include <boost/text/transcode_algorithm.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <fstream>
#include <iostream>


std::string file_slurp(std::string filename)
{
    std::string retval;
    std::ifstream ifs(filename);
    while (ifs) {
        char const c = ifs.get();
        retval += c;
    }
    return retval;
}

std::string const utf8_text = file_slurp("portuguese_wiki.txt") +
                              file_slurp("korean_wiki.txt") +
                              file_slurp("hindi_wiki_in_russian.txt") +
                              file_slurp("perf/portuguese_wiki.txt") +
                              file_slurp("perf/korean_wiki.txt") +
                              file_slurp("perf/hindi_wiki_in_russian.txt");

using namespace boost;

void BM_8_to_16_algorithm_prealloc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result(utf8_text.size());
        auto out = text::transcode_utf_8_to_16(utf8_text, utf16_result.begin());
        utf16_result.resize(out - utf16_result.begin());
    }
}

void BM_8_to_16_algorithm_no_simd_prealloc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result(utf8_text.size());
        auto out = text::detail::transcode_utf_8_to_16(
            utf8_text.begin(),
            utf8_text.end(),
            utf16_result.begin(),
            std::input_iterator_tag{});
        utf16_result.resize(out - utf16_result.begin());
    }
}

void BM_8_to_16_iterator_prealloc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result(utf8_text.size());
        auto out = std::copy(
            text::utf8::make_to_utf16_iterator(
                utf8_text.begin(), utf8_text.begin(), utf8_text.end()),
            text::utf8::make_to_utf16_iterator(
                utf8_text.begin(), utf8_text.end(), utf8_text.end()),
            utf16_result.begin());
        utf16_result.resize(out - utf16_result.begin());
    }
}

void BM_8_to_16_algorithm(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result;
        text::transcode_utf_8_to_16(
            utf8_text, std::back_inserter(utf16_result));
    }
}

void BM_8_to_16_algorithm_no_simd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result;
        text::detail::transcode_utf_8_to_16(
            utf8_text.begin(),
            utf8_text.end(),
            std::back_inserter(utf16_result),
            std::input_iterator_tag{});
    }
}

void BM_8_to_16_iterator(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint16_t> utf16_result;
        std::copy(
            text::utf8::make_to_utf16_iterator(
                utf8_text.begin(), utf8_text.begin(), utf8_text.end()),
            text::utf8::make_to_utf16_iterator(
                utf8_text.begin(), utf8_text.end(), utf8_text.end()),
            std::back_inserter(utf16_result));
    }
}

// TODO

BENCHMARK(BM_8_to_16_algorithm_prealloc);
BENCHMARK(BM_8_to_16_algorithm_no_simd_prealloc);
BENCHMARK(BM_8_to_16_iterator_prealloc);
BENCHMARK(BM_8_to_16_algorithm);
BENCHMARK(BM_8_to_16_algorithm_no_simd);
BENCHMARK(BM_8_to_16_iterator);

BENCHMARK_MAIN()
