// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/collate.hpp>
#include <boost/text/transcode_view.hpp>

#ifndef NO_ICU
#include <unicode/coll.h>
#include <unicode/sortkey.h>
#include <unicode/unistr.h>
U_NAMESPACE_USE
#endif

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

std::string get_utf8_text(bool european)
{
    if (european) {
        return file_slurp("portuguese_wiki.txt") +
               file_slurp("english_wiki.txt") +
               file_slurp("perf/portuguese_wiki.txt") +
               file_slurp("perf/english_wiki.txt") +
               file_slurp("../perf/portuguese_wiki.txt") +
               file_slurp("../perf/english_wiki.txt");
    } else {
        return file_slurp("korean_wiki.txt") +
               file_slurp("chinese_wiki.txt") +
               file_slurp("hindi_wiki_in_russian.txt") +
               file_slurp("perf/korean_wiki.txt") +
               file_slurp("perf/chinese_wiki.txt") +
               file_slurp("perf/hindi_wiki_in_russian.txt") +
               file_slurp("../perf/korean_wiki.txt") +
               file_slurp("../perf/chinese_wiki.txt") +
               file_slurp("../perf/hindi_wiki_in_russian.txt");
    }
}

int string_compare(std::string const & lhs, std::string const & rhs)
{
    return strcmp(lhs.c_str(), rhs.c_str());
}

boost::text::collation_table const table =
    boost::text::default_collation_table();

std::vector<std::string> strings;
#ifndef NO_ICU
std::vector<U_NAMESPACE_QUALIFIER UnicodeString> icu_strings;

UErrorCode ec = U_ZERO_ERROR;
U_NAMESPACE_QUALIFIER Collator * coll =
    U_NAMESPACE_QUALIFIER Collator::createInstance(ec);
#endif


void BM_string_lex_compare(benchmark::State & state)
{
    auto const * str0 = &strings[0];
    while (state.KeepRunning()) {
        for (auto const & x : strings) {
            benchmark::DoNotOptimize(string_compare(x, *str0));
            benchmark::DoNotOptimize(string_compare(*str0, x));
            str0 = &x;
        }
    }
}

void BM_string_lex_sort(benchmark::State & state)
{
    std::vector<std::string> local_strings = strings;
    while (state.KeepRunning()) {
        state.PauseTiming();
        local_strings = strings;
        state.ResumeTiming();
        std::sort(
            local_strings.begin(),
            local_strings.end(),
            [](std::string const & lhs, std::string const & rhs) {
                return string_compare(lhs, rhs) < 0;
            });
        benchmark::ClobberMemory();
    }
}

void BM_string_collate(benchmark::State & state)
{
    auto str0 = boost::text::as_utf32(strings[0]);
    while (state.KeepRunning()) {
        for (auto & x : strings) {
            benchmark::DoNotOptimize(
                boost::text::collate(boost::text::as_utf32(x), str0, table));
            benchmark::DoNotOptimize(
                boost::text::collate(str0, boost::text::as_utf32(x), table));
            str0 = boost::text::as_utf32(x);
        }
    }
}

void BM_string_make_collation_key(benchmark::State & state)
{
    while (state.KeepRunning()) {
        for (auto const & x : strings) {
            benchmark::DoNotOptimize(boost::text::collation_sort_key(
                boost::text::as_utf32(x), table));
        }
    }
}

void BM_string_collation_sort(benchmark::State & state)
{
    std::vector<std::string> local_strings = strings;
    while (state.KeepRunning()) {
        state.PauseTiming();
        local_strings = strings;
        state.ResumeTiming();
        std::sort(
            local_strings.begin(),
            local_strings.end(),
            [](std::string const & lhs,
               std::string const & rhs) {
                return boost::text::collate(
                           boost::text::as_utf32(lhs),
                           boost::text::as_utf32(rhs),
                           table) < 0;
            });
        benchmark::ClobberMemory();
    }
}

#ifndef NO_ICU
void BM_icu_string_collate(benchmark::State & state)
{
    auto const * str0 = &icu_strings[0];
    while (state.KeepRunning()) {
        for (auto const & x : icu_strings) {
            benchmark::DoNotOptimize(coll->compare(x, *str0));
            benchmark::DoNotOptimize(coll->compare(*str0, x));
            str0 = &x;
        }
    }
}

