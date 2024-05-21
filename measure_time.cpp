#include <assert.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

int main ()
{
    size_t ticks_begin = __rdtsc ();

    assert (system ("./exec") == 0);

    size_t ticks_end = __rdtsc ();

    size_t result = ticks_end - ticks_begin;

    printf ("res = %lu\n", result);

    return 0;
}