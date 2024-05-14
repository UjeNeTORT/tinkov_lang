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

// push reg/imm/mem to calc stack
#define CPUSH(r_i_m, ...) WRITE ("sub r15, 8\t\t; push to calc stack\n"); \
                          WRITE ("mov [r15], " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)

// pop value from calc stack to reg/mem
#define CPOP(r_m, ...) WRITE ("mov " r_m ", QWORD [r15]  ; pop from calc stack\n"); \
                       WRITE ("add r15, 8\n" __VA_OPT__(,) __VA_ARGS__)

#define PUSH(r_i_m, ...) WRITE ("push " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define POP(r_m, ...)    WRITE ("pop " r_m "\n"    __VA_OPT__(,) __VA_ARGS__)

#define MOV(r_m, r_i_m, ...) WRITE ("mov " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define RET WRITE ("ret\n\n")

#define COND_COUNT  asm_text->cond_count
#define IF_COUNT    asm_text->if_statements_count
#define WHILE_COUNT asm_text->while_statements_count

#define CURR_N_PARAMS  OFFSET_TABLE->ram_tables[OFFSET_TABLE->curr_table_index].n_params
#define CURR_RAM_TABLE offset_table->ram_tables[offset_table->curr_table_index]
#define OFFSET_TABLE   asm_text->offset_table

#define TEXT (asm_text->text + asm_text->offset)
#define TABS (asm_text->tabs)

#define WRITE_(format, ...)                                          \
    {                                                                \
        int written = 0;                                             \
        sprintf (TEXT, format, __VA_ARGS__ __VA_OPT__ (,) &written); \
        asm_text->offset += written;                                 \
    }

#define WRITE(format, ...) WRITE_ ("%s" format "%n", TABS __VA_OPT__(,) __VA_ARGS__)

#define WRITE_NO_TAB(format, ...) WRITE_ (format "%n", __VA_ARGS__)

#define NODE_IS(type, val) \
    (TYPE (node) == (type) && VAL (node) == (val))

#define NODE_IS_NOT(type, val) \
    (TYPE (node) != (type) || VAL (node) != (val))
// ===========================================================

const size_t MAX_ASM_PROGRAM_SIZE    = 50000;
const size_t LOCAL_VARIABLES_MAPPING = 5;
const size_t OFFSET_TABLE_CAPACITY   = 256; // former RAM_TABLE_CAPACITY

struct RamTable
{
    int index_in_ram[NAMETABLE_CAPACITY];   // stack of base-relative ram offsets of each variable
    size_t free_ram_index;                  // stack pointer
    size_t n_params;                        // n of parameters of func represented by current ram table
};

struct OffsetTable
{
    RamTable* ram_tables;       // stack of ram tables
    size_t    curr_table_index; // stack pointer

    size_t n_params;            // number of parameters of curr function
    int   *offset_table;
    size_t offset_table_ptr;
    size_t offset_table_base;

    StackLight *base_stack;
};

struct AsmText
{
    char*  text;
    size_t offset;
    OffsetTable* offset_table; // stack of RamTables to specify offset of each variable within its scope in memory
    size_t if_statements_count;
    size_t while_statements_count;
    size_t funcs_count;
    size_t cond_count;
    char*  tabs;
};

typedef enum
{
    TRANSLATE_SUCCESS        = 0,
    TRANSLATE_TYPE_NOT_MATCH = 1,
    TRANSLATE_ERROR          = 2,
    TRANSLATE_DECLR_ERROR    = 3,
} TranslateRes;

typedef enum
{
    VAR_NOT_IN_REGISTER = -1,
    RIP_REG             =  0,
    RAX_REG             =  1,
    RBX_REG             =  2,
    RCX_REG             =  3,
    RDX_REG             =  4,
    RSI_REG             =  5,
    RDI_REG             =  6,
    R8_REG              =  7,
    R9_REG              =  8,
    R10_REG             =  9,
    R11_REG             = 10,
    R12_REG             = 11,
    R13_REG             = 12,
    R14_REG             = 13,
    R15_REG             = 14,
    RBP_REG             = 15,
    RSP_REG             = 16,
} REGISTER;

const char *REG_NAMES[16] = {"rax", "rbx", "rcx", "rdx",
                             "rsi", "rdi",
                             "r8",  "r9",  "r10", "r11",
                             "r12", "r13", "r14", "r15",
                             "rbp", "rsp"};

AsmText*       TranslateAST                 (const TreeNode* root_node, AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateASTSubtree          (const TreeNode* node, AsmText* asm_text, const NameTable* nametable);

TranslateRes   TranslateDeclarator          (const TreeNode* declr_node, AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateKeyword             (const TreeNode* kw_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateSeparator           (const TreeNode* sep_node,   AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateOperator            (const TreeNode* op_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateIdentifier          (const TreeNode* id_node,    AsmText* asm_text, const NameTable* nametable);
TranslateRes   TranslateNumber              (const TreeNode* num_node,   AsmText* asm_text, const NameTable* nametable);

DefaultFuncRes PutFuncParamsToRAM           (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);
DefaultFuncRes PushParamsToStack            (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);
DefaultFuncRes PopParamsToRAM               (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);

AsmText*       AsmTextCtor                  ();
DefaultFuncRes AsmTextDtor                  (AsmText* asm_text);

DefaultFuncRes AsmTextAddTab                (AsmText* asm_text);
DefaultFuncRes AsmTextRemoveTab             (AsmText* asm_text);

int            IsFunction                   (const TreeNode *id_node, const NameTable *nametable);

DefaultFuncRes WriteCondition               (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char* comparator);
DefaultFuncRes TranslateCondition           (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char *jmp_comparator);

OffsetTable*   OffsetTableCtor              ();
DefaultFuncRes OffsetTableDtor              (OffsetTable* offset_table);
int            OffsetTableGetCurrFrameWidth (OffsetTable* offset_table);
DefaultFuncRes OffsetTableAddFrame          (OffsetTable* offset_table, size_t n_params);
int            OffsetTableGetVarOffset      (OffsetTable* offset_table, size_t var_id);
DefaultFuncRes OffsetTableDeleteFrame       (OffsetTable* offset_table);
DefaultFuncRes OffsetTableAddVariable       (OffsetTable* offset_table, size_t var_id);
DefaultFuncRes OffsetTableAddFuncParams     (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable);
DefaultFuncRes OffsetTableAddFuncLocals     (OffsetTable* offset_table, const TreeNode* func_body_node, const NameTable* nametable);
DefaultFuncRes OffsetTableDump              (const OffsetTable* offset_table, const NameTable* nametable);

int            DescribeCurrFunction         (AsmText *asm_text, const NameTable *nametable);

#endif // TINKOV_BACKEND_H
