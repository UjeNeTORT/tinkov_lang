/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

/**
 * BUGS: lexer thinks that "131aboba" is a number 131
 *       lexer thinks that "_aboba228_ is unknown lexem"
 *       lexer does not take \n as a space between tokens
 *       syntaxer does not give an error if there is no ; in the end
 *       difference between index of keyword and its opcode is not always trivial
 *
 * TODO: fix bugs
 *       syntax_assert instead of return NULL
*/

#include "frontend.h"

int main()
{
    const char* code_math =
                        "x я_так_чувствую aboba228 ^ 11 сомнительно_но_окей";

    const char* code_double_assign =
                       "олег_не_торопись x я_так_чувствую 11 сомнительно_но_окей "
                       "y я_так_чувствую 12 сомнительно_но_окей я_олигарх_мне_заебись";

    const char* doif_code =
                        "я_ссыкло_или_я_не_ссыкло "
                        "x я_так_чувствую 11 сомнительно_но_окей "
                        "какая_разница aboba228 > 11 ?";

    const char* while_code =
                        "ну_сколько_можно x > 11 ^ aboba228 ? "
                        "x я_так_чувствую x + 1 сомнительно_но_окей ";

    const char* if_else_code =
                        "какая_разница aboba_18 > 666 / 2 ? "
                            "x я_так_чувствую 333 + 0 сомнительно_но_окей "
                        "я_могу_ошибаться "
                            "x я_так_чувствую 11 сомнительно_но_окей ";

    ProgText* prog_text = ProgTextCtor (if_else_code, strlen(if_else_code) + 1);
    ProgCode* prog_code = LexicalAnalysisTokenize (prog_text);
    ProgTextDtor (prog_text);

    Tree* ast = BuildAST (prog_code);

    TreeDotDump ("dump.html", ast);

    ProgCodeDtor (prog_code);
    TreeDtor (ast);

    PRINTF_DEBUG ("done");

    return 0;
}

// ============================================================================================

Tree* BuildAST (ProgCode* prog_code)
{
    assert (prog_code);

    Tree* ast = TreeCtor();
    NameTableCopy (ast->nametable, prog_code->nametable);
    ast->root = GetG (prog_code);

    return ast;
}

// ============================================================================================

TreeNode* GetG (ProgCode* prog_code)
{
    assert (prog_code);

    TreeNode* wrapped_statement_res = GetWrappedStatement (prog_code);
    if (!wrapped_statement_res)
    {
        WARN ("assign res nil");

        return NULL; // error or not?
    }

    return wrapped_statement_res;
}

// ============================================================================================

TreeNode* GetWrappedStatement (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* wrapped_statement = GetStatementBlock (prog_code);

    if (wrapped_statement)
        return wrapped_statement;

    OFFSET = init_offset;

    wrapped_statement = GetSingleStatement (prog_code);

    return wrapped_statement;
}

// ============================================================================================

TreeNode* GetStatementBlock (ProgCode* prog_code)
{
    assert (prog_code);

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_STATEMENT_BEGIN))
        return NULL;

    OFFSET++; // skip {

    TreeNode* new_statement = NULL;

    TreeNode* statement_block = NULL;

    do
    {
        new_statement = GetSingleStatement (prog_code);

        if (new_statement)
            statement_block = TreeNodeCtor (END_STATEMENT, SEPARATOR, NULL, statement_block, NULL, new_statement);
    } while (new_statement);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_STATEMENT_END),
                   "Missing enclose statement bracket \"я_олигарх_мне_заебись\"");

    OFFSET++; // skip }

    return statement_block;
}

// ============================================================================================

TreeNode* GetSingleStatement (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* single_statement = NULL;

    single_statement = GetIfElse (prog_code);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetWhile (prog_code);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetDoIf (prog_code);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetAssign (prog_code);
    if (!single_statement)
        return NULL; // as the last one

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_STATEMENT),
                   "\"сомнительно_но_окей\" expected in the end of statement");

    OFFSET++; // skip ";"

    return single_statement;
}

// ============================================================================================

