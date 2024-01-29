/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#include "backend.h"

int main (int argc, char* argv[])
{
    if (argc < 2)
        RET_ERROR (1, "No ast specified");

    char* ast_name = argv[1]; // todo check if .tree in the end

    FILE* tree_file = fopen (ast_name, "rb");
    Tree* ast = ReadTree (tree_file);
    fclose (tree_file);
    if (!ast)
        RET_ERROR (1, "Error during reading tree");

    TreeDotDump ("dump.html", ast);

    AsmText* asm_text = TranslateAST (ast);
    TreeDtor (ast);

    FILE* output_file = fopen ("out.tinkov", "wb");
    fprintf (output_file, "%s\n", asm_text->text);
    fclose (output_file);

    AsmTextDtor (asm_text);

    PRINTF_DEBUG ("BACKEND ok");

    return 0;
}

// ================================================================================================

AsmText* TranslateAST (const Tree* ast)
{
    assert (ast);

    // TODO move this block outside the func (to main maybe)
    AsmText* asm_text = AsmTextCtor();

    WRITE ("; this program was written in tinkov language, mne poxyi ya v americu\n\n%n");

    WRITE ("push 1 ; default main() parameter like argc\n"
           "call function_%d ; calling main function\n%n", ast->nametable->main_index);
    WRITE ("out\n%n");
    WRITE ("hlt\n\n\n%n");

    TranslateASTSubtree (ast->root, asm_text, ast->nametable);

    return asm_text;
}

// ================================================================================================

