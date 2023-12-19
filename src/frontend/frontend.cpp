/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

/**
 *  ДАННОЕ СООБЩЕНИЕ (МАТЕРИАЛ) СОЗДАНО И (ИЛИ) РАСПРОСТРАНЕНО ИНОСТРАННЫМ
 *  И РОССИЙСКИМ ЮРИДИЧЕСКИМ ЛИЦОМ, ВЫПОЛНЯЮЩИМ ФУНКЦИИ ИНОСТРАННОГО КОМПИЛЯТОРА
 *  А ТАКЖЕ ФИНАНСИРУЕТСЯ ИЗ ФОНДА КОШЕК ЕДИНИЧКИ И УПОМИНАЕТ НЕКОГО ИНОАГЕНТА
 *  ♂♂♂♂ Oleg ♂ TinCock ♂♂♂♂ (КТО БЫ ЭТО МОГ БЫТЬ). КОЛЯ ЛОХ КСТА, WHEN DANIL???
 *  ДЛЯ ПОЛУЧЕНИЯ ВЫИГРЫША НАЖМИТЕ ALT+F4.
*/

#define FOREIGN_AGENT "ДАННОЕ СООБЩЕНИЕ (МАТЕРИАЛ) СОЗДАНО И (ИЛИ) РАСПРОСТРАНЕНО ИНОСТРАННЫМ\n" \
                      "И РОССИЙСКИМ ЮРИДИЧЕСКИМ ЛИЦОМ, ВЫПОЛНЯЮЩИМ ФУНКЦИИ ИНОСТРАННОГО КОМПИЛЯТОРА\n" \
                      "А ТАКЖЕ ФИНАНСИРУЕТСЯ ИЗ ФОНДА КОШЕК ЕДИНИЧКИ И УПОМИНАЕТ НЕКОГО ИНОАГЕНТА\n" \
                      "♂♂♂♂ Oleg ♂ TinCock ♂♂♂♂ (КТО БЫ ЭТО МОГ БЫТЬ). КОЛЯ ЛОХ КСТА, WHEN DANIL???\n" \
                      "ДЛЯ ПОЛУЧЕНИЯ ВЫИГРЫША НАЖМИТЕ ALT+F4.\n"

#define DEC_VAR "але_здравствуйте_меня_зовут_ольга_звоню_вам_из_тинькок_банка_вам_удобно_сейчас_разговаривать"

#define SHOW_PROG_CODE for (int i = 0; i < prog_code->size; i++) printf ("%2d | t %d | v %5d |\n", i, TYPE(prog_code->tokens[i]), VAL(prog_code->tokens[i]));


/**
 * BUGS: - lexer thinks that "131aboba" is a number 131
 *       - lexer thinks that "_aboba228_ is unknown lexem"
 *       x lexer does not take \n as a space between tokens (I THINK THE PROBLEM IS NOT IN THIS, NOT A BUG)
 *       x syntaxer does not give an error if there is no ; in the end
 *       x difference between index of keyword and its opcode is not always trivial
 *       - there are no checks in many places if there are tokens left, if there are no more
 *         tokens left, this may result in attempt to access area behind the array
 *       - in some places i have to write 2 syntax asserts checking if there are tokens left and then
 *         getting access to token if ir exists, so it results in copypaste
 *       x lexer does not understand russian
 *       - syntaxer allows same variable names in function parameters
 *       - chaos with declaration check in lexer
 *       - if no function declared - falls with segfault
 *       - it should handle not Fucnitons sequence but (function | operation)+ sequence
 *       x does not require having an entry point
 *       x var declarators in tree are not represented
 *       - what if we delete global scope?
 *
 * TODO: - fix bugs (lol)
*/

#include "frontend.h"