TreeNode* GetWhile (ProgCode* prog_code)
{
    assert (prog_code);

    if (TOKEN_IS_NOT (KEYWORD, KW_WHILE))
        return NULL;

    OFFSET++; // skip "while"

    TreeNode* condition = GetMathExprRes (prog_code);
    SYNTAX_ASSERT(condition != NULL, "condition error");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "\"?\" expected in the end of condition");
    OFFSET++; // skip "?"

    TreeNode* wrapped_statement = GetWrappedStatement (prog_code);
    SYNTAX_ASSERT (wrapped_statement != NULL, "No wrapped statement given in while");

    return TreeNodeCtor (KW_WHILE, KEYWORD, NULL, wrapped_statement, condition, NULL);
}

// ============================================================================================

TreeNode* GetIfElse (ProgCode* prog_code)
{
    assert (prog_code);

    if (TOKEN_IS_NOT (KEYWORD, KW_IF))
        return NULL;

    OFFSET++; // skip "if"

    TreeNode* condition = GetMathExprRes (prog_code);
    SYNTAX_ASSERT (condition != NULL, "condition error");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "\"?\" expected in the end of condition");
    OFFSET++; // skip "?"

    TreeNode* if_statement = GetWrappedStatement (prog_code);
    SYNTAX_ASSERT (if_statement != NULL, "No wrapped statement given in while");

    if (TOKEN_IS_NOT (KEYWORD, KW_ELSE))
        return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_statement, condition, NULL);

    OFFSET++; // skip "else"

    TreeNode* else_statement = GetWrappedStatement (prog_code);
    SYNTAX_ASSERT (else_statement != NULL, "\"else\" statement expected");

    return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_statement, condition, else_statement);
}

// ============================================================================================

TreeNode* GetDoIf (ProgCode* prog_code)
{
    assert (prog_code);

    if (TOKEN_IS_NOT (KEYWORD, KW_DO))
        return NULL;

    OFFSET++; // skip "do"

    TreeNode* wrapped_statement = GetWrappedStatement (prog_code);
    SYNTAX_ASSERT (wrapped_statement != NULL, "No statement inside do-if given");

    SYNTAX_ASSERT (TOKEN_IS (KEYWORD, KW_IF), "Keyword \"какая_разница\" expected");

    OFFSET++; // skip "if"

    TreeNode* condition = GetMathExprRes (prog_code);
    SYNTAX_ASSERT (condition != NULL, "Condition expected");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "Separator \"?\" expected");

    OFFSET++; // skip "?"

    return TreeNodeCtor (KW_DO, KEYWORD, NULL, wrapped_statement, condition, NULL);
}

// ============================================================================================

