/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
 *************************************************************************/

#ifndef TINKOV_FRONTEND_H
#define TINKOV_FRONTEND_H

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"
#include "tree_dump.h"

// =========================== DSL ===========================

#define LEXER_ERR(err_text_format, ...) {ProgCodeDtor (prog_code); \
        RET_ERROR (NULL, err_text_format __VA_OPT__(,) __VA_ARGS__);}

#define OFFSET     (prog_code->offset)
#define TOKEN(i)   (prog_code->tokens[(i)])
#define CURR_TOKEN (prog_code->tokens[OFFSET])

#define ID_NAME(i) prog_code->nametable->names[VAL(i)]

#define OPEN_NEW_SCOPE SYNTAX_ASSERT (PushScope (sts) == 0, "Only %d nested scopes allowed, " \
                                    "think with your brain to reduce the number", MAX_SCOPE_DEPTH);

#define CLOSE_SCOPE SYNTAX_ASSERT (DelScope (sts) == 0, "Unexpected error while deleting scope");

#define DECLARE(token) \
    {                                                                   \
        SYNTAX_ASSERT (DeclareId (sts, VAL (token)) == 0, "Unexpected \"%s\" declaration error", ID_NAME (token)); \
    }

#define HAS_TOKENS_LEFT (OFFSET < prog_code->size)

#define TOKEN_IS(type, val) \
    (TYPE (CURR_TOKEN) == (type) && VAL (CURR_TOKEN) == (val))

#define TOKEN_IS_NOT(type, val) \
    (TYPE (CURR_TOKEN) != (type) || VAL (CURR_TOKEN) != (val))

#define SHOW_PROG_CODE for (int i = 0; i < prog_code->size; i++) printf ("%2d | t %d | v %5d |\n", i, TYPE(prog_code->tokens[i]), VAL(prog_code->tokens[i]));

// ===========================================================

const char RU_SYMBOLS[] =
    "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя";

char MAIN_FUNC_NAME[] = "ЦАРЬ";

char FOREIGN_AGENT_BANNER[] = "ДАННОЕ СООБЩЕНИЕ (МАТЕРИАЛ) СОЗДАНО И (ИЛИ) РАСПРОСТРАНЕНО ИНОСТРАННЫМ\n"
                              "И РОССИЙСКИМ ЮРИДИЧЕСКИМ ЛИЦОМ, ВЫПОЛНЯЮЩИМ ФУНКЦИИ ИНОСТРАННОГО КОМПИЛЯТОРА\n"
                              "А ТАКЖЕ ФИНАНСИРУЕТСЯ ИЗ ФОНДА КОШЕК ЕДИНИЧКИ И УПОМИНАЕТ НЕКОГО ИНОАГЕНТА\n"
                              "♂♂♂♂ Oleg ♂ TinCock ♂♂♂♂ (КТО БЫ ЭТО МОГ БЫТЬ). КОЛЯ ЛОХ КСТА, WHEN DANIL???\n"
                              "ДЛЯ ПОЛУЧЕНИЯ ВЫИГРЫША НАЖМИТЕ ALT+F4.\n";
const size_t FOREIGN_AGENT_BANNER_WORDS = 47; // i know i know

char NO_FOREIGN_AGENT_BANNER_ERROR[] =
                    RED("祝大家好运和积极!!! ОЙОЙОЙ! МНОГО ПЛОХО! МНОГО СТЫД! XI НЕДОВОЛЕН!\n"
                    "МАЛО") YELLOW(" РАДОСТЬ ") RED("МИН") YELLOW("ЮСТ") RED(" - МАЛО РИС! НЕ ПОЛУЧАТЬ КОШКА ЖЕНА!\n"
                    "ЗАБЫ") YELLOW("ТЬ ПЛАШКА ") RED("ИНОАГЕНТ - ЕХАТЬ УРАНОВЫЙ САНАТОРИЙ СИНЦЗЯН!\n"
                    "МНОГО") YELLOW(" ДУМАТЬ ") RED("ПОСЛЕ") YELLOW("ДСТ") RED("ВИЯ! МНОГО ЖАЛЕТЬ, ПЛАКАТЬ, МАЛО МОЗГ ГОЛОВА!\n"
                    "БЫТЬ РУССКИЙ ИВАН АВОСЬ! ГЛУПОСТЬ ДУРАКА! КУРИТЬ БАМБУК! ПЛОХО НЕХОРОШО\n"
                    "НЕГОДОВАТЬ! C") YELLOW("ЛАВ") RED("НЫЙ ВИННИ ПУХ КИТАЙ! УДАР БУРГЕР КАПИТАЛИЗМ!\n"
                    "ХОТЕТЬ КОШКА ЖЕНА, ПОЛУЧАТЬ КОШКА ЖЕНА! ХОТЕТЬ МНОГА РИС, ПОЛУЧАТЬ\n"
                    "10 РИС, 11 СОЦИАЛЬНЫЙ КРЕДИТ!!! УДАР! 荣耀中国\n");

