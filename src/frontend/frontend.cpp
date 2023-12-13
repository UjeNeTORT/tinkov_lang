#include "frontend.h"


int main()
{

    PRINTF_DEBUG ("done");

    return 0;
}

// ============================================================================================

TreeNode* GetNumber (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    if (TYPE(CURR_TOKEN) != INT_LITERAL)
        return NULL;

    return prog_code->tokens[OFFSET++];
}

// ============================================================================================

TreeNode* GetIdentifier (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    return prog_code->tokens[OFFSET++];
}

// ============================================================================================

TreeNode* GetSimpleOperand (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    TreeNode* IdNode = GetIdentifier (prog_code, tree);
    if (IdNode == NULL)
        return GetNumber (prog_code, tree);

    return IdNode;
}

// ============================================================================================

TreeNode* GetOperand (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    int init_offset = OFFSET;

    if (TYPE (CURR_TOKEN) != SEPARATOR ||
        VAL  (CURR_TOKEN) != ENCLOSE_MATH_EXPR)
        return GetSimpleOperand(prog_code, tree);

    OFFSET++; // skip $

    TreeNode* math_expr = GetMathExpr(prog_code, tree);

    if (TYPE (CURR_TOKEN) != SEPARATOR ||
        VAL  (CURR_TOKEN) != ENCLOSE_MATH_EXPR)
    {
        OFFSET = init_offset;
        return GetSimpleOperand (prog_code, tree);
    }

    OFFSET++; // skip $

    return math_expr;
}

// ============================================================================================

TreeNode* GetPowRes (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    TreeNode* operand_1 = GetOperand (prog_code, tree);
    if (!operand_1) return NULL;

    TreeNode* operand_2 = NULL; // optional

    if (TYPE (CURR_TOKEN) == OPERATOR &&
        VAL  (CURR_TOKEN) == POW)
    {
        OFFSET++; // skip ^

        operand_2 = GetOperand (prog_code, tree);

        if (!operand_2) return NULL; // error: op1 ^ <error> => error
    }

    if (operand_2)
        return TreeNodeCtor (POW, OPERATOR, NULL, operand_1, operand_2);

    return operand_1;
}

// ============================================================================================

TreeNode* GetMulDivRes (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    TreeNode* pow_res_1 = GetPowRes (prog_code, tree);
    if (!pow_res_1) return NULL;

    if (TYPE (CURR_TOKEN) != OPERATOR ||
      !(VAL  (CURR_TOKEN) == MUL ||
        VAL  (CURR_TOKEN) == DIV))
        return pow_res_1;

    TreeNode* pow_res_opt = NULL; // optional

    while (TYPE (CURR_TOKEN) == OPERATOR &&
          (VAL  (CURR_TOKEN) == MUL ||
           VAL  (CURR_TOKEN) == DIV))
    {
        int op_mul_div = VAL (CURR_TOKEN);

        OFFSET++; // skip operator

        TreeNode* curr_pow_res = GetPowRes (prog_code, tree);
        !!! если pow res opt нулевой, то создаем узел с pow_res_1 слева и null справа и
            подпихиваем дальше, только непонятно как !!!
        switch (op_mul_div)
        {
        case MUL:
            break;

        case DIV:
            break;

        default:
            RET_ERROR (NULL, "Syntax error, mul or div operator expected"); // todo
            break;
        }
    }

    if (pow_res_2)
}

// ============================================================================================

TreeNode* GetLvalue (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    if (TYPE (CURR_TOKEN) != IDENTIFIER)
        return NULL;

    return prog_code->tokens[OFFSET++];
}

// ============================================================================================

TreeNode* GetRvalue (ProgCode* prog_code, Tree* tree)
{
    assert (prog_code);
    assert (tree);

    if (TYPE (CURR_TOKEN) != IDENTIFIER ||
        TYPE (CURR_TOKEN) != INT_LITERAL)
        return NULL;

    OFFSET++; // did not think enough about it

    while (TYPE (CURR_TOKEN) == )

    return prog_code->tokens[OFFSET++];
}

// ============================================================================================

// func too big
ProgCode* LexicalAnalysisTokenize (ProgText* text)
{
    assert(text);

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

            new_node = TreeNodeCtor (id_index, IDENTIFIER, NULL, NULL, NULL);
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

            new_node = TreeNodeCtor (kw_index, KEYWORD, NULL, NULL, NULL);
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

            new_node = TreeNodeCtor (sep_index, SEPARATOR, NULL, NULL, NULL);
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

            new_node = TreeNodeCtor (op_index, OPERATOR, NULL, NULL, NULL);
        }

        else if (IsIntLiteral (lexem))
        {
            new_node = TreeNodeCtor (atoi (lexem), INT_LITERAL, NULL, NULL, NULL);
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
        if (streq (lexem, KEYWORDS[i]))
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
        if (streq (lexem, SEPARATORS[i]))
            return 1;
    }

    return 0;
}

// ============================================================================================

int IsOperator (const char* lexem)
{
    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (lexem, OPERATORS[i]))
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
        if (streq (keyword, KEYWORDS[i]))
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
        if (streq (separator, SEPARATORS[i]))
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
        if (streq (operator_, OPERATORS[i]))
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

    NameTableDtor(prog_code->nametable);

    for (int i = 0; prog_code->tokens[i] && i < prog_code->size; i++)
        free (prog_code->tokens[i]);

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
