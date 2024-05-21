/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

/**
 * BUGS: - [x] lexer thinks that "131aboba" is a number 131
 *       - [-] lexer thinks that "_aboba228_ is unknown lexem"
 *       - [x] lexer does not take \n as a space between tokens (I THINK THE PROBLEM IS NOT IN THIS, NOT A BUG)
 *       - [x] syntaxer does not give an error if there is no ; in the end
 *       - [x] difference between index of keyword and its opcode is not always trivial
 *       - [x] there are no checks in many places if there are tokens left, if there are no more
 *       - [ ] tokens left, this may result in attempt to access area behind the array
 *       - [x] in some places i have to write 2 syntax asserts checking if there are tokens left and then
 *       - [ ] getting access to token if ir exists, so it results in copypaste
 *       - [x] lexer does not understand russian
 *       - [x] syntaxer allows same variable names in function parameters
 *       - [x] chaos with declaration check in lexer
 *       - [x] if no function declared - falls with segfault
 *       - [x] it should handle not Fucnitons sequence but (function | operation)+ sequence
 *       - [x] does not require having an entry point
 *       - [x] var declarators in tree are not represented
 *       - [ ] what if we delete global scope?
 *
 * TODO: - fix bugs (lol)
*/

#include "frontend.h"

int main (int argc, char* argv[])
{
    if (argc < 2)
        RET_ERROR (1, "No program file specified");

    char* prog_name = argv[1];

    ProgText* prog_text = GetProgText (prog_name);
    ProgCode* prog_code = LexerTokenize (prog_text);
    ProgTextDtor (prog_text);
    if (!prog_code) RET_ERROR (1, "Error during syntax analysis");

    Tree* ast = BuildAST (prog_code);
    ProgCodeDtor (prog_code);

    TreeDotDump ("dump.html", ast);

    WriteAST (ast, prog_name);

    TreeDtor (ast);

    PRINTF_DEBUG ("FRONTEND ok");

    return 0;
}

// ================================================================================================

ProgText* GetProgText (const char* prog_name)
{
    assert (prog_name);

    FILE* prog_file = fopen (prog_name, "rb");
    int prog_size   = GetProgSize (prog_file);

    char* raw_prog_text = (char*) calloc (prog_size, sizeof (char));
    fread ((char*) raw_prog_text, sizeof (char), prog_size, prog_file); // todo check syscall

    ProgText* prog_text = ProgTextCtor (raw_prog_text, prog_size);
    free (raw_prog_text);
    fclose (prog_file);

    return prog_text;
}

// ================================================================================================

WriteTreeRes WriteAST (const Tree* ast, const char* prog_fname)
{
    assert (ast);
    assert (prog_fname);

    WriteTreeRes wrt_res = WRT_TREE_SUCCESS;
    FILE* ast_file = fopen ("ast.ast", "wb");
    wrt_res = WriteTree (ast_file, ast);
    fclose (ast_file);

    return wrt_res;
}

// ================================================================================================

Tree* BuildAST (ProgCode* prog_code)
{
    assert (prog_code);

    Tree* ast = TreeCtor();
    NameTableCopy (ast->nametable, prog_code->nametable);

    ScopeTableStack* sts = ScopeTableStackCtor ();

    PushScope (sts); // global scope

    ast->root = GetAST (prog_code, sts);

    ScopeTableStackDtor (sts);

    return ast;
}

// ================================================================================================

TreeNode* GetAST (ProgCode* prog_code, ScopeTableStack* sts)
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
                        code_block, new_statement);

    } while (new_statement && HAS_TOKENS_LEFT);

    SYNTAX_ASSERT (code_block != NULL, "Expected at least one function or statements");

    return code_block;
}

// ================================================================================================

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

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, BEGIN_FUNC_PARAMS),
                    "separator begin function params expected");

    OFFSET++; // skip "("

    TreeNode* new_identifier = NULL;
    TreeNode* params_block   = NULL;

    int n_params = 0;

    do
    {
        new_identifier = GetParameter (prog_code, sts);

        if (new_identifier)
        {
            n_params++;

            DECLARE (new_identifier);

            params_block = TreeNodeCtor (END_STATEMENT, SEPARATOR,
                                         NULL, params_block, new_identifier);
        }
    } while (new_identifier);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_FUNC_PARAMS),
                    "separator end function params expected");

    prog_code->nametable->n_params[VAL (func_id)] = n_params;

    OFFSET++; // skip ")"

    TreeNode* func_body = GetStatementBlock (prog_code, sts);
    SYNTAX_ASSERT (func_body != NULL, "no function body");

    CLOSE_SCOPE;

    TreeNode* func_info =
        TreeNodeCtor (END_STATEMENT, SEPARATOR, NULL, params_block, func_id);

    return TreeNodeCtor (FUNC_DECLARATOR, DECLARATOR,
                         NULL, func_body, func_info);
}

