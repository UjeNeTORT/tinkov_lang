#ifndef TINKOV_FRONTEND_H
#define TINKOV_FRONTEND_H

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../tree/tree.h"

const int MAX_TOKEN = 30;

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

TreeNode* GetNumber      (ProgCode* code, Tree* tree);
TreeNode* GetVar         (ProgCode* code, Tree* tree);
TreeNode* GetOperand     (ProgCode* code, Tree* tree);
TreeNode* GetParenthesis (ProgCode* code, Tree* tree);
TreeNode* GetMulDiv      (ProgCode* code, Tree* tree);
TreeNode* GetAddSub      (ProgCode* code, Tree* tree);
TreeNode* GetMathExpr    (ProgCode* code, Tree* tree);

// lexic analysis
LexAnalysRes LexicAnalysis           (ProgText* text);
ProgCode*    LexicalAnalysisTokenize (ProgText* text);

int IsIdentifier (const char* word);
int IsKeyword    (const char* word);
int IsSeparator  (const char* word);
int IsOperator   (const char* word);
int IsIntLiteral (const char* word);

int GetIdentifierIndex (const char* identifier, NameTable* nametable);
int GetKeywordIndex    (const char* keyword);
int GetSeparatorIndex  (const char* separator);
int GetOperatorIndex   (const char* operator_); // !!! operator__________ <- why??

// anscillary
ProgCode* ProgCodeCtor ();
int       ProgCodeDtor (ProgCode* prog_code);

#endif // TINKOV_FRONTEND_H

