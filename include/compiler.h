/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
 *************************************************************************/

#ifndef TINKOV_COMPILER_H
#define TINKOV_COMPILER_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <common.h>

#include "common.h"

const int MAX_COMMAND = 1000;

DefaultFuncRes CompileTinkovProgram         (char* filename);

DefaultFuncRes IsProgramTextFilenameCorrect (const char* program_text_filename);

#endif // TINKOV_COMPILER_H