int main()
{
    const char* math_code =
                        FOREIGN_AGENT
                        DEC_VAR " x я_так_чувствую aboba228 ^ 11 сомнительно_но_окей";

    const char* double_assign_code =
                        FOREIGN_AGENT
                        "олег_не_торопись\n"
                        DEC_VAR " x я_так_чувствую 11 сомнительно_но_окей\n"
                        DEC_VAR " y я_так_чувствую 12 сомнительно_но_окей\n"
                        "я_олигарх_мне_заебись\n";

    const char* doif_code =
                        FOREIGN_AGENT
                        "я_ссыкло_или_я_не_ссыкло "
                            "x я_так_чувствую 11 сомнительно_но_окей "
                        "какая_разница aboba228 > 11 ?";

    const char* while_code =
                        FOREIGN_AGENT
                        "ну_сколько_можно x > 11 ^ aboba228 ?\n"
                            "x я_так_чувствую x + 1 сомнительно_но_окей\n";

    const char* if_else_code =
                        FOREIGN_AGENT
                        "какая_разница aboba_18 > 666 / 2 ? "
                            "x я_так_чувствую 333 + 0 сомнительно_но_окей "
                        "я_могу_ошибаться "
                            "x я_так_чувствую 11 сомнительно_но_окей ";

    const char* if_code =
                        FOREIGN_AGENT
                        DEC_VAR " aboba_18 я_так_чувствую 228 сомнительно_но_окей "
                        DEC_VAR " x        я_так_чувствую 48  сомнительно_но_окей "
                        "какая_разница aboba_18 > 666 / 2 ? "
                            "x я_так_чувствую 333 + 0 сомнительно_но_окей ";

    const char* new_lexer_code =
                        FOREIGN_AGENT
                        DEC_VAR " x я_так_чувствую 11 сомнительно_но_окей "
                        "x я_так_чувствую 12 сомнительно_но_окей "
                        "никто_никогда_не_вернет год_2007 сомнительно_но_окей"
                        ;

    const char* func_lexer_code =
                        FOREIGN_AGENT

                        // DEC_VAR " платно я_так_чувствую 0 сомнительно_но_окей \n"
                        "россии_нужен ЦАРЬ за почти_без_переплат "
                        "олег_не_торопись \n"
                            DEC_VAR " платно я_так_чувствую 10 сомнительно_но_окей "
                            "никто_никогда_не_вернет платно сомнительно_но_окей "
                        "я_олигарх_мне_заебись \n"

                        "россии_нужен ТинькоффПлатинум за платно десять почти_без_переплат "
                        "олег_не_торопись \n"
                            "платно я_так_чувствую 0 сомнительно_но_окей\n"
                            "олег_не_торопись\n"
                                "платно я_так_чувствую 666 сомнительно_но_окей\n"
                            "я_олигарх_мне_заебись\n"
                        "я_олигарх_мне_заебись\n";

    ProgText* prog_text = ProgTextCtor (func_lexer_code, strlen (func_lexer_code) + 1);
    ProgCode* prog_code = LexicalAnalysisTokenize (prog_text);
    ProgTextDtor (prog_text);

    if (!prog_code) return 1;

    SHOW_PROG_CODE; // debug

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

    ScopeTableStack* sts = ScopeTableStackCtor ();

    PushScope (sts); // global scope

    ast->root = GetG (prog_code, sts);

    ScopeTableStackDtor (sts);

    return ast;
}

// ============================================================================================

TreeNode* GetG (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    TreeNode* new_statement = NULL;
    TreeNode* code_block = NULL;

    do
    {
        new_statement = GetFunctionDeclaration (prog_code, sts);

        if (!new_statement)
            new_statement = GetCompoundStatement (prog_code, sts);

        if (new_statement)
            code_block = TreeNodeCtor (END_STATEMENT, SEPARATOR, NULL,
                        code_block, NULL, new_statement);

    } while (new_statement && HAS_TOKENS_LEFT);

    SYNTAX_ASSERT (code_block != NULL, "Expected at least one function or statements");

    return code_block;
}

// ============================================================================================