TranslateRes TranslateASTSubtree (const TreeNode* node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (!node)
    {
        WARN ("[%d] node (nil)", asm_text->offset);

        return TRANSLATE_SUCCESS;
    }

    if (TranslateDeclarator (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateKeyword (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateSeparator (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateOperator (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateIdentifier (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateNumber (node, asm_text, nametable) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    ERROR ("Couldnt translate node:\n"
           "[type = %d]\n"
           "[val  = %d]\n", TYPE (node), VAL (node));

    return TRANSLATE_ERROR;
}

// ================================================================================================

TranslateRes TranslateDeclarator (const TreeNode* declr_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (declr_node) != DECLARATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (VAL (declr_node) == VAR_DECLARATOR)
    {
        const TreeNode* var_id_node = declr_node->left->left;

        OffsetTableAddVariable (OFFSET_TABLE, VAL (var_id_node));

        TranslateASTSubtree (declr_node->left, asm_text, nametable);

        return TRANSLATE_SUCCESS;
    }
    else if (VAL (declr_node) == FUNC_DECLARATOR)
    {
        const TreeNode* func_id_node = declr_node->right->right;
        WRITE ("%sfunction_%d: ; \"%s\"\n%n", TABS, VAL (func_id_node), NAME (func_id_node));

        AsmTextAddTab (asm_text);

        int n_params = nametable->n_params[VAL (func_id_node)];

        WRITE ("%s; parameters[%d]\n%n", TABS, n_params);
        for (int i = 0; i < n_params; i++)
        {
            WRITE ("%s; parameter \"%s\" (%d)\n%n", TABS,
                nametable->names[nametable->params[VAL (func_id_node)][i]],
                nametable->params[VAL (func_id_node)][i]);
        }

        WRITE ("%s\n; BODY\n%n", TABS);

        OffsetTableNewFrame (OFFSET_TABLE);
        OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);

        TranslateASTSubtree (declr_node->left, asm_text, nametable);

        WRITE ("%sret\n\n%n", TABS);

        OffsetTableDeleteFrame (OFFSET_TABLE);

        AsmTextRemoveTab (asm_text);

        return TRANSLATE_SUCCESS;
    }

    return TRANSLATE_ERROR;
}

// ================================================================================================

TranslateRes TranslateKeyword (const TreeNode* kw_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (kw_node) != KEYWORD)
        return TRANSLATE_TYPE_NOT_MATCH;

    switch (VAL (kw_node))
    {
        case KW_PRINT:
        {
            WRITE ("%s; PRINTING\n%n", TABS);

            if (TYPE (kw_node->left) == INT_LITERAL)
            {
                WRITE ("%spush %d\n%n", TABS, VAL (kw_node->left));
            }

            else
            {
                WRITE ("%spush [rpx + %d] ; printing \"%s\"\n%n", TABS,
                    OffsetTableGetVarOffset (OFFSET_TABLE, VAL (kw_node->left)),
                    NAME (kw_node->left));
            }

            WRITE ("%sout\n%n", TABS);

            break;
        }

        case KW_RETURN:
        {
            if (TYPE (kw_node->left) == INT_LITERAL)
            {
                WRITE ("%spush %d\n%n", TABS, VAL (kw_node->left));
            }
            else
            {
                WRITE ("%spush [rpx + %d] ; returning \"%s\"\n%n", TABS,
                    OffsetTableGetVarOffset (OFFSET_TABLE, VAL (kw_node->left)),
                    NAME (kw_node->left));
            }

            WRITE ("%sret\n\n%n", TABS);
            break;
        }

        case KW_IF:
        {
            int curr_if = asm_text->if_statements_count++;

            // condition
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            WRITE ("%spush 0\n"
                   "%sjne if_%d\n"
                   "%sjmp else_%d\n%n",
                    TABS, TABS, curr_if,
                    TABS, curr_if)

            // if
            WRITE ("%sif_%d:\n%n", TABS, curr_if);

            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->left->left, asm_text, nametable);

            WRITE ("%sjmp end_if_%d\n%n",
                    TABS, curr_if);

            AsmTextRemoveTab (asm_text);

            // else
            WRITE ("%selse_%d:\n%n", TABS, curr_if);

            AsmTextAddTab (asm_text);

            if (kw_node->left->right)
                TranslateASTSubtree (kw_node->left->right, asm_text, nametable);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_if_%d:\n\n%n", TABS, curr_if);

            break;
        }

        case KW_WHILE:
        {
            int curr_while = asm_text->while_statements_count++;

            WRITE ("%swhile_%d:\n%n", TABS, curr_while);
            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->right, asm_text, nametable);
            WRITE ("%s; CONDITION\n%n", TABS);
            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%sje end_while_%d\n\n%n", TABS, curr_while);

            TranslateASTSubtree (kw_node->left, asm_text, nametable);
            WRITE ("%sjmp while_%d\n%n", TABS, curr_while);

            AsmTextRemoveTab (asm_text);
            WRITE ("%send_while_%d:\n\n%n", TABS, curr_while);

            break;
        }

        default:
            return TRANSLATE_ERROR;
    }

    WRITE ("%s\n%n", TABS);

    return TRANSLATE_SUCCESS;
}

// ================================================================================================

TranslateRes TranslateSeparator (const TreeNode* sep_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (sep_node) != SEPARATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (VAL (sep_node) == END_STATEMENT)
    {
        if (sep_node->left)
            if (TranslateASTSubtree (sep_node->left, asm_text, nametable) != TRANSLATE_SUCCESS)
                return TRANSLATE_ERROR;

        if (sep_node->right)
            return TranslateASTSubtree (sep_node->right, asm_text, nametable);

        return TRANSLATE_ERROR;
    }

    else if (VAL (sep_node) == ENCLOSE_STATEMENT_BEGIN)
    {
        if (sep_node->left)
            return TranslateASTSubtree (sep_node->left, asm_text, nametable);

        return TRANSLATE_ERROR;
    }

    return TRANSLATE_ERROR;
}

// ================================================================================================

TranslateRes TranslateOperator (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (op_node) != OPERATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    switch (VAL (op_node))
    {
        case ASSIGN:
        {
            TranslateASTSubtree (op_node->right, asm_text, nametable);
            WRITE ("%spop [rpx + %d] ; assign var \"%s\"\n\n%n", TABS, OffsetTableGetVarOffset (OFFSET_TABLE, VAL (op_node->left)), NAME (op_node->left));

            break;
        }

        case ADD:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("%sadd\n%n", TABS);

            break;
        }

        case SUB:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("%ssub\n%n", TABS);

            break;
        }

        case MUL:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("%smul\n%n", TABS);

            break;
        }

        case DIV:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("%sdiv\n%n", TABS);

            break;
        }

        case POW:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            WRITE ("%ssqrt\n%n", TABS);

            break;
        }

        case LESS:
        {
            WriteCondition (op_node, asm_text, nametable, "jbe");

            break;
        }

        case LESS_EQ:
        {
            WriteCondition (op_node, asm_text, nametable, "jb");

            break;
        }

        case EQUAL:
        {
            WriteCondition (op_node, asm_text, nametable, "jne");

            break;
        }

        case MORE_EQ:
        {
            WriteCondition (op_node, asm_text, nametable, "ja");

            break;
        }

        case MORE:
        {
            WriteCondition (op_node, asm_text, nametable, "jae");

            break;
        }

        case UNEQUAL:
        {
            WriteCondition (op_node, asm_text, nametable, "je");

            break;
        }

        default:
        {
            ERROR ("Operator %d not supported, be smarter please", VAL (op_node));

            return TRANSLATE_ERROR;
        }
    }

    return TRANSLATE_SUCCESS;
}

