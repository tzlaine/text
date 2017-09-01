#include <string>


namespace {

    std::string make_string(int size) { return std::string(size, '.'); }

    std::string strings[14] = {make_string(0),
                               make_string(1 << 0),
                               make_string(1 << 1),
                               make_string(1 << 2),
                               make_string(1 << 3),
                               make_string(1 << 4),
                               make_string(1 << 5),
                               make_string(1 << 6),
                               make_string(1 << 8),
                               make_string(1 << 10),
                               make_string(1 << 12),
                               make_string(1 << 16),
                               make_string(1 << 20),
                               make_string(1 << 30)};
}

#define BENCHMARK_ARGS()                                                       \
    ->Arg(0)                                                                   \
        ->Arg(1)                                                               \
        ->Arg(2)                                                               \
        ->Arg(3)                                                               \
        ->Arg(4)                                                               \
        ->Arg(5)                                                               \
        ->Arg(6)                                                               \
        ->Arg(7)                                                               \
        ->Arg(8)                                                               \
        ->Arg(9)                                                               \
        ->Arg(10)                                                              \
        ->Arg(11)                                                              \
        ->Arg(12)                                                              \
        ->Arg(13)
