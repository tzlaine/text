#include <boost/text/collation_search.hpp>
#include <boost/text/text.hpp>

#ifndef NO_ICU
#include <unicode/coll.h>
#include <unicode/sortkey.h>
#include <unicode/unistr.h>
#include <unicode/stsearch.h>
U_NAMESPACE_USE
#endif

#include <benchmark/benchmark.h>


boost::text::collation_table default_table =
    boost::text::default_collation_table();

// Lorem ipsum, repeated 10 times, with "sint" and "proident" capitalized in
// all but the last copy.  This gives us a chance to look for our patterns,
// only find them near the end, and includes several backtracking
// opportunities.
boost::text::text const short_pattern = "int";
boost::text::text const long_pattern = "occaecat cupidatat non proident";
boost::text::text const corpus =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur SINT "
    "occaecat cupidatat non PROIDENT, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum."
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";
#ifndef NO_ICU
UErrorCode ec = U_ZERO_ERROR;
U_NAMESPACE_QUALIFIER UnicodeString const icu_short_pattern = "int";
U_NAMESPACE_QUALIFIER UnicodeString const icu_long_pattern =
    "occaecat cupidatat non proident";
U_NAMESPACE_QUALIFIER UnicodeString const icu_corpus =
    corpus.begin().base().base();
#endif


void BM_search_convenience_short(benchmark::State & state)
{
    while (state.KeepRunning()) {
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(boost::text::collation_search(
                corpus, short_pattern, default_table));
        }
    }
}

void BM_search_convenience_long(benchmark::State & state)
{
    while (state.KeepRunning()) {
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(boost::text::collation_search(
                corpus, long_pattern, default_table));
        }
    }
}

void BM_search_simple_short(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher = boost::text::make_simple_collation_searcher(
            short_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

void BM_search_simple_long(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher = boost::text::make_simple_collation_searcher(
            long_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

void BM_search_bm_short(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher = boost::text::make_boyer_moore_collation_searcher(
            short_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

void BM_search_bm_long(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher = boost::text::make_boyer_moore_collation_searcher(
            long_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

#ifndef NO_ICU
void BM_search_bm_icu_short(benchmark::State & state)
{
    while (state.KeepRunning()) {
        for (int i = 0, end = state.range(0); i < end; ++i) {
            ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER StringSearch iter(
                icu_short_pattern,
                icu_corpus,
                U_NAMESPACE_QUALIFIER Locale::getUS(),
                nullptr,
                ec);
            ec = U_ZERO_ERROR;
            benchmark::DoNotOptimize(iter.first(ec));
        }
    }
}

void BM_search_bm_icu_long(benchmark::State & state)
{
    while (state.KeepRunning()) {
        for (int i = 0, end = state.range(0); i < end; ++i) {
            ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER StringSearch iter(
                icu_long_pattern,
                icu_corpus,
                U_NAMESPACE_QUALIFIER Locale::getUS(),
                nullptr,
                ec);
            ec = U_ZERO_ERROR;
            benchmark::DoNotOptimize(iter.first(ec));
        }
    }
}
#endif

void BM_search_bmh_short(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher =
            boost::text::make_boyer_moore_horspool_collation_searcher(
                short_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

void BM_search_bmh_long(benchmark::State & state)
{
    while (state.KeepRunning()) {
        auto const searcher =
            boost::text::make_boyer_moore_horspool_collation_searcher(
                long_pattern, default_table);
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(
                boost::text::collation_search(corpus, searcher));
        }
    }
}

BENCHMARK(BM_search_convenience_short)->Range(1, 2 << 7);
BENCHMARK(BM_search_convenience_long)->Range(1, 2 << 7);
BENCHMARK(BM_search_simple_short)->Range(1, 2 << 7);
BENCHMARK(BM_search_simple_long)->Range(1, 2 << 7);
BENCHMARK(BM_search_bm_short)->Range(1, 2 << 7);
BENCHMARK(BM_search_bm_long)->Range(1, 2 << 7);
#ifndef NO_ICU
BENCHMARK(BM_search_bm_icu_short)->Range(1, 2 << 7);
BENCHMARK(BM_search_bm_icu_long)->Range(1, 2 << 7);
#endif
BENCHMARK(BM_search_bmh_short)->Range(1, 2 << 7);
BENCHMARK(BM_search_bmh_long)->Range(1, 2 << 7);

BENCHMARK_MAIN()
