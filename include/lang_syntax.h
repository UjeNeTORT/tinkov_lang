/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#ifndef LANG_SYNTAX_H
#define LANG_SYNTAX_H

const int ILL_OPNUM = __INT_MAX__;
const size_t MAX_SCOPE_DEPTH = 10; // naming?

const char LINE_COMMENT_NAME[] = "сарказм";
const char STRING_LITERAL_TERMINAL_SYMBOL = '"';

const char * const MEANLESS_LEXEMS[] =
{
    "бы",
    "был",
    "великий",
    "величайший",
    "говорил",
    "да",
    "даже",
    "давно",
    "долго",
    "и",
    "из",
    "или",
    "император",
    "капитальный",
    "какой-то",
    "надо",
    "не",
    "нет",
    "никому",
    "но",
    "новый",
    "ну",
    "полный",
    "пиздец",
    "прихерел",
    "самый",
    "совсем",
    "сказал",
    "тотальный",
    "тоже",
    "уже",
    "хотя",
    "это",
    "этот",
};
const int N_MEANLESS_LEXEMS = sizeof (MEANLESS_LEXEMS) / sizeof (MEANLESS_LEXEMS[0]);

typedef enum
{
    FUNC_DECLARATOR = 0, // "россии_нужен"
    VAR_DECLARATOR  = 1, // "грешник"
} DeclaratorCode;

struct Declarator
{
    const char* const name;
    DeclaratorCode dclr_code;
};

const Declarator DECLARATORS[] =
{
    {"россии_нужен", FUNC_DECLARATOR},
    {"грешник",      VAR_DECLARATOR},
};
const int N_DECLARATORS = sizeof (DECLARATORS) / sizeof (DECLARATORS[0]);

typedef enum
{
    KW_DO     = 0, // "я_ссыкло_или_я_не_ссыкло"
    KW_IF     = 1, // "какая_разница"
    KW_ELSE   = 2, // "я_могу_ошибаться"
    KW_WHILE  = 3, // "ну_сколько_можно"
    KW_RETURN = 4, // "никто_никогда_не_вернет"
    KW_INPUT  = 5, // "мне_надо"
    KW_PRINT  = 6, // "там_кто_то_мне_пишет"
} KeywordCode;

struct Keyword
{
    const char* const name;
    KeywordCode    kw_code;
};

const Keyword KEYWORDS[] =
{
    {"я_ссыкло_или_я_не_ссыкло", KW_DO},
    {"какая_разница",            KW_IF},
    {"я_могу_ошибаться",         KW_ELSE},
    {"ну_сколько_можно",         KW_WHILE},
    {"никто_никогда_не_вернет",  KW_RETURN},
    {"мне_надо",                 KW_INPUT},
    {"там_кто_то_мне_пишет",     KW_PRINT},
};
const int N_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

typedef enum
{
    END_STATEMENT           = 0, // "сомнительно_но_окей"
    ENCLOSE_STATEMENT_BEGIN = 1, // "олег_не_торопись"
    ENCLOSE_STATEMENT_END   = 2, // "я_олигарх_мне_заебись"
    ENCLOSE_MATH_EXPR_L     = 3, // "("
    ENCLOSE_MATH_EXPR_R     = 4, // ")"
    END_CONDITION           = 5, // "?"
    BEGIN_FUNC_PARAMS       = 6, // "за"
    END_FUNC_PARAMS         = 7, // "почти_без_переплат"
} SeparatorCode;

struct Separator
{
    const char* const name;
    SeparatorCode sep_code;
};

const Separator SEPARATORS[] =
{
    {"сомнительно_но_окей",   END_STATEMENT},
    {"олег_не_торопись",      ENCLOSE_STATEMENT_BEGIN},
    {"я_олигарх_мне_заебись", ENCLOSE_STATEMENT_END},
    {"(",                     ENCLOSE_MATH_EXPR_L},
    {")",                     ENCLOSE_MATH_EXPR_R},
    {"?",                     END_CONDITION},
    {"за",                    BEGIN_FUNC_PARAMS},
    {"почти_без_переплат",    END_FUNC_PARAMS},
};
const int N_SEPARATORS = sizeof (SEPARATORS) / sizeof (SEPARATORS[0]);

typedef enum
{
    ASSIGN  = 0,  // "я_так_чувствую"
    ADD     = 1,  // "+"
    SUB     = 2,  // "-"
    MUL     = 3,  // "*"
    DIV     = 4,  // "/"
    SQRT    = 5,  // "корень"
    LESS    = 6,  // "<"
    LESS_EQ = 7,  // "<="
    EQUAL   = 8,  // "=="
    MORE_EQ = 9,  // ">="
    MORE    = 10, // ">"
    UNEQUAL = 11, // "!="
} OperatorCode;

struct Operator
{
    OperatorCode       op_code;
    const char* const  name;
};

const Operator OPERATORS[] =
{
    {ASSIGN, "я_так_чувствую"},
    {ADD,    "+"},
    {SUB,    "-"},
    {MUL,    "*"},
    {DIV,    "/"},
    {SQRT,    "корень"},

    {LESS,    "<"},
    {LESS_EQ, "<="},
    {EQUAL,   "=="},
    {MORE_EQ, ">="},
    {MORE,    ">"},
    {UNEQUAL, "!="},
};
const int N_OPERATORS = sizeof (OPERATORS) / sizeof (OPERATORS[0]);

typedef enum
{
    ERROR       = 0, // none of other token types match, error (it is to be caught during syntax analysis) ?DO I NEED THIS?
    IDENTIFIER  = 1, // names assigned by the programmer
    DECLARATOR  = 2, //
    KEYWORD     = 3, // reserved words of the language (while, for, if, return, int in C)
    SEPARATOR   = 4, // punctuation characters ({, (, ;, in C)
    OPERATOR    = 5, // symbols that operate on arguments and produce result (<, =, +, / in C)
    INT_LITERAL = 6, // integer decimal number
    STR_LITERAL = 7, // null-terminated sequence of chars
} NodeType;
typedef NodeType TokenType;

#endif // LANG_SYNTAX_H