// ================================================================================================

TranslateRes TranslateIdentifier (const TreeNode* id_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (id_node) != IDENTIFIER)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (IsFunction (id_node, nametable))
    {
        // function call
        WRITE ("%s\n; function call of \"%s\" (%d)\n%n", TABS, NAME (id_node), VAL (id_node));

        PutFuncParamsToRAM (id_node, asm_text, nametable);

        WRITE ("%scall function_%d ; \"%s\"\n%n", TABS, VAL (id_node), NAME (id_node));

        // after function call routine
        OffsetTableDeleteFrame (OFFSET_TABLE);
        WRITE ("%spop rpx ; recover previous rpx value\n\n%n", TABS);

        return TRANSLATE_SUCCESS;
    }
    else
    {
        // variable
        WRITE ("%spush [rpx + %d] ; get value of \"%s\"\n%n", TABS,
            OffsetTableGetVarOffset (OFFSET_TABLE, VAL (id_node)), NAME (id_node));

        return TRANSLATE_SUCCESS;
    }

    return TRANSLATE_ERROR;
}

// ================================================================================================

TranslateRes TranslateNumber (const TreeNode* num_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (num_node) != INT_LITERAL)
        return TRANSLATE_TYPE_NOT_MATCH;

    WRITE ("%spush %d\n%n", TABS, VAL (num_node));

    return TRANSLATE_SUCCESS;
}

// ================================================================================================

int PutFuncParamsToRAM (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    PushParamsToStack (func_id_node, asm_text, nametable);

    WRITE ("%spush rpx\n%n", TABS);
    WRITE ("%spush rpx\n%n", TABS);
    WRITE ("%spush %d\n%n",  TABS, OffsetTableGetCurrFrameWidth (OFFSET_TABLE));
    WRITE ("%sadd\n%n",      TABS);
    WRITE ("%spop  rpx\n%n", TABS);

    OffsetTableNewFrame      (OFFSET_TABLE);
    OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);

    OffsetTableDump          (OFFSET_TABLE, nametable);

    PopParamsToRAM (func_id_node, asm_text, nametable);

    return 0;
}

// ================================================================================================

int PushParamsToStack (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    /**
     * pushing function parameters to stack happens before increase of rpx
     *
     * maybe i dont need all this shit, i can instead translate each parameter and it will be pushed this way
    */

    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    // !!! error may occur if function was not declared yet -
    // SOLUTION - create flag is_declared in nametable and fill it during backend

    int n_params = nametable->n_params[VAL (func_id_node)];

    WRITE ("%s; pushing %d parameters of function \"%s\" to stack\n%n", TABS, n_params, NAME (func_id_node));
    if (n_params == 0)
        return 0;

    const TreeNode* curr_param = func_id_node->left;

    for (int i = 0; i < n_params; i++)
    {
        WRITE ("%s; \"%s\"\n%n", TABS,
            nametable->names[nametable->params[VAL (func_id_node)][n_params - 1 - i]]);

        TranslateASTSubtree (curr_param->right, asm_text, nametable);

        curr_param = curr_param->left;
    }

    return 0;
}

