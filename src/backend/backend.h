/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#ifndef TINKOV_BACKEND_H
#define TINKOV_BACKEND_H

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../common/common.h"
#include "../tree/tree.h"
#include "../tree/tree_dump/tree_dump.h"
#include "../processor/assembler/asm.h"
#include "../processor/processor/spu.h"

// =========================== DSL ===========================

#define CURR_RAM_TABLE offset_table->ram_tables[offset_table->curr_table_index]
#define OFFSET_TABLE   asm_text->offset_table

#define TEXT (asm_text->text + asm_text->offset)
#define TABS (asm_text->tabs)

#define WRITE(format, ...)                   \
    {                                        \
        int written = 0;                     \
        sprintf (TEXT, format, __VA_ARGS__ __VA_OPT__ (,) &written); \
        asm_text->offset += written;         \
    }

#define NODE_IS(type, val) \
    (TYPE (node) == (type) && VAL (node) == (val))

#define NODE_IS_NOT(type, val) \
    (TYPE (node) != (type) || VAL (node) != (val))
// ===========================================================

const size_t MAX_ASM_PROGRAM_SIZE = 50000;

struct RamTable
{
    int index_in_ram[NAMETABLE_CAPACITY];
    int free_ram_index;
};

struct OffsetTable
{
    RamTable* ram_tables;
    int curr_table_index;
};

struct AsmText
{
    char* text;
    int   offset;
    OffsetTable* offset_table; // stack of RamTables to specify offset of each variable within its location in RAM
    int if_statements_count;
    int while_statements_count;
    int funcs_count;
    int cond_count;
    char* tabs;
};

typedef enum
{
    TRANSLATE_SUCCESS        = 0,
    TRANSLATE_TYPE_NOT_MATCH = 1,
    TRANSLATE_ERROR          = 2,
} TranslateRes;

AsmText*     TranslateAST                 (const Tree* ast);
TranslateRes TranslateASTSubtree          (const TreeNode* node, AsmText* asm_text, const NameTable* nametable);

TranslateRes TranslateDeclarator          (const TreeNode* declr_node, AsmText* asm_text, const NameTable* nametable);
TranslateRes TranslateKeyword             (const TreeNode* kw_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes TranslateSeparator           (const TreeNode* sep_node,   AsmText* asm_text, const NameTable* nametable);
TranslateRes TranslateOperator            (const TreeNode* op_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes TranslateIdentifier          (const TreeNode* id_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes TranslateNumber              (const TreeNode* num_node,   AsmText* asm_text, const NameTable* nametable);

int          PutFuncParamsToRAM           (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);
int          PushParamsToStack            (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);
int          PopParamsToRAM               (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);

AsmText*     AsmTextCtor                  ();
int          AsmTextDtor                  (AsmText* asm_text);

int          AsmTextAddTab                (AsmText* asm_text);
int          AsmTextRemoveTab             (AsmText* asm_text);

int          IsFunction                   (const TreeNode* id_node, const NameTable* nametable);

int          WriteCondition               (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char* comparator);

OffsetTable* OffsetTableCtor              ();
int          OffsetTableDtor              (OffsetTable* offset_table);
int          OffsetTableNewFrame          (OffsetTable* offset_table);
int          OffsetTableDeleteFrame       (OffsetTable* offset_table);
int          OffsetTableAddVariable       (OffsetTable* offset_table, int var_id);
int          OffsetTableGetVarOffset      (OffsetTable* offset_table, int var_id);
int          OffsetTableAddFuncParams     (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable);
int          OffsetTableGetCurrFrameWidth (OffsetTable* offset_table);
int          OffsetTableDump              (const OffsetTable* offset_table, const NameTable* nametable);


#endif // TINKOV_BACKEND_H
