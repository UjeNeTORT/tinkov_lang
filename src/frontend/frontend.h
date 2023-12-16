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

// =========================== DSL ===========================

#define OFFSET     (prog_code->offset)
#define TOKEN(i)   (prog_code->tokens[(i)])
#define CURR_TOKEN (prog_code->tokens[OFFSET])

#define NO_MORE_TOKENS (prog_code->size <= OFFSET)

#define TOKEN_IS(type, val) \
    (TYPE (CURR_TOKEN) == (type) && VAL (CURR_TOKEN) == (val))

#define TOKEN_IS_NOT(type, val) \
    (TYPE (CURR_TOKEN) != (type) || VAL (CURR_TOKEN) != (val))
// ===========================================================

char FOREIGN_AGENT_BANNER[] = "ДАННОЕ СООБЩЕНИЕ (МАТЕРИАЛ) СОЗДАНО И (ИЛИ) РАСПРОСТРАНЕНО ИНОСТРАННЫМ\n"
                              "И РОССИЙСКИМ ЮРИДИЧЕСКИМ ЛИЦОМ, ВЫПОЛНЯЮЩИМ ФУНКЦИИ ИНОСТРАННОГО КОМПИЛЯТОРА\n"
                              "А ТАКЖЕ ФИНАНСИРУЕТСЯ ИЗ ФОНДА КОШЕК ЕДИНИЧКИ И УПОМИНАЕТ НЕКОГО ИНОАГЕНТА\n"
                              "♂♂♂♂ Oleg ♂ TinCock ♂♂♂♂ (КТО БЫ ЭТО МОГ БЫТЬ). КОЛЯ ЛОХ КСТА, WHEN DANIL???\n"
                              "ДЛЯ ПОЛУЧЕНИЯ ВЫИГРЫША НАЖМИТЕ ALT+F4.\n";
const int  FOREIGN_AGENT_BANNER_WORDS = 47; // i know i know

char NO_FOREIGN_AGENT_BANNER_ERROR[] =
                    RED("祝大家好运和积极!!! ОЙОЙОЙ! МНОГО ПЛОХО! МНОГО СТЫД! XI НЕДОВОЛЕН!\n"
                    "МАЛО") YELLOW(" РАДОСТЬ ") RED("МИН") YELLOW("ЮСТ") RED(" - МАЛО РИС! НЕ ПОЛУЧАТЬ КОШКА ЖЕНА!\n"
                    "ЗАБЫ") YELLOW("ТЬ ПЛАШКА ") RED("ИНОАГЕНТ - ЕХАТЬ УРАНОВЫЙ САНАТОРИЙ СИНЦЗЯН!\n"
                    "МНОГО") YELLOW(" ДУМАТЬ ") RED("ПОСЛЕ") YELLOW("ДСТ") RED("ВИЯ! МНОГО ЖАЛЕТЬ, ПЛАКАТЬ, МАЛО МОЗГ ГОЛОВА!\n"
                    "БЫТЬ РУССКИЙ ИВАН АВОСЬ! ГЛУПОСТЬ ДУРАКА! КУРИТЬ БАМБУК! ПЛОХО НЕХОРОШО\n"
                    "НЕГОДОВАТЬ! C") YELLOW("ЛАВ") RED("НЫЙ ВИННИ ПУХ КИТАЙ! УДАР БУРГЕР КАПИТАЛИЗМ!\n"
                    "ХОТЕТЬ КОШКА ЖЕНА, ПОЛУЧАТЬ КОШКА ЖЕНА! ХОТЕТЬ МНОГА РИС, ПОЛУЧАТЬ\n"
                    "10 РИС, 11 СОЦИАЛЬНЫЙ КРЕДИТ!!! УДАР! 荣耀中国\n");

const int MAX_LEXEM = 228;

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
    if (SyntaxAssert((condition), prog_code, (format) __VA_OPT__(,) __VA_ARGS__)) \
        assert (0);

Tree*     BuildAST             (ProgCode* prog_code);
TreeNode* GetG                 (ProgCode* prog_code);
TreeNode* GetCompoundStatement (ProgCode* prog_code);
TreeNode* GetStatementBlock    (ProgCode* prog_code);
TreeNode* GetSingleStatement   (ProgCode* prog_code);
TreeNode* GetWhile             (ProgCode* prog_code);
TreeNode* GetIfElse            (ProgCode* prog_code);
TreeNode* GetDoIf              (ProgCode* prog_code);
TreeNode* GetAssign            (ProgCode* prog_code);
TreeNode* GetRvalue            (ProgCode* prog_code);
TreeNode* GetLvalue            (ProgCode* prog_code);
TreeNode* GetMathExprRes       (ProgCode* prog_code);
TreeNode* GetAddSubRes         (ProgCode* prog_code);
TreeNode* GetMulDivRes         (ProgCode* prog_code);
TreeNode* GetPowRes            (ProgCode* prog_code);
TreeNode* GetOperand           (ProgCode* prog_code);
TreeNode* GetSimpleOperand     (ProgCode* prog_code);
TreeNode* GetIdentifier        (ProgCode* prog_code);
TreeNode* GetNumber            (ProgCode* prog_code);

// lexic analysis
LexAnalysRes LexicAnalysis           (ProgText* text);
ProgCode*    LexicalAnalysisTokenize (ProgText* text);

int HasForeignAgent (ProgText* text);

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

