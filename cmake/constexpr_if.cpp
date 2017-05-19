int main ()
{
#if CHECK_CONSTEXPR_IF
    if constexpr (true) {
        return 0;
    }
#endif
}
