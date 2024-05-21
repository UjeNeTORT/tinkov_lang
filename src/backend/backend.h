/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
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

// push reg/imm/mem to calc stack (r15)
#define CPUSH(r_i_m, ...) WRITE ("sub r15, 8\t\t\t; cpush\n"); \
                          WRITE ("mov QWORD [r15], " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)

// pop value from calc stack (r15) to reg/mem
#define CPOP(r_m, ...) WRITE ("mov " r_m ", QWORD [r15]\t\t; cpop\n"); \
                       WRITE ("add r15, 8\n" __VA_OPT__(,) __VA_ARGS__)

#define PUSH(r_i_m, ...) WRITE ("push " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define POP(r_m, ...)    WRITE ("pop " r_m "\n"    __VA_OPT__(,) __VA_ARGS__)

/**
 * pop value from calc stack (r15) and push it to the real one (rsp)
 *
 * DESTR: rax
*/
#define REPUSH PUSH ("rax\t\t\t; repush begin");        \
               CPOP ("rax");                            \
               XCHG ("rax", "QWORD [rsp]\t\t; repush end")

#define CALL(func_name, ...)    WRITE ("call " func_name "\n" __VA_OPT__(,) __VA_ARGS__ )
#define MOV(r_m, r_i_m, ...)    WRITE ("mov " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define XCHG(r_m, r_i_m, ...)   WRITE ("xchg " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define CMP(r_m, r_i_m, ...)    WRITE ("cmp " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)

#define LEA(r_m_1, m, ...)      WRITE ("lea " r_m_1 ", " m "\n" __VA_OPT__(,) __VA_ARGS__)
#define XOR(r_m_1, r_m_2, ...)  WRITE ("xor " r_m_1 ", " r_m_2 "\n" __VA_OPT__(,) __VA_ARGS__)
#define ADD(r_m, r_i_m, ...)    WRITE ("add " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define SUB(r_m, r_i_m, ...)    WRITE ("sub " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)
#define IMUL(r_m, r_i_m, ...)   WRITE ("imul " r_m ", " r_i_m "\n" __VA_OPT__(,) __VA_ARGS__)

#define RET                     WRITE ("ret\n")

#define COND_COUNT  asm_text->cond_count
#define IF_COUNT    asm_text->if_statements_count
#define WHILE_COUNT asm_text->while_statements_count

#define CURR_N_PARAMS       OFFSET_TABLE->ram_tables[OFFSET_TABLE->curr_table_index].n_params
#define CURR_RAM_TABLE      offset_table->ram_tables[offset_table->curr_table_index]
#define OFFSET_TABLE        asm_text->offset_table
#define EFF_OFFSET(id_node) OffsetTableGetVarOffset (OFFSET_TABLE, VAL (id_node))

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
#define WRITE_NEWLINE WRITE_NO_TAB ("\n")

#define NODE_IS(type, val) \
    (TYPE (node) == (type) && VAL (node) == (val))

#define NODE_IS_NOT(type, val) \
    (TYPE (node) != (type) || VAL (node) != (val))
// ===========================================================

const unsigned INT_PRECISION_POW       = 2;
const unsigned QWORD_SIZE              = 8;
const size_t   MAX_ASM_PROGRAM_SIZE    = 50000;
const size_t   OFFSET_TABLE_CAPACITY   = 256;
const size_t   CALC_STACK_CAPACITY     = 2048;

const char * const DFLT_STDLIB_PATH    = "/home/netort/language/src/stdlib_tnkff/stdlib_tnkff.s";

struct OffsetTable
{
    int   *offset_table;        // array of function params and loc.vars
                                // first n_params values - ids of params
                                // all the rest          - local vars

    size_t offset_table_ptr;    // points to a free position
    size_t offset_table_base;   // points to a curr frame base
    StackLight *base_stack;     // stores all the frame base ptrs

    size_t n_params;            // number of parameters of curr function
};

