/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
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

    fprintf (stderr, GREEN_CLR "[DEBUG | %s:%d %s]\n<< ", funcname, line, filename);

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

    fprintf (stderr, RED_CLR "[%s:%d %s]\nERROR! ", funcname, line, filename);

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

    fprintf (stderr, CYAN_CLR "[%s:%d %s]\nWARNING! ", funcname, line, filename);

    va_list ptr;

    va_start (ptr, format);

    int res = vfprintf (stderr, format, ptr);

    va_end (ptr);

    fprintf (stdout, RST_CLR "\n" );

    return res;
}