// ================================================================================================

int PopParamsToRAM (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    /**
     * popping function parameters to RAM happens after increase of rpx
    */

    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    int n_params = nametable->n_params[VAL (func_id_node)];

    for (int i = 0; i < n_params; i++)
    {
        WRITE ("%spop [rpx + %d] ; put \"%s\" to RAM\n%n", TABS,
            OffsetTableGetVarOffset (OFFSET_TABLE, nametable->params[VAL (func_id_node)][i]),
            nametable->names[nametable->params[VAL (func_id_node)][i]]);
    }

    return 0;
}

// ================================================================================================

AsmText* AsmTextCtor ()
{
    AsmText* asm_text = (AsmText*) calloc (1, sizeof (AsmText));

    asm_text->text = (char*) calloc (MAX_ASM_PROGRAM_SIZE, sizeof (char));
    asm_text->offset = 0;

    asm_text->offset_table = OffsetTableCtor ();

    asm_text->if_statements_count    = 0;
    asm_text->while_statements_count = 0;
    asm_text->funcs_count            = 0;
    asm_text->cond_count             = 0;
    asm_text->tabs  = (char*) calloc (MAX_SCOPE_DEPTH, sizeof (char));

    return asm_text;
}

// ================================================================================================

int AsmTextDtor (AsmText* asm_text)
{
    assert (asm_text);

    OffsetTableDtor (asm_text->offset_table);

    asm_text->offset = -1;

    free (asm_text->text);
    free (asm_text->tabs);
    free (asm_text);

    return 0;
}

// ================================================================================================
// unsafe (buffer overflow)
int AsmTextAddTab (AsmText* asm_text)
{
    assert (asm_text);

    if (asm_text->tabs[0] == 0)
    {
        asm_text->tabs[0] = '\t';

        return 0;
    }

    char* last_tab = strrchr (asm_text->tabs, '\t');
    last_tab++;
    *last_tab = '\t';

    return 0;
}

// ================================================================================================
// unsafe (buffer overflow)
int AsmTextRemoveTab (AsmText* asm_text)
{
    assert (asm_text);

    char* last_tab = strrchr (asm_text->tabs, '\t');
    *last_tab = 0;

    return 0;
}

// ================================================================================================

int IsFunction (const TreeNode* id_node, const NameTable* nametable)
{
    assert (id_node);
    assert (nametable);

    if (nametable->n_params[VAL (id_node)] > -1)
        return 1;

    return 0;
}

// ================================================================================================

int WriteCondition (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char* comparator)
{
    assert (op_node);
    assert (asm_text);
    assert (nametable);
    assert (comparator);

    WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
    TranslateASTSubtree (op_node->left, asm_text, nametable);
    TranslateASTSubtree (op_node->right, asm_text, nametable);

    AsmTextAddTab (asm_text);

    WRITE ("%spush 0\n%n", TABS);
    WRITE ("%spop rax\n%n", TABS);
    WRITE ("%s%s end_condition_%d\n%n", TABS, comparator, asm_text->cond_count);
    WRITE ("%spush 1\n%n", TABS);
    WRITE ("%spop rax\n%n", TABS);

    AsmTextRemoveTab (asm_text);

    WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
    WRITE ("%spush rax\n%n", TABS);

    asm_text->cond_count++;

    return 0;
}

// ================================================================================================