// ================================================================================================

TreeNode* GetCompoundStatement (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* wrapped_statement = GetStatementBlock (prog_code, sts);
    if (wrapped_statement) return wrapped_statement;

    OFFSET = init_offset;

    wrapped_statement = GetSingleStatement (prog_code, sts);

    return wrapped_statement;
}

// ================================================================================================

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
                                            NULL, statement_block, new_statement);
    } while (new_statement);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_STATEMENT_END),
                   "Missing enclose statement bracket \"я_олигарх_мне_заебись\"");

    OFFSET++; // skip }

    CLOSE_SCOPE;

    return statement_block;
}

// ================================================================================================

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

    single_statement = GetInput (prog_code, sts);
    if (single_statement)
    {
        SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_STATEMENT),
            "\"сомнительно_но_окей\" expected in the end of statement");

        OFFSET++; // skip ";"

        return single_statement;
    }

    OFFSET = init_offset;

    single_statement = GetPrint (prog_code, sts);
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
    {
        OFFSET = init_offset;

        return NULL; // as the last one
    }

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, END_STATEMENT),
                   "\"сомнительно_но_окей\" expected in the end of statement");

    OFFSET++; // skip ";"

    return single_statement;
}

// ================================================================================================

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

    TreeNode* while_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (while_statement != NULL, "No compound statement given in while");

    return TreeNodeCtor (KW_WHILE, KEYWORD, NULL, while_statement, condition);
}

// ================================================================================================

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

    TreeNode* if_and_else_statements = TreeNodeCtor (KW_IF, KEYWORD, NULL, if_statement, NULL);

    if (!HAS_TOKENS_LEFT || TOKEN_IS_NOT (KEYWORD, KW_ELSE))
        return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_and_else_statements, condition);

    OFFSET++; // skip "else"

    TreeNode* else_statement = GetCompoundStatement (prog_code, sts);
    SYNTAX_ASSERT (else_statement != NULL, "\"else\" statement expected");

    if_and_else_statements->right = else_statement;

    return TreeNodeCtor (KW_IF, KEYWORD, NULL, if_and_else_statements, condition);
}

// ================================================================================================

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

    SubtreeDtor (condition);

    return TreeNodeCtor (KW_DO, KEYWORD, NULL, NULL, wrapped_statement);
}

// ================================================================================================

TreeNode* GetAssign (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset  = OFFSET;

    int var_is_being_declared = 0;

    if (TOKEN_IS (DECLARATOR, VAR_DECLARATOR))
    {
        var_is_being_declared = 1;
        OFFSET++; // skip "let"
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

    TreeNode* assign_node = TreeNodeCtor (ASSIGN, OPERATOR, NULL, lvalue, rvalue);

    if (var_is_being_declared)
        return TreeNodeCtor (VAR_DECLARATOR, DECLARATOR, NULL, assign_node, NULL);

    return assign_node;
}

// ================================================================================================

TreeNode* GetInput (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_INPUT))
        return NULL;

    OFFSET++; // skip "input"

    TreeNode* input_var = GetIdentifier (prog_code, sts);
    SYNTAX_ASSERT (input_var != NULL, "Identifier expected after input keyword");

    return TreeNodeCtor (KW_INPUT, KEYWORD, NULL, NULL, input_var);
}

// ================================================================================================

TreeNode* GetReturn (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_RETURN))
        return NULL;

    OFFSET++; // skip "return"

    TreeNode* return_value = GetRvalue (prog_code, sts);
    SYNTAX_ASSERT (return_value != NULL, "Return value expected after return");

    return TreeNodeCtor (KW_RETURN, KEYWORD, NULL, return_value, NULL);
}

// ================================================================================================

TreeNode* GetPrint (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (KEYWORD, KW_PRINT))
        return NULL;

    OFFSET++; // skip "print"

    TreeNode* print_value  = GetRvalue (prog_code, sts);
    SYNTAX_ASSERT (print_value != NULL, "Print value (nil)");

    return TreeNodeCtor (KW_PRINT, KEYWORD, NULL, NULL, print_value);
}

