#include <cstddef>
#include <cstdint>
#include <cassert>


extern "C"
int LLVMFuzzerTestOneInput (uint8_t const * data, size_t size)
{
    if (size == 333) {
        if (data[15] == 13) {
            assert(!"Found it!");
        }
    }

    return 0;
}