TreeNode* GetFunctionDeclaration (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (DECLARATOR, FUNC_DECLARATOR))
        return NULL;

    OFFSET++; // skip "def" - func declarator

    OPEN_NEW_SCOPE;

    TreeNode* func_id = GetIdentifier (prog_code, sts);
    SYNTAX_ASSERT (func_id != NULL, "Identifier expected after function declarator");

    DECLARE (func_id);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, BEGIN_FUNC_PARAMS),
                    "separator begin function params expected");

    OFFSET++; // skip "("

    TreeNode* new_identifier = NULL;
    TreeNode* params_block   = NULL;

    do
    {
        new_identifier = GetIdentifier (prog_code, sts);

        if (new_identifier)
        {
            DECLARE (new_identifier);

            params_block = TreeNodeCtor (END_STATEMENT, SEPARATOR,
                                         NULL, params_block, NULL, new_identifier);
        }
    } while (new_identifier);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_FUNC_PARAMS),
                    "separator end function params expected");

    OFFSET++; // skip ")"

    TreeNode* func_body = GetStatementBlock (prog_code, sts);
    SYNTAX_ASSERT (func_body != NULL, "no function body");

    CLOSE_SCOPE;

    return TreeNodeCtor (FUNC_DECLARATOR, DECLARATOR,
                         NULL, func_body, params_block, func_id);
}

// ============================================================================================

TreeNode* GetCompoundStatement (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* wrapped_statement = GetStatementBlock (prog_code, sts);
    if (wrapped_statement)
        return wrapped_statement;

    OFFSET = init_offset;

    wrapped_statement = GetSingleStatement (prog_code, sts);

    return wrapped_statement;
}

// ============================================================================================

TreeNode* GetStatementBlock (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_STATEMENT_BEGIN))
        return NULL;

    OFFSET++; // skip {

    OPEN_NEW_SCOPE;

    TreeNode* new_statement = NULL;

    TreeNode* statement_block = NULL;

    do
    {
        new_statement = GetCompoundStatement (prog_code, sts);

        if (new_statement)
            statement_block = TreeNodeCtor (END_STATEMENT, SEPARATOR,
                                            NULL, statement_block, NULL, new_statement);
    } while (new_statement);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_STATEMENT_END),
                   "Missing enclose statement bracket \"я_олигарх_мне_заебись\"");

    OFFSET++; // skip }

    CLOSE_SCOPE;

    return statement_block;
}

// ============================================================================================

TreeNode* GetSingleStatement (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* single_statement = NULL;

    single_statement = GetIfElse (prog_code, sts);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetWhile (prog_code, sts);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetDoIf (prog_code, sts);
    if (single_statement)
        return single_statement;

    OFFSET = init_offset;

    single_statement = GetReturn (prog_code, sts);
    if (single_statement)
    {
        SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_STATEMENT),
               "\"сомнительно_но_окей\" expected in the end of statement");

        OFFSET++; // skip ";"

        return single_statement;
    }

    OFFSET = init_offset;

    single_statement = GetAssign (prog_code, sts);
    if (!single_statement)
        return NULL; // as the last one

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_STATEMENT),
                   "\"сомнительно_но_окей\" expected in the end of statement");

    OFFSET++; // skip ";"

    return single_statement;
}

// ============================================================================================

TreeNode* GetWhile (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (!HAS_TOKENS_LEFT || TOKEN_IS_NOT (KEYWORD, KW_WHILE))
        return NULL;

    OFFSET++; // skip "while"

    TreeNode* condition = GetMathExprRes (prog_code, sts);
    SYNTAX_ASSERT(condition != NULL, "condition error");

    SYNTAX_ASSERT (HAS_TOKENS_LEFT, "\"?\" expected in the end of condition");
    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "\"?\" expected in the end of condition");
    OFFSET++; // skip "?"

    TreeNode* wrapped_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (wrapped_statement != NULL, "No wrapped statement given in while");

    return TreeNodeCtor (KW_WHILE, KEYWORD, NULL, wrapped_statement, condition, NULL);
}

// ============================================================================================

