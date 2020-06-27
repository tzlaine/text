// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/collate.hpp>
#include <boost/text/text.hpp>
#include <boost/text/data/da.hpp>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include <benchmark/benchmark.h>

#include <iostream>
#include <set>


#define BE_REASONABLE 1
#if BE_REASONABLE
template<typename T, typename Cmp = std::less<T>>
using set_t = boost::container::flat_multiset<T, Cmp>;
template<typename Key, typename T>
using map_t = boost::container::flat_multimap<Key, T>;
#else
template<typename T, typename Cmp = std::less<T>>
using set_t = std::multiset<T, Cmp>;
template<typename Key, typename T>
using map_t = std::multimap<Key, T>;
#endif

//[ collation_text_cmp_naive
struct text_cmp
{
    bool operator()(
        boost::text::text const & lhs, boost::text::text const & rhs) const
        noexcept
    {
        // Binary comparison of code point values.
        return std::lexicographical_compare(
            lhs.begin().base(),
            lhs.end().base(),
            rhs.begin().base(),
            rhs.end().base());
    }
};
//]

//[ collation_text_cmp
struct text_cmp_2
{
    bool operator()(
        boost::text::text const & lhs, boost::text::text const & rhs) const
        noexcept
    {
        // Binary comparison of code point values.
        return std::lexicographical_compare(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
};
//]

//[ collation_text_coll_cmp
struct text_coll_cmp
{
    bool operator()(
        boost::text::text const & lhs, boost::text::text const & rhs) const
        noexcept
    {
        return boost::text::collate(lhs, rhs, table) < 0;
    }

    boost::text::collation_table table; // Cheap to copy.
};
//]

std::vector<boost::text::text> make_texts()
{
    std::vector<boost::text::text> retval;
    for (int i = 0; i < 2 << 15; ++i) {
        retval.push_back(boost::text::text(std::to_string(i)));
    }
    return retval;
}
std::vector<boost::text::text> const texts = make_texts();

boost::text::collation_table default_table =
    boost::text::default_collation_table();

void BM_set_inserts_binary_compare_text_naive(benchmark::State & state)
{
    while (state.KeepRunning()) {
        set_t<boost::text::text, text_cmp> set;
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(set.insert(texts[i]));
        }
    }
}

void BM_set_inserts_binary_compare_text(benchmark::State & state)
{
    while (state.KeepRunning()) {
        set_t<boost::text::text, text_cmp_2> set;
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(set.insert(texts[i]));
        }
    }
}

void BM_set_inserts_collate(benchmark::State & state)
{
    while (state.KeepRunning()) {
        set_t<boost::text::text, text_coll_cmp> set(
            text_coll_cmp{default_table});
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(set.insert(texts[i]));
        }
    }
}

void BM_map_inserts(benchmark::State & state)
{
    while (state.KeepRunning()) {
        map_t<boost::text::text_sort_key, boost::text::text> map;
        for (int i = 0, end = state.range(0); i < end; ++i) {
            benchmark::DoNotOptimize(map.emplace(
                boost::text::collation_sort_key(texts[i], default_table),
                texts[i]));
        }
    }
}

BENCHMARK(BM_set_inserts_binary_compare_text_naive)->Range(16, 2 << 12);
BENCHMARK(BM_set_inserts_binary_compare_text)->Range(16, 2 << 12);
BENCHMARK(BM_set_inserts_collate)->Range(16, 2 << 12);
BENCHMARK(BM_map_inserts)->Range(16, 2 << 12);

BENCHMARK_MAIN()