void BM_icu_string_make_collation_key(benchmark::State & state)
{
    U_NAMESPACE_QUALIFIER CollationKey key;
    while (state.KeepRunning()) {
        for (auto const & x : icu_strings) {
            UErrorCode ec = U_ZERO_ERROR;
            benchmark::DoNotOptimize(coll->getCollationKey(x, key, ec));
        }
    }
}

void BM_icu_string_collation_sort(benchmark::State & state)
{
    std::vector<U_NAMESPACE_QUALIFIER UnicodeString> local_icu_strings =
        icu_strings;
    while (state.KeepRunning()) {
        state.PauseTiming();
        local_icu_strings = icu_strings;
        state.ResumeTiming();
        std::sort(
            local_icu_strings.begin(),
            local_icu_strings.end(),
            [](U_NAMESPACE_QUALIFIER UnicodeString const & lhs,
               U_NAMESPACE_QUALIFIER UnicodeString const & rhs) {
                return coll->compare(lhs, rhs) ==
                       U_NAMESPACE_QUALIFIER Collator::LESS;
            });
        benchmark::ClobberMemory();
    }
}

void BM_icu_string_collate_utf8(benchmark::State & state)
{
    U_NAMESPACE_QUALIFIER StringPiece str0(strings[0]);
    while (state.KeepRunning()) {
        for (auto const & x_ : strings) {
            U_NAMESPACE_QUALIFIER StringPiece x(x_);
            UErrorCode ec = U_ZERO_ERROR;
            benchmark::DoNotOptimize(coll->compareUTF8(x, str0, ec));
            ec = U_ZERO_ERROR;
            benchmark::DoNotOptimize(coll->compareUTF8(str0, x, ec));
            str0 = x;
        }
    }
}

void BM_icu_string_collation_sort_utf8(benchmark::State & state)
{
    std::vector<std::string> local_strings = strings;
    while (state.KeepRunning()) {
        state.PauseTiming();
        local_strings = strings;
        state.ResumeTiming();
        std::sort(
            local_strings.begin(),
            local_strings.end(),
            [](std::string const & lhs_, std::string & rhs_) {
                U_NAMESPACE_QUALIFIER StringPiece lhs(lhs_);
                U_NAMESPACE_QUALIFIER StringPiece rhs(rhs_);
                UErrorCode ec = U_ZERO_ERROR;
                return coll->compareUTF8(lhs, rhs, ec) ==
                       U_NAMESPACE_QUALIFIER Collator::LESS;
            });
        benchmark::ClobberMemory();
    }
}
#endif

BENCHMARK(BM_string_lex_compare);
BENCHMARK(BM_string_lex_sort);
BENCHMARK(BM_string_collate);
BENCHMARK(BM_string_make_collation_key);
BENCHMARK(BM_string_collation_sort);
#ifndef NO_ICU
BENCHMARK(BM_icu_string_collate);
BENCHMARK(BM_icu_string_make_collation_key);
BENCHMARK(BM_icu_string_collation_sort);
BENCHMARK(BM_icu_string_collate_utf8);
BENCHMARK(BM_icu_string_collation_sort_utf8);
#endif

std::string usage_string()
{
    return "Usage: icu_normalization [--help] --european|--non-european "
        "[Google benchmark params ...]";
}

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cerr << "Error: Not enough parameters.\n"
                  << usage_string() << std::endl;
        return 1;
    }

    std::string param_0 = argv[1];
    if (param_0 == "--help" ||
        (param_0 != "--european" && param_0 != "--non-european")) {
        std::cerr << usage_string() << std::endl;
        return 1;
    }

    std::string const file_contents = get_utf8_text(param_0 == "--european");
    std::pair<std::ptrdiff_t, std::ptrdiff_t> sizes_and_strides[] = {
        {10, 40},
        {30, 60},
        {50, 80},
        {70, 100},
        {90, 120},
        {140, 170},
        {190, 230},
        {500, 510},
    };
    auto const contents_first = file_contents.begin();
    for (auto size_and_stride : sizes_and_strides) {
        for (std::ptrdiff_t i = 0; i + size_and_stride.first <
                                   (std::ptrdiff_t)file_contents.size() / 500;
             i += size_and_stride.second) {
            strings.push_back(std::string(
                contents_first + i,
                contents_first + i + size_and_stride.first));
#ifndef NO_ICU
            icu_strings.push_back(U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(
                U_NAMESPACE_QUALIFIER StringPiece(
                    &*contents_first + i, size_and_stride.first)));
#endif
        }
    }

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}
