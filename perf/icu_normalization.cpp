#include <boost/text/normalize.hpp>

#include <unicode/normalizer2.h>

#include <benchmark/benchmark.h>

#include <fstream>
#include <iostream>


std::string file_contents;
std::vector<uint32_t> file_cps;

UnicodeString file_contents_ustr;

UErrorCode ec;
Normalizer2 const * const to_nfd = Normalizer2::getNFDInstance(ec);
Normalizer2 const * const to_nfc = Normalizer2::getNFCInstance(ec);
Normalizer2 const * const to_nfkd = Normalizer2::getNFKDInstance(ec);
Normalizer2 const * const to_nfkc = Normalizer2::getNFKCInstance(ec);


void BM_icu_utf8_nfd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::string result;
        StringByteSink<std::string> sink(&result);
        to_nfd->normalizeUTF8(0, StringPiece(file_contents), sink, nullptr, ec);
        benchmark::ClobberMemory();
    }
}

void BM_icu_utf16_nfd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(to_nfd->normalize(file_contents_ustr, ec));
    }
}

void BM_text_utf8_nfd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        auto r = boost::text::make_to_utf32_range(file_contents);
        benchmark::DoNotOptimize(
            boost::text::normalize_to_nfd(r, std::back_inserter(normalized)));
    }
}

void BM_text_utf32_nfd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        benchmark::DoNotOptimize(boost::text::normalize_to_nfd(
            file_cps, std::back_inserter(normalized)));
    }
}

void BM_icu_utf8_nfc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::string result;
        StringByteSink<std::string> sink(&result);
        to_nfc->normalizeUTF8(0, StringPiece(file_contents), sink, nullptr, ec);
        benchmark::ClobberMemory();
    }
}

void BM_icu_utf16_nfc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(to_nfc->normalize(file_contents_ustr, ec));
    }
}

void BM_text_utf8_nfc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        auto r = boost::text::make_to_utf32_range(file_contents);
        benchmark::DoNotOptimize(
            boost::text::normalize_to_nfc(r, std::back_inserter(normalized)));
    }
}

void BM_text_utf32_nfc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        benchmark::DoNotOptimize(boost::text::normalize_to_nfc(
            file_cps, std::back_inserter(normalized)));
    }
}

void BM_icu_utf8_nfkd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::string result;
        StringByteSink<std::string> sink(&result);
        to_nfkd->normalizeUTF8(
            0, StringPiece(file_contents), sink, nullptr, ec);
        benchmark::ClobberMemory();
    }
}

void BM_icu_utf16_nfkd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(to_nfkd->normalize(file_contents_ustr, ec));
    }
}

void BM_text_utf8_nfkd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        auto r = boost::text::make_to_utf32_range(file_contents);
        benchmark::DoNotOptimize(
            boost::text::normalize_to_nfkd(r, std::back_inserter(normalized)));
    }
}

void BM_text_utf32_nfkd(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        benchmark::DoNotOptimize(boost::text::normalize_to_nfkd(
            file_cps, std::back_inserter(normalized)));
    }
}

void BM_icu_utf8_nfkc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::string result;
        StringByteSink<std::string> sink(&result);
        to_nfkc->normalizeUTF8(
            0, StringPiece(file_contents), sink, nullptr, ec);
        benchmark::ClobberMemory();
    }
}

void BM_icu_utf16_nfkc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(to_nfkc->normalize(file_contents_ustr, ec));
    }
}

void BM_text_utf8_nfkc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        auto r = boost::text::make_to_utf32_range(file_contents);
        benchmark::DoNotOptimize(
            boost::text::normalize_to_nfkc(r, std::back_inserter(normalized)));
    }
}

void BM_text_utf32_nfkc(benchmark::State & state)
{
    while (state.KeepRunning()) {
        std::vector<uint32_t> normalized;
        benchmark::DoNotOptimize(boost::text::normalize_to_nfkc(
            file_cps, std::back_inserter(normalized)));
    }
}

BENCHMARK(BM_icu_utf8_nfd);
BENCHMARK(BM_icu_utf16_nfd);
BENCHMARK(BM_text_utf8_nfd);
BENCHMARK(BM_text_utf32_nfd);
BENCHMARK(BM_icu_utf8_nfc);
BENCHMARK(BM_icu_utf16_nfc);
BENCHMARK(BM_text_utf8_nfc);
BENCHMARK(BM_text_utf32_nfc);
BENCHMARK(BM_icu_utf8_nfkd);
BENCHMARK(BM_icu_utf16_nfkd);
BENCHMARK(BM_text_utf8_nfkd);
BENCHMARK(BM_text_utf32_nfkd);
BENCHMARK(BM_icu_utf8_nfkc);
BENCHMARK(BM_icu_utf16_nfkc);
BENCHMARK(BM_text_utf8_nfkc);
BENCHMARK(BM_text_utf32_nfkc);

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cerr << "Error: No input file provided." << std::endl;
        return 1;
    }

    // Last arg must be the name of the file to parse.
    char const * filename = argv[argc - 1];
    std::ifstream ifs(filename, std::ios_base::binary);

    if (!ifs) {
        std::cerr << "Error: Could not open input file: " << filename
                  << std::endl;
        return 1;
    }

    getline(ifs, file_contents, '\0');
    auto const file_cp_range = boost::text::make_to_utf32_range(file_contents);
    file_cps.assign(file_cp_range.begin(), file_cp_range.end());
    file_contents_ustr = UnicodeString::fromUTF8(StringPiece(file_contents));

    int argc_ = argc - 1; // Don't give filename arg to Benchmark.

#if 0
    if (1 < argc_ && argv[argc_ - 1] == std::string("-v")) {
        verbose = true;
        --argc_;
    }
#endif

    ::benchmark::Initialize(&argc_, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}

// BENCHMARK_MAIN()
