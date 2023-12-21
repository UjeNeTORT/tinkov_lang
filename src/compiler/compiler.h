/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#ifndef TINKOV_COMPILER_H
#define TINKOV_COMPILER_H

#include <stdio.h>

#include "../common/common.h"
#include "../frontend/frontend.h"
#include "../backend/backend.h"

const int MAX_COMMAND = 1000;

int CompileTinkovProgram (char* filename);

#endif // TINKOV_COMPILER_H
