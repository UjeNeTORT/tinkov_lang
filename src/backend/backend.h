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

#define ASSERT_BLOCK_FUNC_TRANSLATE \
    assert (asm_text);              \
    assert (asm_text->text);        \
    assert (ast);                   \
    assert (ast->root);             \
    assert (ast->nametable);

#define IN_RAM(node) asm_text->ram_table.index_in_ram[VAL (node)]

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

struct AsmText
{
    char* text;
    int   offset;
    RamTable ram_table; // table to specify location in ram where identifier values are stored
    int if_statements_count;
    int while_statements_count;
    int funcs_count;
    char* tabs;
};

typedef enum
{
    TRANSLATE_SUCCESS        = 0,
    TRANSLATE_TYPE_NOT_MATCH = 1,
    TRANSLATE_ERROR          = 2,
} TranslateRes;

AsmText*     TranslateAST (const Tree* ast);
TranslateRes TranslateASTSubtree (const TreeNode* node, AsmText* asm_text, const Tree* ast);

TranslateRes TranslateDeclarator (const TreeNode* declr_node, AsmText* asm_text, const Tree* ast);
TranslateRes TranslateKeyword    (const TreeNode* kw_node,    AsmText* asm_text, const Tree* ast);
TranslateRes TranslateSeparator  (const TreeNode* sep_node,   AsmText* asm_text, const Tree* ast);
TranslateRes TranslateOperator   (const TreeNode* op_node,    AsmText* asm_text, const Tree* ast);
TranslateRes TranslateIdentifier (const TreeNode* id_node,    AsmText* asm_text, const Tree* ast);
TranslateRes TranslateNumber     (const TreeNode* num_node,   AsmText* asm_text, const Tree* ast);

int AddIdToRAM (int identifier_index, AsmText* ast);

AsmText* AsmTextCtor ();
int      AsmTextDtor (AsmText* asm_text);

int AsmTextAddTab    (AsmText* asm_text);
int AsmTextRemoveTab (AsmText* asm_text);

#endif // TINKOV_BACKEND_H
