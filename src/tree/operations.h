#ifndef OPERATIONS_H
#define OPERATIONS_H

const int ILL_OPNUM = __INT_MAX__;

const char * const KEYWORDS[] =
{
    "я_ссыкло_или_я_не_ссыкло",
    "я_могу_ошибаться",
    "какая_разница",
    "ну_сколько_можно"
};
const int N_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

const char * const SEPARATORS[] =
{
    "сомнительно_но_окей",
    "олег_не_торопись", "я_олигарх_мне_заебись",
    "$",
    "?"
};
const int N_SEPARATORS = sizeof(SEPARATORS) / sizeof(SEPARATORS[0]);

const char * const OPERATORS[] =
{
    "я_так_чувствую",
    "+", "-", "*", "/", "^"
};
const int N_OPERATORS = sizeof(OPERATORS) / sizeof(OPERATORS[0]);

typedef enum
{
    EQUAL = 0,
    ADD  = 1,
    SUB  = 2,
    MUL  = 3,
    DIV  = 4,
    POW  = 5,
} Opcodes;

typedef enum
{
    ERROR       = 0, // none of other token types match, error (it is to be caught during syntax analysis) ?DO I NEED THIS?
    IDENTIFIER  = 1, // names assigned by the programmer
    KEYWORD     = 2, // reserved words of the language (while, for, if, return, int in C)
    SEPARATOR   = 3, // punctuation characters ({, (, ;, in C)
    OPERATOR    = 4, // symbols that operate on arguments and produce result (<, =, +, / in C)
    INT_LITERAL = 5, // integer decimal number
} NodeType;
typedef NodeType TokenType;

struct Operation
{
    Opcodes      opcode;
    const char*  name;
    NodeType     type; // binary or unary
    int          priority;
};

const Operation OPERATIONS[] =
{
    {EQUAL, "=", OPERATOR, 0},
    {ADD,   "+", OPERATOR, 1},
    {SUB,   "-", OPERATOR, 1},
    {MUL,   "*", OPERATOR, 2},
    {DIV,   "/", OPERATOR, 2},
    {POW,   "^", OPERATOR, 3},
};

const int OPERATIONS_NUM = sizeof(OPERATIONS) / sizeof(Operation);

#endif // OPERATIONS_H