TreeNode* GetAssign (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset  = OFFSET;
    TreeNode* lvalue = GetLvalue (prog_code);
    if (!lvalue)
    {
        WARN ("lvalue nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!TOKEN_IS (OPERATOR, ASSIGN))
    {
        OFFSET = init_offset;

        WARN ("no assign operator");

        return NULL;
    }

    OFFSET++; // skip "=" operator

    TreeNode* rvalue = GetRvalue (prog_code);
    SYNTAX_ASSERT (rvalue != NULL, "Rvalue (nil) error");

    return TreeNodeCtor (ASSIGN, OPERATOR, NULL, lvalue, NULL, rvalue);
}

// ============================================================================================

TreeNode* GetRvalue (ProgCode* prog_code)
{
    assert (prog_code);

    return GetMathExprRes (prog_code);
}

// ============================================================================================

TreeNode* GetLvalue (ProgCode* prog_code)
{
    assert (prog_code);

    return GetIdentifier (prog_code);
}

// ============================================================================================

TreeNode* GetMathExprRes (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* math_expr_res = GetAddSubRes (prog_code);
    if (!math_expr_res)
    {
        WARN ("add_sub_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (TOKEN_IS_NOT (OPERATOR, LESS)     &&
        TOKEN_IS_NOT (OPERATOR, LESS_EQ)  &&
        TOKEN_IS_NOT (OPERATOR, EQUAL)    &&
        TOKEN_IS_NOT (OPERATOR, MORE_EQ)  &&
        TOKEN_IS_NOT (OPERATOR, MORE)     &&
        TOKEN_IS_NOT (OPERATOR, UNEQUAL))
        return math_expr_res;

    int op_cmp = VAL (CURR_TOKEN);

    OFFSET++; // skip operator

    TreeNode* curr_add_sub_res = GetAddSubRes (prog_code);
    SYNTAX_ASSERT (curr_add_sub_res != NULL, "x >= <error> - nil after comparison operator");

    switch (op_cmp)
    {
    case LESS:
        math_expr_res = TreeNodeCtor (LESS, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    case LESS_EQ:
        math_expr_res = TreeNodeCtor (LESS_EQ, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    case EQUAL:
        math_expr_res = TreeNodeCtor (EQUAL, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    case MORE_EQ:
        math_expr_res = TreeNodeCtor (MORE_EQ, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    case MORE:
        math_expr_res = TreeNodeCtor (MORE, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    case UNEQUAL:
        math_expr_res = TreeNodeCtor (UNEQUAL, OPERATOR, NULL, math_expr_res, NULL, curr_add_sub_res);
        break;

    default:
        SYNTAX_ASSERT (0, "comparison operator expected");
        break;
    }

    return math_expr_res;
}

// ============================================================================================

TreeNode* GetAddSubRes (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* add_sub_res = GetMulDivRes (prog_code);
    if (!add_sub_res)
    {
        WARN ("mul_div_res_1 nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (TOKEN_IS_NOT (OPERATOR, ADD) &&
        TOKEN_IS_NOT (OPERATOR, SUB))
        return add_sub_res;

    while (TOKEN_IS (OPERATOR, ADD) ||
           TOKEN_IS (OPERATOR, SUB))
    {
        int op_add_sub = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_mul_div_res = GetMulDivRes (prog_code);
        if (!curr_mul_div_res)
        {
            SubtreeDtor (add_sub_res);
            SYNTAX_ASSERT (0, "x + <error> - nil after add/sub operator");
        }

        switch (op_add_sub)
        {
        case ADD:
            add_sub_res = TreeNodeCtor (ADD, OPERATOR, NULL, add_sub_res, NULL, curr_mul_div_res);
            break;

        case SUB:
            add_sub_res = TreeNodeCtor (SUB, OPERATOR, NULL, add_sub_res, NULL, curr_mul_div_res);
            break;

        default:
            RET_ERROR (NULL, "Syntax error, add or sub operator expected"); // todo
            break;
        }
    }

    return add_sub_res;
}

// ============================================================================================

TreeNode* GetMulDivRes (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* mul_div_res = GetPowRes (prog_code);
    if (!mul_div_res)
    {
        WARN ("mul_div_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (TOKEN_IS_NOT (OPERATOR, MUL) &&
        TOKEN_IS_NOT (OPERATOR, DIV))
        return mul_div_res;

    while (TOKEN_IS (OPERATOR, MUL) ||
           TOKEN_IS (OPERATOR, DIV))
    {
        int op_mul_div = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_pow_res = GetPowRes (prog_code);
        if (!curr_pow_res)
        {
            SubtreeDtor (mul_div_res);
            SYNTAX_ASSERT (0, "x / <error> - nil after mul/div operator");
        }

        switch (op_mul_div)
        {
        case MUL:
            mul_div_res = TreeNodeCtor (MUL, OPERATOR, NULL, mul_div_res, NULL, curr_pow_res);
            break;

        case DIV:
            mul_div_res = TreeNodeCtor (DIV, OPERATOR, NULL, mul_div_res, NULL, curr_pow_res);
            break;

        default:

            RET_ERROR (NULL, "Syntax error, mul or div operator expected"); // todo
            break;
        }
    }

    return mul_div_res;
}

// ============================================================================================

TreeNode* GetPowRes (ProgCode* prog_code)
{
    assert (prog_code);

    int init_offset = OFFSET;

    TreeNode* pow_res = GetOperand (prog_code);
    if (!pow_res)
    {
        WARN ("pow_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (TOKEN_IS_NOT (OPERATOR, POW))
        return pow_res;

    OFFSET++; // skip ^

    TreeNode* operand_2 = GetOperand (prog_code);
    SYNTAX_ASSERT (operand_2 != NULL, "in power right operand nil");

    pow_res = TreeNodeCtor (POW, OPERATOR, NULL, pow_res, NULL, operand_2);

    return pow_res;
}

// ============================================================================================

TreeNode* GetOperand (ProgCode* prog_code)
{
    assert (prog_code);

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_MATH_EXPR))
        return GetSimpleOperand (prog_code);

    OFFSET++; // skip $

    TreeNode* math_expr = GetMathExprRes (prog_code);
    SYNTAX_ASSERT (math_expr != NULL, "error inside brackets");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR), "Missing separator \"$\"");

    OFFSET++; // skip $

    return math_expr;
}

// ============================================================================================

TreeNode* GetSimpleOperand (ProgCode* prog_code)
{
    assert (prog_code);

    TreeNode* ret_val = GetIdentifier (prog_code);

    if (!ret_val)
        return GetNumber (prog_code);

    return ret_val;
}

// ============================================================================================

TreeNode* GetIdentifier (ProgCode* prog_code)
{
    assert (prog_code);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL, NULL);

    OFFSET++; // skip identifier

    return ret_val;
}

// ============================================================================================

TreeNode* GetNumber (ProgCode* prog_code)
{
    assert (prog_code);

    if (TYPE (CURR_TOKEN) != INT_LITERAL)
        return NULL;

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL, NULL);

    OFFSET++; // skip number

    return ret_val;
}

// ============================================================================================

// func too big
ProgCode* LexicalAnalysisTokenize (ProgText* text)
{
    assert (text);

    ProgCode* prog_code = ProgCodeCtor ();

    int n_readen = 0;

    char lexem[MAX_STRING_TOKEN] = "";

    while (sscanf (text->text + text->offset, "%s%n", lexem, &n_readen) != EOF)
    {
        StripLexem (lexem);

        text->offset += n_readen;

        TreeNode* new_node = NULL;

        // the whole statement is quite unoptimal because many functions duplicate each other
        if (IsIdentifier (lexem))
        {
            int id_index = GetIdentifierIndex (lexem, prog_code->nametable);
            if (id_index == -1)
            {
                ProgCodeDtor (prog_code);

                RET_ERROR (NULL, "Unexpected error: \"%s\" identifier index = -1", lexem);
            }

            new_node = TreeNodeCtor (id_index, IDENTIFIER, NULL, NULL, NULL, NULL);
        }

        else if (IsKeyword (lexem)) // unoptimal, requires 2 cycles
        {
            int kw_index = GetKeywordIndex (lexem);
            if (kw_index == -1)
            {
                ProgCodeDtor (prog_code);

                RET_ERROR (NULL, "Unexpected error: keyword \"%s\" "
                                 "index not found in keywords table", lexem);
            }

            new_node = TreeNodeCtor (kw_index, KEYWORD, NULL, NULL, NULL, NULL);
        }

        else if (IsSeparator (lexem))
        {
            int sep_index = GetSeparatorIndex (lexem);
            if (sep_index == -1)
            {
                ProgCodeDtor (prog_code);

                RET_ERROR (NULL, "Unexpected error: separator \"%s\" "
                                 "index not found in separators table", lexem);
            }

            new_node = TreeNodeCtor (sep_index, SEPARATOR, NULL, NULL, NULL, NULL);
        }

        else if (IsOperator (lexem))
        {
            int op_index = GetOperatorIndex (lexem);
            if (op_index == -1)
            {
                ProgCodeDtor (prog_code);

                RET_ERROR (NULL, "Unexpected error: operator \"%s\" "
                                 "index not found in operators table", lexem);
            }

            new_node = TreeNodeCtor (op_index, OPERATOR, NULL, NULL, NULL, NULL);
        }

        else if (IsIntLiteral (lexem))
        {
            new_node = TreeNodeCtor (atoi (lexem), INT_LITERAL, NULL, NULL, NULL, NULL);
        }

        else
        {
            ProgCodeDtor (prog_code);
            RET_ERROR (NULL, "Unknown lexem \"%s\"", lexem);
        }

        prog_code->tokens[prog_code->size++] = new_node;
    }

    return prog_code;
}

// ============================================================================================

int IsIdentifier (const char* lexem)
{
    assert (lexem);

    if (!isalpha(*lexem)) return 0;

    while (*++lexem)
        if (!isalnum(*lexem) && *lexem != '_') return 0;

    return 1;
}

// ============================================================================================

int IsKeyword (const char* lexem)
{
    assert (lexem);

    for (int i = 0; i < N_KEYWORDS; i++)
    {
        if (streq (lexem, KEYWORDS[i].name))
            return 1;
    }

    return 0;
}

// ============================================================================================

int IsSeparator  (const char* lexem)
{
    assert (lexem);

    for (int i = 0; i < N_SEPARATORS; i++)
    {
        if (streq (lexem, SEPARATORS[i].name))
            return 1;
    }

    return 0;
}

// ============================================================================================

int IsOperator (const char* lexem)
{
    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (lexem, OPERATORS[i].name))
            return 1;
    }

    return 0;
}

// ============================================================================================

int IsIntLiteral (const char* lexem)
{
    assert (lexem);

    if (atoi (lexem) != 0)
        return 1;

    if (atoi (lexem) == 0 && lexem[0] == '0') // temporary, does not cover many cases
        return 1;

    return 0;
}

// ============================================================================================

int GetIdentifierIndex (const char* identifier, NameTable* nametable)
{
    assert (identifier);
    assert (nametable);

    int id_index = FindVarInNametable (identifier, nametable);
    if (id_index  != -1)
        return id_index;

    return UpdNameTable (identifier, nametable);
}

// ============================================================================================

int GetKeywordIndex (const char* keyword)
{
    assert (keyword);

    for (int i = 0; i < N_KEYWORDS; i++)
    {
        if (streq (keyword, KEYWORDS[i].name))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

// ============================================================================================

int GetSeparatorIndex  (const char* separator)
{
    assert (separator);

    for (int i = 0; i < N_SEPARATORS; i++)
    {
        if (streq (separator, SEPARATORS[i].name))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

// ============================================================================================

int GetOperatorIndex (const char* operator_)
{
    assert (operator_);

    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (operator_, OPERATORS[i].name))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

// ============================================================================================

ProgCode* ProgCodeCtor ()
{
    ProgCode* prog_code = (ProgCode*) calloc (1, sizeof(ProgCode));

    prog_code->nametable = NameTableCtor ();

    prog_code->tokens = (TreeNode**) calloc (MAX_N_NODES, sizeof(TreeNode*));
    prog_code->size   = 0;
    OFFSET = 0;

    return prog_code;
}

// ============================================================================================

int ProgCodeDtor (ProgCode* prog_code)
{
    assert (prog_code);

    NameTableDtor (prog_code->nametable);

    for (int i = 0; prog_code->tokens[i] && i < prog_code->size; i++)
        free (prog_code->tokens[i]);

    free (prog_code->tokens);
    free (prog_code);

    return 0;
}

// ============================================================================================


ProgText* ProgTextCtor (const char* text, int text_len)
{
    assert (text);

    char* text_copy = (char*) calloc (text_len, sizeof (char));

    strcpy (text_copy, text);

    ProgText* prog_text = (ProgText*) calloc (1, sizeof(ProgText));

    prog_text->text   = text_copy;
    prog_text->offset = 0;
    prog_text->len    = text_len;

    return prog_text;
}

// ============================================================================================

int ProgTextDtor (ProgText* prog_text)
{
    assert (prog_text);

    prog_text->offset = -1;
    prog_text->len    = -1;

    free (prog_text->text);

    free (prog_text);

    return 0;
}

// ============================================================================================

int StripLexem (char* lexem)
{
    assert (lexem);

    lexem[strcspn (lexem, "\t\r\n ")] = 0;

    return 0;
}

// ============================================================================================

int SyntaxAssert (int condition, ProgCode* prog_code, const char* format, ...)
{
    // assert (prog_code);

    if (!condition)
    {
        fprintf (stderr, RED ("In token (TYPE = %d, VAL = %d) OFSSET = %d\n"),
                                         TYPE (CURR_TOKEN), VAL (CURR_TOKEN), OFFSET);
        fprintf (stderr, RED ("SYNTAX ERROR! "));

        va_list  (ptr);
        va_start (ptr, format);

        vfprintf (stderr, format, ptr);

        va_end (ptr);

        fprintf (stderr, RST_CLR "\n" RST_CLR);

        ProgCodeDtor (prog_code);

        return 1;
    }

    return 0;
}