// ================================================================================================

TreeNode* GetRvalue (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    return GetMathExprRes (prog_code, sts);
}

// ================================================================================================

TreeNode* GetLvalue (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    return GetIdentifier (prog_code, sts);
}

// ================================================================================================

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
        (
            TOKEN_IS_NOT (OPERATOR, LESS)     &&
            TOKEN_IS_NOT (OPERATOR, LESS_EQ)  &&
            TOKEN_IS_NOT (OPERATOR, EQUAL)    &&
            TOKEN_IS_NOT (OPERATOR, MORE_EQ)  &&
            TOKEN_IS_NOT (OPERATOR, MORE)     &&
            TOKEN_IS_NOT (OPERATOR, UNEQUAL)
        )
    )
        return math_expr_res;

    int op_cmp = VAL (CURR_TOKEN);

    OFFSET++; // skip operator

    TreeNode* curr_add_sub_res = GetAddSubRes (prog_code, sts);
    SYNTAX_ASSERT (curr_add_sub_res != NULL, "x >= <error> - nil after comparison operator");

    switch (op_cmp)
    {
    case LESS:
        math_expr_res = TreeNodeCtor (LESS, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    case LESS_EQ:
        math_expr_res = TreeNodeCtor (LESS_EQ, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    case EQUAL:
        math_expr_res = TreeNodeCtor (EQUAL, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    case MORE_EQ:
        math_expr_res = TreeNodeCtor (MORE_EQ, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    case MORE:
        math_expr_res = TreeNodeCtor (MORE, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    case UNEQUAL:
        math_expr_res = TreeNodeCtor (UNEQUAL, OPERATOR, NULL, math_expr_res, curr_add_sub_res);
        break;

    default:
        SYNTAX_ASSERT (0, "comparison operator expected");
        break;
    }

    return math_expr_res;
}

// ================================================================================================

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
        (
            TOKEN_IS_NOT (OPERATOR, ADD) &&
            TOKEN_IS_NOT (OPERATOR, SUB))
        )
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
            add_sub_res = TreeNodeCtor (ADD, OPERATOR, NULL, add_sub_res, curr_mul_div_res);
            break;

        case SUB:
            add_sub_res = TreeNodeCtor (SUB, OPERATOR, NULL, add_sub_res, curr_mul_div_res);
            break;

        default:
            RET_ERROR (NULL, "Syntax error, add or sub operator expected");
            break;
        }
    }

    return add_sub_res;
}

// ================================================================================================

TreeNode* GetMulDivRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    TreeNode* mul_div_res = GetSqrtRes (prog_code, sts);
    if (!mul_div_res)
    {
        WARN ("mul_div_res nil");
        OFFSET = init_offset;

        return NULL;
    }

    if (!HAS_TOKENS_LEFT ||
        (
            TOKEN_IS_NOT (OPERATOR, MUL) &&
            TOKEN_IS_NOT (OPERATOR, DIV))
        )
        return mul_div_res;

    while (HAS_TOKENS_LEFT && (
           TOKEN_IS (OPERATOR, MUL) ||
           TOKEN_IS (OPERATOR, DIV)))
    {
        int op_mul_div = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_sqrt_res = GetSqrtRes (prog_code, sts);
        if (!curr_sqrt_res)
        {
            SubtreeDtor (mul_div_res);
            SYNTAX_ASSERT (0, "x / <error> - nil after mul/div operator");
        }

        switch (op_mul_div)
        {
        case MUL:
            mul_div_res = TreeNodeCtor (MUL, OPERATOR, NULL, mul_div_res, curr_sqrt_res);
            break;

        case DIV:
            mul_div_res = TreeNodeCtor (DIV, OPERATOR, NULL, mul_div_res, curr_sqrt_res);
            break;

        default:
            RET_ERROR (NULL, "Syntax error, mul or div operator expected");
            break;
        }
    }

    return mul_div_res;
}

// ================================================================================================

TreeNode* GetSqrtRes (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    if (!HAS_TOKENS_LEFT || TOKEN_IS_NOT (OPERATOR, SQRT))
    {
        OFFSET = init_offset;

        return GetOperand (prog_code, sts);
    }

    OFFSET++; // skip "sqrt"

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR_L), "left bracket expected");

    OFFSET++; // skip "("

    TreeNode *sqrt_operand = GetAddSubRes (prog_code, sts);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR_R), "right bracket expected");

    OFFSET++; // skip ")"

    SYNTAX_ASSERT (sqrt_operand != NULL, "sqrt operand nil");

    TreeNode *sqrt_res = TreeNodeCtor (SQRT, OPERATOR, NULL, NULL, sqrt_operand);

    return sqrt_res;
}

