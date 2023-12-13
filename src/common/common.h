/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/Tree
 *************************************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "colors.h"

const double EPS = 1e-7;

const int USER_ANSW_SIZE = 10;

#define streq(s1, s2) (!strcmp ((s1), (s2)))
#define dbleq(d1, d2) (fabs((d1) - (d2)) < EPS)

// todo ask ded how to not destroy PC
#define ABORT()                                              \
{                                                            \
    for (int i = 0; i < 100; i++)                            \
    {                                                        \
        printf("hehe aborting your computer and your nerves right now for free!))))))\n"); \
    }                                                        \
}                                                            \
    // system("shutdown -P now");

int PrintfDebug   (const char* funcname, int line, const char* filename, const char* format, ...); // todo COPYPASTE, need single unique function
int PrintfError   (const char* funcname, int line, const char* filename, const char* format, ...); // todo COPYPASTE, need single unique function
int PrintfWarning (const char* funcname, int line, const char* filename, const char* format, ...); // todo COPYPASTE, need single unique function

#define PRINTF_DEBUG(format, ...) \
    PrintfDebug (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)

#define WARN(format, ...) \
    PrintfWarning (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)

#define ERROR(format, ...) \
    PrintfError (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)

#define RET_ERROR(ret_val, format, ...) \
    { PrintfError (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__); \
      return ret_val;}


#endif // TOOLS_H