struct AsmText
{
    char*  text;
    size_t offset;
    OffsetTable* offset_table; // stores offset of each variable within its scope in memory
    size_t if_statements_count;
    size_t while_statements_count;
    size_t funcs_count;
    size_t cond_count;
    size_t prec_count;
    char*  tabs;
    unsigned precision_correction;
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

/**************************************************************************************************
 * @brief starting on root_node, translate AST to nasm text and write it to asm_text structure
 *
 * @param [root_node]   AST root
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @details this func sets up calculations stack (r15), includes standart library, calls main,
 *          describes after main call routine (exit). It also starts the translation of main itself.
 *
 * @return filled asm_text structure
**************************************************************************************************/
AsmText*       TranslateAST                 (const TreeNode* root_node, AsmText* asm_text, const NameTable* nametable);

/**************************************************************************************************
 * @brief translate AST subtree with root in node into nasm
 *
 * @param [node]        AST node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateASTSubtree          (const TreeNode* node, AsmText* asm_text, const NameTable* nametable);

/**************************************************************************************************
 * @brief translate declarator node subtree
 *
 * @param [declr_node]  AST declarator node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateDeclarator          (const TreeNode* declr_node, AsmText* asm_text, const NameTable* nametable);

/**************************************************************************************************
 * @brief translate keyword node subtree
 *
 * @param [kw_node]     AST keyword node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateKeyword             (const TreeNode* kw_node,    AsmText* asm_text, const NameTable* nametable);

/**************************************************************************************************
 * @brief translate separator node subtree
 *
 * @param [sep_node]    AST separator node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateSeparator           (const TreeNode* sep_node,   AsmText* asm_text, const NameTable* nametable);

/**************************************************************************************************
 * @brief translate operator node subtree
 *
 * @param [op_node]     AST operator node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateOperator            (const TreeNode* op_node,    AsmText* asm_text, const NameTable* nametable);


/**************************************************************************************************
 * @brief translate identifier node subtree
 *
 * @param [id_node]     AST identifier node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @details identifier is not only variable but also a function call
 *          the difference is established by nametable n_params field.
 *          The easiest way to tell apart is via IsFunction (...)
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateIdentifier          (const TreeNode* id_node,    AsmText* asm_text, const NameTable* nametable);


/**************************************************************************************************
 * @brief translate int_literal node subtree
 *
 * @param [num_node]    AST int_literal node
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateIntLiteral          (const TreeNode* num_node,   AsmText* asm_text, const NameTable* nametable);


/**************************************************************************************************
 * @brief translate function call
 *
 * @param [func_id_node]    AST function call node
 * @param [asm_text]        structure which stores assembler text
 * @param [nametable]       nametable
 *
 * @return TranslateRes (0 - on success)
**************************************************************************************************/
TranslateRes   TranslateFunctionCall        (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable);

AsmText*       AsmTextCtor                  ();
DefaultFuncRes AsmTextDtor                  (AsmText* asm_text);

DefaultFuncRes AsmTextAddTab                (AsmText* asm_text);
DefaultFuncRes AsmTextRemoveTab             (AsmText* asm_text);

/**************************************************************************************************
 * @brief based on data in nametable tell if id_node is a function name or a var/par name
 *
 * @param [id_node]     "suspected" function node or var/par node
 * @param [nametable]   nametable
 *
 * @return 1 - is function, 0 - not a function (is var/par)
**************************************************************************************************/
int            IsFunction                   (const TreeNode *id_node, const NameTable *nametable);


/**************************************************************************************************
 * @brief translate subtree in such a way that result of its execution marks if condition is
 *        is true or false
 *
 * @param [op_node]     operator node (usually comparison operators)
 * @param [asm_text]    structure which stores assembler text
 * @param [nametable]   nametable
 * @param [comparator]  asm conditional jump instruction relevant to the situation // todo
 *
 * @return 0 - on success
**************************************************************************************************/
DefaultFuncRes TranslateCondition           (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char *jmp_comparator);

OffsetTable*   OffsetTableCtor              ();
DefaultFuncRes OffsetTableDtor              (OffsetTable* offset_table);
int            OffsetTableGetCurrFrameWidth (OffsetTable* offset_table);
DefaultFuncRes OffsetTableAddFrame          (OffsetTable* offset_table);

/**************************************************************************************************
 * @brief get effective variable offset within its scope
 *
 * @param [offset_table] offset table
 * @param [var_id] id of variable/parameter to look for
 *
 * todo: here details section is not accurate
 * @details parameters go first, followed by local variables.
 *          Parameter eff_offset = index - n_params
 *          Loc.var.  eff_offset = index - n_params + 1 // to avoid addressing [rbp + 0]
 *
 * @return 0 - not found
**************************************************************************************************/
int            OffsetTableGetVarOffset      (OffsetTable* offset_table, size_t var_id);
DefaultFuncRes OffsetTableDeleteFrame       (OffsetTable* offset_table);
DefaultFuncRes OffsetTableAddVariable       (OffsetTable* offset_table, size_t var_id);
DefaultFuncRes OffsetTableAddFuncParams     (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable);
DefaultFuncRes OffsetTableAddFuncLocals     (OffsetTable* offset_table, const TreeNode* func_body_node, const NameTable* nametable);

/**************************************************************************************************
 *
**************************************************************************************************/
int            DescribeCurrFunction         (AsmText *asm_text, const NameTable *nametable);

#endif // TINKOV_BACKEND_H
