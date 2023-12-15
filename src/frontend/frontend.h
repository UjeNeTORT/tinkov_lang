/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#ifndef TINKOV_FRONTEND_H
#define TINKOV_FRONTEND_H

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../tree/tree.h"
#include "../tree/tree_dump/tree_dump.h"

// ===================== DSL =====================

#define OFFSET     (prog_code->offset)
#define TOKEN(i)   (prog_code->tokens[(i)])
#define CURR_TOKEN (prog_code->tokens[OFFSET])
#define TOKEN_IS(type, val) \
    (TYPE (CURR_TOKEN) == (type) && VAL (CURR_TOKEN) == (val))

#define TOKEN_IS_NOT(type, val) \
    (TYPE (CURR_TOKEN) != (type) || VAL (CURR_TOKEN) != (val))
// ===============================================

const int MAX_STRING_TOKEN = 300;

struct ProgText
{
    char* text;   // string with program code
    int   offset; // current position in code array
    int   len;    // code string length
};

struct ProgCode
{
    TreeNode** tokens;
    NameTable* nametable;
    int        offset;
    int        size;
};

// todo more
typedef enum
{
    LEX_SUCCESS = 0,
    LEX_ERROR   = 1,
} LexAnalysRes;

// syntax analysis
int     SyntaxAssert (int condition, ProgCode* prog_code, const char* format, ...);
#define SYNTAX_ASSERT(condition, format, ...) \
    SyntaxAssert((condition), prog_code, (format) __VA_OPT__(,) __VA_ARGS__)

Tree*     BuildAST            (ProgCode* prog_code);
TreeNode* GetG                (ProgCode* prog_code, Tree* tree);
TreeNode* GetWrappedStatement (ProgCode* prog_code, Tree* tree);
TreeNode* GetStatementBlock   (ProgCode* prog_code, Tree* tree);
TreeNode* GetSingleStatement  (ProgCode* prog_code, Tree* tree);
TreeNode* GetDoIf             (ProgCode* prog_code, Tree* tree);
TreeNode* GetAssign           (ProgCode* prog_code, Tree* tree);
TreeNode* GetRvalue           (ProgCode* prog_code, Tree* tree);
TreeNode* GetLvalue           (ProgCode* prog_code, Tree* tree);
TreeNode* GetMathExprRes      (ProgCode* prog_code, Tree* tree);
TreeNode* GetAddSubRes        (ProgCode* prog_code, Tree* tree);
TreeNode* GetMulDivRes        (ProgCode* prog_code, Tree* tree);
TreeNode* GetPowRes           (ProgCode* prog_code, Tree* tree);
TreeNode* GetOperand          (ProgCode* prog_code, Tree* tree);
TreeNode* GetSimpleOperand    (ProgCode* prog_code, Tree* tree);
TreeNode* GetIdentifier       (ProgCode* prog_code, Tree* tree);
TreeNode* GetNumber           (ProgCode* prog_code, Tree* tree);

// lexic analysis
LexAnalysRes LexicAnalysis           (ProgText* text);
ProgCode*    LexicalAnalysisTokenize (ProgText* text);

int IsIdentifier (const char* lexem);
int IsKeyword    (const char* lexem);
int IsSeparator  (const char* lexem);
int IsOperator   (const char* lexem);
int IsIntLiteral (const char* lexem);

int GetIdentifierIndex (const char* identifier, NameTable* nametable);
int GetKeywordIndex    (const char* keyword);
int GetSeparatorIndex  (const char* separator);
int GetOperatorIndex   (const char* operator_); // !!! operator__________ <- why??

// anscillary
ProgCode* ProgCodeCtor ();
int       ProgCodeDtor (ProgCode* prog_code);

ProgText* ProgTextCtor (const char* text, int text_len);
int       ProgTextDtor (ProgText* prog_text);

int StripLexem (char* lexem);

#endif // TINKOV_FRONTEND_H