OffsetTable* OffsetTableCtor ()
{
    OffsetTable* offset_table = (OffsetTable*) calloc (1, sizeof (OffsetTable));
    offset_table->ram_tables  = (RamTable*)    calloc (MAX_SCOPE_DEPTH, sizeof (RamTable));
    offset_table->curr_table_index = 0;

    for (int i = 0; i < MAX_SCOPE_DEPTH; i++)
    {
        for (int j = 0; j < NAMETABLE_CAPACITY; j++)
        {
            offset_table->ram_tables[i].index_in_ram[j] = -1;
        }

        offset_table->ram_tables[i].free_ram_index = 0;
    }

    return offset_table;
}

// ================================================================================================

int OffsetTableDtor (OffsetTable* offset_table)
{
    assert (offset_table);

    offset_table->curr_table_index = 0;
    free (offset_table->ram_tables);
    free (offset_table);

    return 0;
}

// ================================================================================================

int OffsetTableNewFrame (OffsetTable* offset_table)
{
    assert (offset_table);

    if (-1 < offset_table->curr_table_index &&
             offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    {
        offset_table->curr_table_index++;

        return 0; // return code - success
    }

    return 1; // return code - error
}

// ================================================================================================

int OffsetTableDeleteFrame (OffsetTable* offset_table)
{
    assert (offset_table);

    if (0 < offset_table->curr_table_index &&
            offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    {
        for (int j = 0; j < NAMETABLE_CAPACITY; j++)
        {
            offset_table->ram_tables[offset_table->curr_table_index].index_in_ram[j] = -1;
        }

        offset_table->curr_table_index--;

        return 0; // return code - success
    }

    return 1; // return code - error
}

// ================================================================================================

int OffsetTableAddVariable (OffsetTable* offset_table, int var_id)
{
    assert (offset_table);
    assert (var_id > -1);

    CURR_RAM_TABLE.index_in_ram[CURR_RAM_TABLE.free_ram_index] = var_id;
    CURR_RAM_TABLE.free_ram_index++;

    return 0; // return code - success
}

// ================================================================================================

int OffsetTableGetVarOffset (OffsetTable* offset_table, int var_id)
{
    assert (offset_table);
    assert (0 <= var_id && var_id < NAMETABLE_CAPACITY);

    for (int i = 0; i < sizeof (CURR_RAM_TABLE.index_in_ram); i++)
    {
        if (CURR_RAM_TABLE.index_in_ram[i] == var_id)
            return i;
    }

    return -1; // return code - not found
}

// ================================================================================================

int OffsetTableAddFuncParams (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable)
{
    assert (offset_table);
    assert (func_id_node);

    for (int i = 0; i < nametable->n_params[VAL (func_id_node)]; i++)
        OffsetTableAddVariable (offset_table, nametable->params[VAL (func_id_node)][i]);

    return 0;
}

// ================================================================================================

int OffsetTableGetCurrFrameWidth (OffsetTable* offset_table)
{
    assert (offset_table);

    int frame_width = 0;

    while (CURR_RAM_TABLE.index_in_ram[frame_width] != -1)
        frame_width++;

    return frame_width;
}

// ================================================================================================

int OffsetTableDump (const OffsetTable* offset_table, const NameTable* nametable)
{
    assert (offset_table);

    for (int curr_table_id = 0; curr_table_id < offset_table->curr_table_index + 1; curr_table_id++)
    {
        printf ("============================================ table %d ============================================\n", curr_table_id);

        for (int i = 0; i < NAMETABLE_CAPACITY; i++)
            printf ("%6d ", i);

        printf ("\n");

        for (int i = 0; i < NAMETABLE_CAPACITY; i++)
            printf ("%6d ", offset_table->ram_tables[curr_table_id].index_in_ram[i]);

        printf ("\n");

        if (nametable)
            for (int i = 0; i < NAMETABLE_CAPACITY; i++)
                if (offset_table->ram_tables[curr_table_id].index_in_ram[i] != -1)
                    printf ("%6.6s ", nametable->names[offset_table->ram_tables[curr_table_id].index_in_ram[i]]);

        printf ("\n\n");
    }

    return 0;
}

