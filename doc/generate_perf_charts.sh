#!/bin/bash

# Transconding perf output
../build/perf/transcoding_perf --benchmark_out_format=json --benchmark_out=transcoding_perf.out

# UTF-8 to UTF-16 transcoding chart
./make_perf_chart.py transcoding_perf.out BM_8_to_16_iterator_prealloc,'Iterators' BM_8_to_16_algorithm,'Algorithm std::back_inserter' BM_8_to_16_algorithm_prealloc_pointer,'Algorithm using SIMD' BM_8_to_16_algorithm_no_simd_prealloc,'Algorithm no SIMD' BM_8_to_16_algorithm_icu,ICU > utf_8_to_16_perf.svg

# UTF-8 to UTF-16 transcoding chart
./make_perf_chart.py transcoding_perf.out BM_8_to_32_iterator_prealloc,'Iterators' BM_8_to_32_algorithm,'Algorithm std::back_inserter' BM_8_to_32_algorithm_prealloc_pointer,'Algorithm using SIMD' BM_8_to_32_algorithm_no_simd_prealloc,'Algorithm no SIMD' > utf_8_to_32_perf.svg

# Normalization perf output
../build/perf/icu_normalization --benchmark_out_format=json --benchmark_out=icu_normalization.out  ../perf/portuguese_wiki.txt ../perf/korean_wiki.txt ../perf/chinese_wiki.txt ../perf/hindi_wiki_in_russian.txt

# NFC normalization chart
./make_perf_chart.py icu_normalization.out BM_text_utf8_nfc,'Algorithm with back-inserter' BM_text_utf8_nfc_string_append,'String append' BM_icu_utf8_nfc,'ICU' BM_icu_utf16_nfc,'ICU UTF-16'> norm_nfc_perf.svg

# FCC normalization chart
./make_perf_chart.py icu_normalization.out BM_text_utf8_fcc,'Algorithm with back-inserter' BM_text_utf8_fcc_string_append,'String append' BM_icu_utf8_fcc,'ICU' BM_icu_utf16_fcc,'ICU UTF-16'> norm_fcc_perf.svg
