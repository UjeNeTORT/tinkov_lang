/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "colors.h"

const double EPS = 1e-7;

const int PROGRESS_BAR_LENGTH = 50;

#define LOGS
#define WARNINGS
// #define LOGS_DETAILED
#define DEBUG_PRINTFS_DETAILED
#define ERROR_PRINTFS_DETAILED

#define streq(s1, s2) (!strcmp ((s1), (s2)))
#define dbleq(d1, d2) (fabs((d1) - (d2)) < EPS)
#define sizearr(arr) sizeof(arr) / sizeof((arr)[0])

typedef enum
{
    FUNC_SUCCESS = 0,
    FUNC_ERROR   = 1,
} DefaultFuncRes;

int PrintfDebug   (const char* funcname, int line, const char* filename, const char* format, ...);
int PrintfWarning (const char * funcname, int line, const char * filename, const char * format, ...);
int PrintfLog     (const char* funcname, int line, const char* filename, const char* format, ...);
int PrintfError   (const char* funcname, int line, const char* filename, const char* format, ...);

#define PRINTF_DEBUG(format, ...) \
    PrintfDebug (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)

#ifdef WARNINGS
    #define WARN(format, ...) \
    PrintfWarning (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)
#else // WARNINGS
    #define WARN(format, ...) ;
#endif // WARNINGS

#ifdef LOGS
    #define LOG(format, ...) \
    PrintfLog (__FUNCTION__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)
#else // LOGS
    #define LOG(format, ...) ;
#endif // LOGS

#define ERROR(format, ...) \
    PrintfError (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__)

#define RET_ERROR(ret_val, format, ...) \
    { PrintfError (__FUNCTION__, __LINE__, __FILE__, format __VA_OPT__(,) __VA_ARGS__); \
      return ret_val;}

int PrintProgressBar (unsigned count, unsigned max);

u_int64_t GetCPUTicks ();

#endif // TOOLS_H

