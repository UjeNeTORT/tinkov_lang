/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/Differentiator
 *************************************************************************/

#ifndef TINKOV_FRONTEND_H
#define TINKOV_FRONTEND_H

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../tree/tree.h"
#include "../tree/tree_dump/tree_dump.h"

// ===================== DSL =====================

#define OFFSET     (prog_code->offset)
#define TOKEN(i)   (prog_code->tokens[(i)])
#define CURR_TOKEN (prog_code->tokens[OFFSET])

// ===============================================

const int MAX_STRING_TOKEN = 30;

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

Tree* ParseAST (ProgCode code);

// syntax analysis
int SyntaxAssert (int condition);

Tree*     BuildAST         (ProgCode* prog_code);
TreeNode* GetG             (ProgCode* prog_code, Tree* tree);
TreeNode* GetAssign        (ProgCode* prog_code, Tree* tree);
TreeNode* GetRvalue        (ProgCode* prog_code, Tree* tree);
TreeNode* GetLvalue        (ProgCode* prog_code, Tree* tree);
TreeNode* GetMathExpr      (ProgCode* prog_code, Tree* tree);
TreeNode* GetMulDivRes     (ProgCode* prog_code, Tree* tree);
TreeNode* GetPowRes        (ProgCode* prog_code, Tree* tree);
TreeNode* GetOperand       (ProgCode* prog_code, Tree* tree);
TreeNode* GetSimpleOperand (ProgCode* prog_code, Tree* tree);
TreeNode* GetIdentifier    (ProgCode* prog_code, Tree* tree);
TreeNode* GetNumber        (ProgCode* prog_code, Tree* tree);

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