// ================================================================================================

TreeNode* GetOperand (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_MATH_EXPR_L))
        return GetSimpleOperand (prog_code, sts);

    OFFSET++; // skip (

    TreeNode* math_expr = GetMathExprRes (prog_code, sts);
    SYNTAX_ASSERT (math_expr != NULL, "error inside brackets");

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR_R), "Missing separator \")\"");

    OFFSET++; // skip )

    return math_expr;
}

// ================================================================================================

TreeNode* GetSimpleOperand (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    TreeNode* ret_val = GetNumber (prog_code, sts);

    if (!ret_val)
        ret_val = GetFunctionCall (prog_code, sts);

    if (!ret_val)
        ret_val = GetIdentifier (prog_code, sts);

    return ret_val;
}

// ================================================================================================

TreeNode* GetFunctionCall (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    int init_offset = OFFSET;

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    TreeNode* identifier = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL); // something is wrong

    OFFSET++; // skip func name

    if (TOKEN_IS_NOT (SEPARATOR, ENCLOSE_MATH_EXPR_L))
    {
        OFFSET = init_offset;
        TreeNodeDtor (identifier);

        return NULL;
    }

    OFFSET++; // skip "("

    TreeNode* new_parameter = NULL;
    TreeNode* parameters    = NULL;

    int n_params = 0;

    do
    {
        new_parameter = GetMathExprRes (prog_code, sts);

        if (new_parameter)
        {
            SYNTAX_ASSERT (TYPE (new_parameter) != IDENTIFIER || IsIdDeclared (sts, VAL (new_parameter)), "Undeclared parameter");

            n_params++;

            parameters = TreeNodeCtor (END_STATEMENT, SEPARATOR,
                                       NULL, parameters, new_parameter);
        }
    } while (new_parameter && HAS_TOKENS_LEFT);

    SYNTAX_ASSERT (TOKEN_IS (SEPARATOR, ENCLOSE_MATH_EXPR_R), "Missing closing brackets");

    OFFSET++; // skip ")"

    SYNTAX_ASSERT (prog_code->nametable->n_params[VAL (identifier)] == n_params,
                    "Invalid parameters count (%s(%d) vs (%d))",
                    ID_NAME (identifier), prog_code->nametable->n_params[VAL (identifier)], n_params);

    identifier->left = parameters;

    return identifier;
}

// ================================================================================================

TreeNode* GetIdentifier (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    if (TYPE (prog_code->tokens[OFFSET - 1]) == DECLARATOR)
    {
        SYNTAX_ASSERT (IsIdDeclared (sts, VAL (CURR_TOKEN)) == 0,
            "Redeclaration of \"%s\"", prog_code->nametable->names[VAL (CURR_TOKEN)]);
        DECLARE (CURR_TOKEN);
    }

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL);
    SYNTAX_ASSERT (IsIdDeclared (sts, VAL (ret_val)),
                "Undefined reference to %s", ID_NAME (ret_val));
    OFFSET++; // skip identifier

    return ret_val;
}

// ================================================================================================

TreeNode* GetParameter (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    SYNTAX_ASSERT (IsIdDeclared (sts, VAL (CURR_TOKEN)) == 0, "Redeclaration of parameter");
    DECLARE (CURR_TOKEN);

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL);
    SYNTAX_ASSERT (IsIdDeclared (sts, VAL (ret_val)),
                "Undefined reference to %s", ID_NAME (ret_val));
    OFFSET++; // skip identifier

    return ret_val;
}

// ================================================================================================

TreeNode* GetNumber (ProgCode* prog_code, ScopeTableStack* sts)
{
    assert (prog_code);
    assert (sts);

    if (TYPE (CURR_TOKEN) != INT_LITERAL)
        return NULL;

    TreeNode* ret_val = TreeNodeCtor (VAL (CURR_TOKEN), TYPE (CURR_TOKEN), NULL, NULL, NULL);

    OFFSET++; // skip number

    return ret_val;
}

// ================================================================================================

