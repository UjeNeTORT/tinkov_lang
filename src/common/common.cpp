/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email: tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "common.h"

int PrintfDebug (const char * funcname, int line, const char * filename, const char * format, ...)
{
    assert (funcname);
    assert (filename);
    assert (format);

    #ifdef DEBUG_PRINTFS_DETAILED
        fprintf (stderr, GREEN_CLR "[DEBUG | %s:%d %s]\n<< ", funcname, line, filename);
    #else
        fprintf (stderr, GREEN_CLR "[DEBUG] ");
    #endif // DEBUG_PRINTFS_DETAILED

    va_list ptr;

    va_start (ptr, format);

    int res = vfprintf (stderr, format, ptr);

    va_end (ptr);

    fprintf (stdout, RST_CLR "\n" RST_CLR);

    return res;
}

int PrintfError (const char * funcname, int line, const char * filename, const char * format, ...)
{
    assert (funcname);
    assert (filename);
    assert (format);

    #ifdef ERROR_PRINTFS_DETAILED
        fprintf (stderr, RED_CLR "[%s:%d %s]\nERROR! ", funcname, line, filename);
    #else
        fprintf (stderr, RED_CLR "[ERROR] ");
    #endif // ERROR_PRINTFS_DETAILED

    va_list ptr;

    va_start (ptr, format);

    int res = vfprintf (stderr, format, ptr);

    va_end (ptr);

    fprintf (stdout, RST_CLR "\n" );

    return res;
}

int PrintfWarning (const char * funcname, int line, const char * filename, const char * format, ...)
{
    assert (funcname);
    assert (filename);
    assert (format);

    #ifdef ERROR_PRINTFS_DETAILED
        fprintf (stderr, MAGENTA_CLR "[%s:%d %s]\nWARNING: ", funcname, line, filename);
    #else
        fprintf (stderr, RED_CLR "[ERROR] ");
    #endif // ERROR_PRINTFS_DETAILED

    va_list ptr;

    va_start (ptr, format);

    int res = vfprintf (stderr, format, ptr);

    va_end (ptr);

    fprintf (stdout, RST_CLR "\n" );

    return res;
}

int PrintfLog (const char * funcname, const char * filename, const char * format, ...)
{
    assert (funcname);
    assert (filename);
    assert (format);

    #ifdef LOGS_DETAILED
        fprintf (stderr, CYAN_CLR "[log from %s:%d %s] ", funcname, line, filename);
    #else
        fprintf (stderr, CYAN_CLR "[LOG] ");
    #endif // LOGS_DETAILED

    va_list ptr;

    va_start (ptr, format);

    int res = vfprintf (stderr, format, ptr);

    va_end (ptr);

    fprintf (stdout, RST_CLR "\n" );

    return res;
}

StackLight* StackLightCtor (size_t capacity)
{
    assert (capacity != 0);

    StackLight *stack = (StackLight *) calloc (1, sizeof (StackLight));
    stack->buffer   = (int *) calloc (capacity, sizeof (int));
    stack->capacity = capacity;

    return stack;
}

int StackLightDtor (StackLight* stack)
{
    assert (stack);

    free (stack->buffer);
    free (stack);

    return 0;
}

int StackLightPush (StackLight* stack, int val)
{
    assert (stack);
    if (stack->sp >= stack->capacity) return 1;

    stack->buffer[stack->sp] = val;
    stack->sp++;

    return 0;
}

int StackLightPop  (StackLight* stack)
{
    assert (stack);

    stack->sp--;
    int ret_val = stack->buffer[stack->sp];
    stack->buffer[stack->sp] = 0;

    return ret_val;
}

int PrintProgressBar (unsigned curr_progress, unsigned max_progress)
{
    assert (curr_progress <= max_progress);
    // assert (max != 0);

    const char prefix[] = BLUE ("---[");
    const char suffix[] = BLUE ("]---");

    char progress_bar[PROGRESS_BAR_LENGTH + 1] = {};

    for (size_t i = 0; i < PROGRESS_BAR_LENGTH; i++)
        progress_bar[i] = (i * max_progress < curr_progress * PROGRESS_BAR_LENGTH) ? '#' : '_';

    fprintf (stderr, "\r%s%s%s (%d/%d)",
                    prefix, progress_bar, suffix, curr_progress, max_progress);

    return 0;
}

u_int64_t GetCPUTicks ()
{
    u_int64_t ticks_rax = 0;
    u_int64_t ticks_rdx = 0;

    asm volatile (
        "rdtsc\n\t"
        : "=a" (ticks_rax), "=d" (ticks_rdx)
    );

    return (ticks_rdx << 32) | ticks_rax;
}