const size_t MAX_LEXEM = 228;

struct ProgText
{
    char* text;         // string with program code
    int   offset;       // current position in text
    char* strings;      // string storing all the STR_LITERAL data
    int   str_offset;   // current position in strings
    int   str_index;    // index of free place for a string
};

struct ProgCode
{
    TreeNode** tokens;
    NameTable* nametable;
    int        offset;
    int        size;
};

struct ScopeTableStack
{
    int** is_declared;
    int size;
    int capacity;
};

// in main
ProgText*    GetProgText (const char* prog_name);
WriteTreeRes WriteAST    (const Tree* ast, const char* prog_name);

// syntax analysis
int SyntaxAssert (int line, int has_tokens_left, int condition, ProgCode* prog_code, const char* format, ...);
#define SYNTAX_ASSERT(condition, format, ...) \
    if (SyntaxAssert(__LINE__, HAS_TOKENS_LEFT, (condition), prog_code, (format) __VA_OPT__(,) __VA_ARGS__)) \
        assert (0);

// recursive descent
Tree*     BuildAST               (ProgCode* prog_code);
TreeNode* GetAST                 (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetFunctionDeclaration (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetCompoundStatement   (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetStatementBlock      (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetSingleStatement     (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetWhile               (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetIfElse              (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetDoIf                (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetAssign              (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetInput               (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetReturn              (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetPrint               (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetLvalue              (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetRvalue              (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetMathExprRes         (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetAddSubRes           (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetMulDivRes           (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetSqrtRes             (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetOperand             (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetSimpleOperand       (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetFunctionCall        (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetIdentifier          (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetParameter           (ProgCode* prog_code, ScopeTableStack* sts);
TreeNode* GetNumber              (ProgCode* prog_code, ScopeTableStack* sts);

// lexical analysis
ProgCode* LexerTokenize (ProgText* text);

int HasForeignAgent (ProgText* text);

int GetNewLineDistance (ProgText* text);
int GetStrLiteralLen   (ProgText* text);

int IsLexemMeanless (const char* lexem);
int IsLineComment   (const char* lexem);
int IsIdentifier    (const char* lexem);
int IsDeclarator    (const char* lexem);
int IsKeyword       (const char* lexem);
int IsSeparator     (const char* lexem);
int IsOperator      (const char* lexem);
int IsIntLiteral    (const char* lexem);
int IsStrLiteral    (const char* lexem);
int IsMainFunction  (const char* lexem);

int GetDeclaratorIndex (const char* keyword);
int GetKeywordIndex    (const char* keyword);
int GetSeparatorIndex  (const char* separator);
int GetOperatorIndex   (const char* operator_);

// anscillary
ScopeTableStack* ScopeTableStackCtor ();
int ScopeTableStackDtor (ScopeTableStack* sts);

int PushScope    (ScopeTableStack* sts);
int DelScope     (ScopeTableStack* sts);
int IsIdDeclared (const ScopeTableStack* sts, const int identifier_index);
int DeclareId    (ScopeTableStack* sts, const int identifier_index);

ProgCode* ProgCodeCtor ();
int       ProgCodeDtor (ProgCode* prog_code);

ProgText* ProgTextCtor (const char* text, int text_len);
int       ProgTextDtor (ProgText* prog_text);

size_t GetProgSize (FILE* prog_file);

int StripLexem (char* lexem);

int CheckVarsDeclared (TreeNode* node, ScopeTableStack* sts);

#endif // TINKOV_FRONTEND_H

