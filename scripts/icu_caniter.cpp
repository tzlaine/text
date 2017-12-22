#include <unicode/caniter.h>


extern "C" {

static int results[1024];

/** Returns a null-terminated list of null-terminated utf32 strings. */
int * canonical_closure(int * utf32_str, int len)
{
    UErrorCode err = U_ZERO_ERROR;
    icu_60::UnicodeString str =
        icu_60::UnicodeString::fromUTF32(utf32_str, len);
    icu_60::CanonicalIterator can_iter(str, err);
    int * out_it = results;
    while (true) {
        icu_60::UnicodeString permutation = can_iter.next();
        if (permutation.isBogus()) {
            *out_it = 0;
            break;
        }
        out_it += permutation.toUTF32(out_it, 1024, err);
        *out_it++ = 0;
    }
    return results;
}
}

#ifdef TEST
int main()
{
    icu_60::UnicodeString str = icu_60::UnicodeString::fromUTF8("ǭ");
    if (str.isBogus()) {
        printf("Error creating string from UTF8\n");
        return 1;
    }
    int utf32[1024];
    UErrorCode err = U_ZERO_ERROR;
    auto const len = str.toUTF32(utf32, 1024, err);
    if (U_FAILURE(err)) {
        printf("Error converting to UTF32: %s\n", u_errorName(err));
        return 1;
    }
    icu_60::CanonicalIterator can_iter(str, err);
    if (U_FAILURE(err)) {
        printf("Error creating CanonicalIterator: %s\n", u_errorName(err));
        return 1;
    }
    while (true) {
        icu_60::UnicodeString permutation = can_iter.next();
        if (permutation.isBogus()) {
            printf("Got bogus string; terminating....\n");
            break;
        }
        printf("Got non-bogus string.\n");
    }
}
#endif