// func too big
ProgCode* LexerTokenize (ProgText* text)
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
                new_node = TreeNodeCtor (VAR_DECLARATOR, DECLARATOR, NULL, NULL, NULL);
                break;

            case FUNC_DECLARATOR:
                new_node = TreeNodeCtor (FUNC_DECLARATOR, DECLARATOR, NULL, NULL, NULL);
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

            new_node = TreeNodeCtor (kw_index, KEYWORD, NULL, NULL, NULL);
        }

        else if (IsSeparator (lexem))
        {
            int sep_index = GetSeparatorIndex (lexem);
            if (sep_index == -1)
                LEXER_ERR ("Unexpected error: separator \"%s\" "
                           "index not found in separators table", lexem);

            new_node = TreeNodeCtor (sep_index, SEPARATOR, NULL, NULL, NULL);
        }

        else if (IsOperator (lexem))
        {
            int op_index = GetOperatorIndex (lexem);
            if (op_index == -1)
                LEXER_ERR ("Unexpected error: operator \"%s\" "
                           "index not found in operators table", lexem);

            new_node = TreeNodeCtor (op_index, OPERATOR, NULL, NULL, NULL);
        }

        else if (IsIdentifier (lexem))
        {
            int id_index = FindInNametable (lexem, prog_code->nametable);
            if (id_index == -1)
                id_index = UpdNameTable (lexem, prog_code->nametable);
            if (id_index == -1)
                LEXER_ERR ("Couldnt find/place identifier to nametable");

            if (IsMainFunction (lexem)) prog_code->nametable->main_index = id_index;

            new_node = TreeNodeCtor (id_index, IDENTIFIER, NULL, NULL, NULL);
        }

        else if (IsIntLiteral (lexem))
        {
            new_node = TreeNodeCtor (atoi (lexem), INT_LITERAL, NULL, NULL, NULL);
        }

        else
        {
            LEXER_ERR ("Unknown lexem \"%s\"", lexem);
        }

        prog_code->tokens[prog_code->size++] = new_node;
    }

    if (prog_code->nametable->main_index == -1)
        LEXER_ERR ("No function \"%s\", no entry point", MAIN_FUNC_NAME);

    return prog_code;
}

// ================================================================================================

int HasForeignAgent (ProgText* text)
{
    assert (text);

    if (text->offset != 0) return 0;

    char lexem[MAX_LEXEM] = "";
    char *next_word = NULL;

    int add_offset = 0;
    int is_first_call = 1;

    for (size_t i = 0; i < FOREIGN_AGENT_BANNER_WORDS; i++)
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

// ================================================================================================

int IsIdentifier (const char* lexem)
{
    assert (lexem);

    if (!isalpha (*lexem) && *lexem != '_' && !strchr (RU_SYMBOLS, *lexem)) return 0;

    while (*++lexem)
        if (!isalnum (*lexem) && *lexem != '_' && !strchr (RU_SYMBOLS, *lexem)) return 0;

    return 1;
}

// ================================================================================================

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

// ================================================================================================

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

// ================================================================================================

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

// ================================================================================================

int IsOperator (const char* lexem)
{
    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (lexem, OPERATORS[i].name))
            return 1;
    }

    return 0;
}

// ================================================================================================

int IsIntLiteral (const char* lexem)
{
    assert (lexem);

    if ((char) *lexem == '-') lexem++;

    while (isdigit (*lexem))
    {
        lexem++;
    }

    if (isspace (*lexem) || *lexem == 0)
        return 1;

    return 0;
}

// ================================================================================================

int IsMainFunction (const char* lexem)
{
    assert (lexem);

    return streq (lexem, MAIN_FUNC_NAME);
}

// ================================================================================================

