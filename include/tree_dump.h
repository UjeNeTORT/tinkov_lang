/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
 *************************************************************************/

#ifndef TREE_DUMP_H
#define TREE_DUMP_H

#include <stdio.h>

#include "tree.h"

// #define WARNINGS

const char DOT_FILE_PATH[]   = "src/tree/tree_dump/dumps/dot/";
const char GRAPH_SVGS_PATH[] = "src/tree/tree_dump/dumps/png/";
const char HTML_DUMPS_PATH[] = "src/tree/tree_dump/dumps/dumps/";
const char TEX_FILE_PATH[]   = "src/tree/tree_dump/dumps/tex/";
const char PDF_DUMPS_PATH[]  = "src/tree/tree_dump/dumps/pdf";

const int DOT_CODE_BUF_SIZE = 15000;

const int COMMAND_BUF_SIZE = 500;
const int MAX_PATH = 110;
const int MAX_TREE_FNAME = 100;

const char GRAPH_BGCLR[]    = "#EDDDD4";
const char GRAPH_TEXTCLR[]  = "#EDDDD4";
const char GRAPH_SCOPECLR[] = "#7D387D";
const char GRAPH_INTCLR[]   = "#197278";
const char GRAPH_DECCLR[]   = "#FF8966";
const char GRAPH_OPCLR[]    = "#C44536";
const char GRAPH_IDCLR[]    = "#283D3B";
const char GRAPH_ERRCLR[]   = "#000000";
const char GRAPH_KWCLR[]    = "#000000";
const char GRAPH_SEPCLR[]   = "#C45BAA";

typedef enum
{
    SIMPLE_DUMP   = 0,
    DETAILED_DUMP = 1,
} DotDumpType;

typedef enum
{
    TEX_PRINT_SUCCESS = 0,
    TEX_PRINT_ERR     = 1,
} TexTreePrintRes;

typedef enum
{
    TEX_SUBT_PRINT_SUCCESS = 0,
    TEX_SUBT_PRINT_ERR     = 1,
} TexSubtreePrintRes;

typedef enum
{
    DOT_PRINT_SUCCESS    = 0,
    DOT_PRINT_ERR        = 1,
    DOT_PRINT_ERR_PARAMS = 2,
} DotTreePrintRes;

int   TreeDotDump     (const char* fname, const Tree* tree);
FILE* InitDotDump     (const Tree* tree, char* dot_path, DotDumpType dump_type);
int   CompileDot      (char* dot_path, int dump_id, DotDumpType dump_type);
int   WriteHTML       (const char* HTML_fname, int dump_id);
int   ConcludeDotDump (FILE* tex_file);

DotTreePrintRes DotTreePrint    (FILE* dot_file, const Tree* tree);
DotTreePrintRes DotSubtreePrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id);

DotTreePrintRes DotTreeDetailedPrint    (FILE* dot_file, const Tree* tree);
DotTreePrintRes DotSubtreeDetailedPrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id);

char* GetFilePath (const char* path, const char* fname);

#endif // TREE_DUMP_H