TreeNode* GetIfElse (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_IF))
        return NULL;

    OFFSET++; // skip "if"

    TreeNode* condition = GetMathExprRes (prog_code, sts);
    SYNTAX_ASSERT (condition != NULL, "condition error");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "\"?\" expected in the end of condition");
    OFFSET++; // skip "?"

    TreeNode* if_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (if_statement != NULL, "No wrapped statement given in while");

    if (!HAS_TOKENS_LEFT || TOKEN_IS_NOT (KEYWORD, KW_ELSE))
        return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_statement, condition, NULL);

    OFFSET++; // skip "else"

    TreeNode* else_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (else_statement != NULL, "\"else\" statement expected");

    return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_statement, condition, else_statement);
}

// ============================================================================================

TreeNode* GetDoIf (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_DO))
        return NULL;

    OFFSET++; // skip "do"

    TreeNode* wrapped_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (wrapped_statement != NULL, "No statement inside do-if given");

    SYNTAX_ASSERT (TOKEN_IS (KEYWORD, KW_IF), "Keyword \"какая_разница\" expected");

    OFFSET++; // skip "if"

    TreeNode* condition = GetMathExprRes (prog_code, sts);
    SYNTAX_ASSERT (condition != NULL, "Condition expected");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_CONDITION), "Separator \"?\" expected");

    OFFSET++; // skip "?"

    return TreeNodeCtor (KW_DO, KEYWORD, NULL, wrapped_statement, condition, NULL);
}

// ============================================================================================

TreeNode* GetAssign (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset  = OFFSET;

    int var_is_being_declared = 0;
    PRINTF_DEBUG ("inside assign");
    if (TOKEN_IS (DECLARATOR, VAR_DECLARATOR))
    {
        var_is_being_declared = 1;
        OFFSET++; // skip "let" - declarator TODOTODO TODO TODO
    }

    TreeNode* id_token = CURR_TOKEN;

    TreeNode* lvalue = GetLvalue (prog_code, sts);

    if (var_is_being_declared)
    {
        SYNTAX_ASSERT (lvalue != NULL, "Identifier expected after veriable declarator");

        SYNTAX_ASSERT (VAL (id_token) > -1,
                        "%s not found in nametable", ID_NAME (id_token)); // precaution


        DECLARE (id_token);
    }

    else
    {
        if (!lvalue)
        {
            OFFSET = init_offset;

            return NULL;
        }

        SYNTAX_ASSERT (IsIdDeclared (sts, VAL (id_token)) == 1,
                    "Undefined reference to %s", ID_NAME (id_token));
    }

    if (var_is_being_declared)
        SYNTAX_ASSERT (TOKEN_IS (OPERATOR, ASSIGN), "Initialize variable");

    if (TOKEN_IS_NOT (OPERATOR, ASSIGN))
    {
        OFFSET = init_offset;

        WARN ("no assign operator");

        return NULL;
    }

    OFFSET++; // skip "=" operator

    TreeNode* rvalue = GetRvalue (prog_code, sts);

    SYNTAX_ASSERT (rvalue != NULL, "Rvalue (nil) error");

    TreeNode* assign_node = TreeNodeCtor (ASSIGN, OPERATOR, NULL, lvalue, NULL, rvalue);

    if (var_is_being_declared)
        return TreeNodeCtor (VAR_DECLARATOR, DECLARATOR, NULL, assign_node, NULL, NULL);

    return assign_node;
}

// ============================================================================================

TreeNode* GetReturn (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_RETURN))
        return NULL;

    OFFSET++; // skip "return"

    TreeNode* identifier = GetIdentifier (prog_code, sts);
    SYNTAX_ASSERT (identifier != NULL, "Identifier expected after return");

    return TreeNodeCtor (KW_RETURN, KEYWORD, NULL, identifier, NULL, NULL);
}

// ============================================================================================

TreeNode* GetRvalue (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    return GetMathExprRes (prog_code, sts);
}

// ============================================================================================

TreeNode* GetLvalue (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    return GetIdentifier (prog_code, sts);
}

// ============================================================================================