int GetDeclaratorIndex (const char* declarator)
{
    assert (declarator);

    for (int i = 0; i < N_DECLARATORS; i++)
    {
        if (streq (declarator, DECLARATORS[i].name))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

// ================================================================================================

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

// ================================================================================================

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

// ================================================================================================

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

// ================================================================================================

ScopeTableStack* ScopeTableStackCtor ()
{
    ScopeTableStack* sts =
            (ScopeTableStack*) calloc (1, sizeof (ScopeTableStack));

    sts->capacity = MAX_SCOPE_DEPTH;
    sts->size     = 0;
    sts->is_declared =
            (int **) calloc (MAX_SCOPE_DEPTH, sizeof (int *));

    for (size_t i = 0; i < MAX_SCOPE_DEPTH; i++)
        sts->is_declared[i] =
            (int*) calloc (NAMETABLE_CAPACITY, sizeof (int)); // everything is false by default

    return sts;
}

// ================================================================================================

int ScopeTableStackDtor (ScopeTableStack* sts)
{
    assert (sts);

    for (size_t i = 0; i < MAX_SCOPE_DEPTH; i++)
        free (sts->is_declared[i]);

    free (sts->is_declared);

    sts->size = 0;

    free (sts);

    return 0;
}

// ================================================================================================

int PushScope (ScopeTableStack* sts)
{
    assert (sts);

    if (sts->size >= sts->capacity)
        return 1; // error code

    memset (sts->is_declared[sts->size], 0, NAMETABLE_CAPACITY * sizeof (int));

    sts->size += 1;

    return 0; // success
}

// ================================================================================================

int DelScope (ScopeTableStack* sts)
{
    assert (sts);

    if (sts->size <= 0)
        return 1; // error code

    sts->size -= 1;

    memset (sts->is_declared[sts->size], 0, NAMETABLE_CAPACITY * sizeof (int));

    return 0;
}

// ================================================================================================

int IsIdDeclared (const ScopeTableStack* sts, const int id_index)
{
    assert (sts);

    for (size_t i = 0; i < MAX_SCOPE_DEPTH; i++)
        if (sts->is_declared[i][id_index] == 1) return 1;

    return 0;
}

// ================================================================================================

int DeclareId (ScopeTableStack* sts, const int id_index)
{
    assert (sts);

    if (sts->size <= 0)
        return 1; // error code

    sts->is_declared[sts->size - 1][id_index] = 1;

    return 0; // success
}

// ================================================================================================

ProgCode* ProgCodeCtor ()
{
    ProgCode* prog_code = (ProgCode*) calloc (1, sizeof(ProgCode));

    prog_code->nametable = NameTableCtor ();

    prog_code->tokens = (TreeNode**) calloc (MAX_N_NODES, sizeof(TreeNode*));
    prog_code->size     = 0;
    prog_code->offset   = 0;

    return prog_code;
}

// ================================================================================================

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

// ================================================================================================


ProgText* ProgTextCtor (const char* text, int text_len)
{
    assert (text);

    char* text_copy = (char*) calloc (text_len, sizeof (char));

    strncpy (text_copy, text, text_len);

    ProgText* prog_text = (ProgText*) calloc (1, sizeof (ProgText));

    prog_text->text   = text_copy;
    prog_text->offset = 0;
    prog_text->len    = text_len;

    return prog_text;
}

// ================================================================================================

int ProgTextDtor (ProgText* prog_text)
{
    assert (prog_text);

    prog_text->offset = -1;
    prog_text->len    = -1;

    free (prog_text->text);

    free (prog_text);

    return 0;
}

// ================================================================================================

int GetProgSize (FILE* prog_file)
{
    assert (prog_file);

    fseek (prog_file, 0, SEEK_END);
    int size = ftell (prog_file);
    rewind (prog_file);

    return size;
}

// ================================================================================================

int StripLexem (char* lexem)
{
    assert (lexem);

    lexem[strcspn (lexem, "\t\r\n ")] = 0;

    return 0;
}

// ================================================================================================

int SyntaxAssert (int line, int has_tokens_left, int condition, ProgCode* prog_code, const char* format, ...)
{
    assert (prog_code);

    if (!condition)
    {
        if (has_tokens_left)
            fprintf (stderr, RED ("In token (TYPE = %d, VAL = %d) OFSSET = %d\n"),
                                       TYPE (CURR_TOKEN), VAL (CURR_TOKEN), OFFSET);
        fprintf (stderr, RED ("(%d) SYNTAX ERROR! "), line);

        va_list ptr;
        va_start (ptr, format);

        vfprintf (stderr, format, ptr);

        va_end (ptr);

        fprintf (stderr, RST_CLR "\n\n\n" RST_CLR);

        ProgCodeDtor (prog_code);

        return 1;
    }

    return 0;
}

// ================================================================================================

int CheckVarsDeclared (TreeNode* node, ScopeTableStack* sts)
{
    assert (sts);
    if (!node)
        return 1;

    int ret_val = 1;

    if (TYPE (node) == IDENTIFIER && IsIdDeclared (sts, VAL (node)) == 0)
        return 0;

    ret_val *= CheckVarsDeclared (node->left, sts) * CheckVarsDeclared (node->right, sts);

    return ret_val;
}