TreeNode* GetMathExprRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* math_expr_res = GetAddSubRes (prog_code, sts);
    if (!math_expr_res)
    {
        WARN ("add_sub_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!HAS_TOKENS_LEFT ||
        TOKEN_IS_NOT (OPERATOR, LESS)     &&
        TOKEN_IS_NOT (OPERATOR, LESS_EQ)  &&
        TOKEN_IS_NOT (OPERATOR, EQUAL)    &&
        TOKEN_IS_NOT (OPERATOR, MORE_EQ)  &&
        TOKEN_IS_NOT (OPERATOR, MORE)     &&
        TOKEN_IS_NOT (OPERATOR, UNEQUAL))
        return math_expr_res;

    int op_cmp = VAL (CURR_TOKEN);

    OFFSET++; // skip operator

    TreeNode* curr_add_sub_res = GetAddSubRes (prog_code, sts);
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

TreeNode* GetAddSubRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* add_sub_res = GetMulDivRes (prog_code, sts);
    if (!add_sub_res)
    {
        WARN ("mul_div_res_1 nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!HAS_TOKENS_LEFT ||
        TOKEN_IS_NOT (OPERATOR, ADD) &&
        TOKEN_IS_NOT (OPERATOR, SUB))
        return add_sub_res;

    while (HAS_TOKENS_LEFT && (
           TOKEN_IS (OPERATOR, ADD) ||
           TOKEN_IS (OPERATOR, SUB)))
    {
        int op_add_sub = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_mul_div_res = GetMulDivRes (prog_code, sts);
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

TreeNode* GetMulDivRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* mul_div_res = GetPowRes (prog_code, sts);
    if (!mul_div_res)
    {
        WARN ("mul_div_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!HAS_TOKENS_LEFT ||
        TOKEN_IS_NOT (OPERATOR, MUL) &&
        TOKEN_IS_NOT (OPERATOR, DIV))
        return mul_div_res;

    while (HAS_TOKENS_LEFT && (
           TOKEN_IS (OPERATOR, MUL) ||
           TOKEN_IS (OPERATOR, DIV)))
    {
        int op_mul_div = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_pow_res = GetPowRes (prog_code, sts);
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

TreeNode* GetPowRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;
    TreeNode* pow_res = GetOperand (prog_code, sts);
    if (!pow_res)
    {
        WARN ("pow_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!HAS_TOKENS_LEFT || TOKEN_IS_NOT (OPERATOR, POW))
        return pow_res;

    OFFSET++; // skip ^

    TreeNode* operand_2 = GetOperand (prog_code, sts);
    SYNTAX_ASSERT (operand_2 != NULL, "in power right operand nil");

    pow_res = TreeNodeCtor (POW, OPERATOR, NULL, pow_res, NULL, operand_2);

    return pow_res;
}

// ============================================================================================

TreeNode* GetOperand (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_MATH_EXPR))
        return GetSimpleOperand (prog_code, sts);

    OFFSET++; // skip $

    TreeNode* math_expr = GetMathExprRes (prog_code, sts);
    SYNTAX_ASSERT (math_expr != NULL, "error inside brackets");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR), "Missing separator \"$\"");

    OFFSET++; // skip $

    return math_expr;
}

// ============================================================================================

TreeNode* GetSimpleOperand (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    TreeNode* ret_val = GetIdentifier (prog_code, sts);

    if (!ret_val)
        return GetNumber (prog_code, sts);

    return ret_val;
}

// ============================================================================================

TreeNode* GetIdentifier (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL, NULL);

    OFFSET++; // skip identifier

    return ret_val;
}

// ============================================================================================

TreeNode* GetNumber (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

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

    if (!HasForeignAgent (text))
    {
        printf("%s", NO_FOREIGN_AGENT_BANNER_ERROR);

        return NULL;
    }

    ProgCode* prog_code = ProgCodeCtor ();

    int n_readen = 0;

    char lexem[MAX_LEXEM] = "";

    while (sscanf (text->text + text->offset, "%s%n", lexem, &n_readen) != EOF)
    {
        StripLexem (lexem);

        text->offset += n_readen;

        TreeNode* new_node = NULL;

        // the whole statement is quite unoptimal because many functions duplicate each other
        if (IsDeclarator (lexem))
        {
            int dclr_index = GetDeclaratorIndex (lexem);
            if (dclr_index == -1)
                LEXER_ERR ("Unexpected error: declarator \"%s\" "
                           "index not found in declarators table", lexem);

            switch (DECLARATORS[dclr_index].dclr_code)
            {
            case VAR_DECLARATOR:
                new_node = TreeNodeCtor (VAR_DECLARATOR, DECLARATOR, NULL, NULL, NULL, NULL);
                break;

            case FUNC_DECLARATOR:
                new_node = TreeNodeCtor (FUNC_DECLARATOR, DECLARATOR, NULL, NULL, NULL, NULL);
                break;

            default:
                LEXER_ERR ("Declarator not found in DECLARATORS[]");
                break;
            }
        }

        else if (IsKeyword (lexem)) // unoptimal, requires 2 cycles
        {
            int kw_index = GetKeywordIndex (lexem);
            if (kw_index == -1)
                LEXER_ERR ("Unexpected error: keyword \"%s\" "
                           "index not found in keywords table", lexem);

            new_node = TreeNodeCtor (kw_index, KEYWORD, NULL, NULL, NULL, NULL);
        }

        else if (IsSeparator (lexem))
        {
            int sep_index = GetSeparatorIndex (lexem);
            if (sep_index == -1)
                LEXER_ERR ("Unexpected error: separator \"%s\" "
                           "index not found in separators table", lexem);

            new_node = TreeNodeCtor (sep_index, SEPARATOR, NULL, NULL, NULL, NULL);
        }

        else if (IsOperator (lexem))
        {
            int op_index = GetOperatorIndex (lexem);
            if (op_index == -1)
                LEXER_ERR ("Unexpected error: operator \"%s\" "
                           "index not found in operators table", lexem);

            new_node = TreeNodeCtor (op_index, OPERATOR, NULL, NULL, NULL, NULL);
        }

        else if (IsIdentifier (lexem))
        {
            int id_index = FindInNametable (lexem, prog_code->nametable);
            if (id_index == -1)
                id_index = UpdNameTable (lexem, prog_code->nametable);
            if (id_index == -1)
                LEXER_ERR ("Couldnt find/place identifier to nametable");

            if (IsMainFunction (lexem)) prog_code->nametable->main_index = id_index;

            new_node = TreeNodeCtor (id_index, IDENTIFIER, NULL, NULL, NULL, NULL);
        }

        else if (IsIntLiteral (lexem))
        {
            new_node = TreeNodeCtor (atoi (lexem), INT_LITERAL, NULL, NULL, NULL, NULL);
        }

        else
        {
            LEXER_ERR ("Unknown lexem \"%s\"", lexem);
        }

        prog_code->tokens[prog_code->size++] = new_node;
    }

    // if (prog_code->nametable->main_index == -1)
        // LEXER_ERR ("No function \"%s\", no entry point", MAIN_FUNC_NAME);

    return prog_code;
}

// ============================================================================================

int HasForeignAgent (ProgText* text)
{
    assert (text);

    if (text->offset != 0) return 0;

    char lexem[MAX_LEXEM] = "";
    char *next_word = NULL;

    int add_offset = 0;
    int is_first_call = 1;

    for (int i = 0; i < FOREIGN_AGENT_BANNER_WORDS; i++)
    {
        sscanf (text->text + text->offset, "%s%n", lexem, &add_offset);

        next_word = strtok((is_first_call ? FOREIGN_AGENT_BANNER : NULL), " \n\r\t");

        if (is_first_call) is_first_call = 0;

        if (strcmp (lexem, next_word))
            return 0;

        text->offset += add_offset;
    }

    return 1;
}

// ============================================================================================

int IsIdentifier (const char* lexem)
{
    assert (lexem);

    if (!isalpha (*lexem) && *lexem != '_' && !strchr (RU_SYMBOLS, *lexem)) return 0;

    while (*++lexem)
        if (!isalnum (*lexem) && *lexem != '_' && !strchr (RU_SYMBOLS, *lexem)) return 0;

    return 1;
}

// ============================================================================================

int IsDeclarator (const char* lexem)
{
    assert (lexem);

    for (int i = 0; i < N_DECLARATORS; i++)
    {
        if (streq (lexem, DECLARATORS[i].name))
            return 1;
    }

    return 0;
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

int IsMainFunction (const char* lexem)
{
    assert (lexem);

    return streq (lexem, MAIN_FUNC_NAME);
}

// ============================================================================================

int GetDeclaratorIndex (const char* keyword)
{
    assert (keyword);

    for (int i = 0; i < N_DECLARATORS; i++)
    {
        if (streq (keyword, DECLARATORS[i].name))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
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

ScopeTableStack* ScopeTableStackCtor ()
{
    ScopeTableStack* sts =
            (ScopeTableStack*) calloc (1, sizeof (ScopeTableStack));

    sts->capacity = MAX_SCOPE_DEPTH;
    sts->size     = 0;
    sts->is_declared =
            (int **) calloc (MAX_SCOPE_DEPTH, sizeof (int *));

    for (int i = 0; i < MAX_SCOPE_DEPTH; i++)
        sts->is_declared[i] =
            (int*) calloc (NAMETABLE_CAPACITY, sizeof (int)); // everything is false by default

    return sts;
}

// ============================================================================================

int ScopeTableStackDtor (ScopeTableStack* sts)
{
    assert (sts);

    for (int i = 0; i < MAX_SCOPE_DEPTH; i++)
        free (sts->is_declared[i]);

    free (sts->is_declared);

    sts->size = 0;

    free (sts);

    return 0;
}

// ============================================================================================

int PushScope (ScopeTableStack* sts)
{
    assert (sts);

    if (sts->size >= sts->capacity)
        return 1; // error code

    memset (sts->is_declared[sts->size], 0, NAMETABLE_CAPACITY * sizeof (int));

    sts->size += 1;

    return 0; // success
}

// ============================================================================================

int DelScope (ScopeTableStack* sts)
{
    assert (sts);

    if (sts->size <= 0)
        return 1; // error code

    sts->size -= 1;

    memset (sts->is_declared[sts->size], 0, NAMETABLE_CAPACITY * sizeof (int));

    return 0;
}

// ============================================================================================

int IsIdDeclared (const ScopeTableStack* sts, const int id_index)
{
    assert (sts);

    for (int i = 0; i < MAX_SCOPE_DEPTH; i++)
        if (sts->is_declared[i][id_index] == 1) return 1;

    return 0;
}

// ============================================================================================

int DeclareId (ScopeTableStack* sts, const int id_index)
{
    assert (sts);

    if (sts->size <= 0)
        return 1; // error code

    sts->is_declared[sts->size - 1][id_index] = 1;

    return 0; // success
}

// ============================================================================================

ProgCode* ProgCodeCtor ()
{
    ProgCode* prog_code = (ProgCode*) calloc (1, sizeof(ProgCode));

    prog_code->nametable = NameTableCtor ();

    prog_code->tokens = (TreeNode**) calloc (MAX_N_NODES, sizeof(TreeNode*));
    prog_code->size     = 0;
    prog_code->offset   = 0;

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

int SyntaxAssert (int has_tokens_left, int condition, ProgCode* prog_code, const char* format, ...)
{
    assert (prog_code);

    if (!condition)
    {
        if (has_tokens_left)
            fprintf (stderr, RED ("In token (TYPE = %d, VAL = %d) OFSSET = %d\n"),
                                         TYPE (CURR_TOKEN), VAL (CURR_TOKEN), OFFSET);
        fprintf (stderr, RED ("SYNTAX ERROR! "));

        va_list  (ptr);
        va_start (ptr, format);

        vfprintf (stderr, format, ptr);

        va_end (ptr);

        fprintf (stderr, RST_CLR "\n\n\n" RST_CLR);

        ProgCodeDtor (prog_code);

        return 1;
    }

    return 0;
}
