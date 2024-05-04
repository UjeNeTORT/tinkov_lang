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

    AsmText* asm_text = AsmTextCtor ();
    asm_text = TranslateAST (ast->root, asm_text, ast->nametable);
    TreeDtor (ast);

    FILE* output_file = fopen ("out.tinkov", "wb");
    fprintf (output_file, "%s\n", asm_text->text);
    fclose (output_file);

    AsmTextDtor (asm_text);

    PRINTF_DEBUG ("BACKEND ok");

    return 0;
}

// ================================================================================================

AsmText* TranslateAST (const TreeNode* root_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);

    WRITE ("; this program was written in tinkov language, mne poxyi ya v americu\n\n");

    WRITE ("push %ld\n", LOCAL_VARIABLES_MAPPING);
    WRITE ("pop rpx\n");
    WRITE ("push 1 ; default main() parameter like argc\n"
           "call function_%d ; calling main function\n", nametable->main_index);

    WRITE ("hlt\n\n\n");

    TranslateASTSubtree (root_node, asm_text, nametable);

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

    switch (VAL (declr_node))
    {
        case VAR_DECLARATOR:
        {
            const TreeNode* var_id_node = declr_node->left->left;

            OffsetTableAddVariable (OFFSET_TABLE, VAL (var_id_node));

            TranslateASTSubtree (declr_node->left, asm_text, nametable);

            return TRANSLATE_SUCCESS;
        }

        case FUNC_DECLARATOR:
        {
            const TreeNode* func_id_node = declr_node->right->right;
            WRITE ("function_%d: ; \"%s\"\n", VAL (func_id_node), NAME (func_id_node));

            AsmTextAddTab (asm_text);

            int n_params = nametable->n_params[VAL (func_id_node)];

            WRITE ("; parameters[%d]\n", n_params);
            for (int i = 0; i < n_params; i++)
            {
                WRITE ("; parameter \"%s\" (%d)\n",
                    nametable->names[nametable->params[VAL (func_id_node)][i]],
                    nametable->params[VAL (func_id_node)][i]);
            }

            WRITE ("\n; BODY\n");

            OffsetTableAddFrame (OFFSET_TABLE);
            OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);

            TranslateASTSubtree (declr_node->left, asm_text, nametable);

            WRITE ("ret\n\n");

            OffsetTableDeleteFrame (OFFSET_TABLE);

            AsmTextRemoveTab (asm_text);

            return TRANSLATE_SUCCESS;
        }

        default:
            return TRANSLATE_ERROR;
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
        case KW_INPUT:
        {
            WRITE ("in ; input of variable \"%s\"\n",
                NAME (kw_node->right));
            WRITE ("pop [rpx + %d]\n",
                OffsetTableGetVarOffset (OFFSET_TABLE, VAL (kw_node->right)));

            break;
        }

        case KW_PRINT:
        {
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            WRITE ("out\n");

            break;
        }

        case KW_RETURN:
        {
            WRITE ("; return \n");

            TranslateASTSubtree (kw_node->left, asm_text, nametable);

            WRITE ("pop     rrx ; obligatory return value\n");

            WRITE ("ret\n\n");
            break;
        }

        case KW_IF:
        {
            size_t curr_if = asm_text->if_statements_count++;

            // condition
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            WRITE ("push    0\n");
            WRITE ("jne     if_%ld\n", curr_if);
            WRITE ("jmp     else_%ld\n", curr_if);

            // if
            WRITE ("if_%ld:\n", curr_if);

            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->left->left, asm_text, nametable);

            WRITE ("jmp     end_if_%ld\n", curr_if);

            AsmTextRemoveTab (asm_text);

            // else
            WRITE ("else_%ld:\n", curr_if);

            AsmTextAddTab (asm_text);

            if (kw_node->left->right)
                TranslateASTSubtree (kw_node->left->right, asm_text, nametable);

            AsmTextRemoveTab (asm_text);

            WRITE ("end_if_%ld:\n\n", curr_if);

            break;
        }

        case KW_DO: // do if
        {
            // ignore condition (it is not even presented in ast)
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            break;
        }

        case KW_WHILE:
        {
            size_t curr_while = asm_text->while_statements_count++;

            WRITE ("while_%ld:\n", curr_while);
            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->right, asm_text, nametable);
            WRITE ("; CONDITION\n");
            WRITE ("push    0\n");
            WRITE ("je      end_while_%ld\n\n", curr_while);

            TranslateASTSubtree (kw_node->left, asm_text, nametable);
            WRITE ("jmp     while_%ld\n", curr_while);

            AsmTextRemoveTab (asm_text);
            WRITE ("end_while_%ld:\n\n", curr_while);

            break;
        }

        default:
            return TRANSLATE_ERROR;
    }

    WRITE ("\n");

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

    switch (VAL (sep_node))
    {
        case END_STATEMENT:
        {
            // translate code from tree bottom to the current container node
            if (sep_node->left)
                if (TranslateASTSubtree (sep_node->left, asm_text, nametable) != TRANSLATE_SUCCESS)
                    return TRANSLATE_ERROR;

            // translation of the container node
            if (sep_node->right)
                return TranslateASTSubtree (sep_node->right, asm_text, nametable);

            return TRANSLATE_ERROR;
        }

        default:
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
            WRITE ("pop     [rpx + %d] ; assign var \"%s\"\n\n", OffsetTableGetVarOffset (OFFSET_TABLE, VAL (op_node->left)), NAME (op_node->left));

            break;
        }

        case ADD:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("add\n");

            break;
        }

        case SUB:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("sub\n");

            break;
        }

        case MUL:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("mul\n");

            break;
        }

        case DIV:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            WRITE ("div\n");

            break;
        }

        case POW:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            WRITE ("sqrt\n");

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
        WRITE ("; calling \"%s\" (%d)\n", NAME (id_node), VAL (id_node));

        PutFuncParamsToRAM (id_node, asm_text, nametable);

        WRITE ("call    function_%d ; \"%s\"\n", VAL (id_node), NAME (id_node));

        // after function call routine
        OffsetTableDeleteFrame (OFFSET_TABLE);
        WRITE ("pop     rpx ; recover previous rpx value\n\n");
        WRITE ("push    rrx ; return value\n\n");

        return TRANSLATE_SUCCESS;
    }
    else
    {
        // variable
        WRITE ("push    [rpx + %d] ; \"%s\"\n",
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

    WRITE ("push    %d\n", VAL (num_node));

    return TRANSLATE_SUCCESS;
}

// ================================================================================================

DefaultFuncRes PutFuncParamsToRAM (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    PushParamsToStack (func_id_node, asm_text, nametable);

    WRITE ("push    rpx\n");
    WRITE ("push    rpx\n");
    WRITE ("push    %d\n", OffsetTableGetCurrFrameWidth (OFFSET_TABLE));
    WRITE ("add\n");
    WRITE ("pop     rpx\n");
    WRITE ("pop     rax ; save previous rpx to rax\n");

    OffsetTableAddFrame      (OFFSET_TABLE);
    OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);

    PopParamsToRAM (func_id_node, asm_text, nametable);
    WRITE ("push    rax ; recover previous rpx from rax\n");

    return FUNC_SUCCESS;
}

// ================================================================================================

DefaultFuncRes PushParamsToStack (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    int n_params = nametable->n_params[VAL (func_id_node)];

    WRITE ("; pushing %d parameters of function \"%s\" to stack\n", n_params, NAME (func_id_node));
    if (n_params == 0)
        return FUNC_SUCCESS;

    const TreeNode* curr_param = func_id_node->left;

    for (int i = 0; i < n_params; i++)
    {
        WRITE ("; \"%s\"\n",
            nametable->names[nametable->params[VAL (func_id_node)][n_params - 1 - i]]);

        TranslateASTSubtree (curr_param->right, asm_text, nametable);

        curr_param = curr_param->left;
    }

    return FUNC_SUCCESS;
}

// ================================================================================================

DefaultFuncRes PopParamsToRAM (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
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
        WRITE ("pop     [rpx + %d] ; put \"%s\" to RAM\n",
            OffsetTableGetVarOffset (OFFSET_TABLE, nametable->params[VAL (func_id_node)][i]),
            nametable->names[nametable->params[VAL (func_id_node)][i]]);
    }

    return FUNC_SUCCESS;
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

DefaultFuncRes AsmTextDtor (AsmText* asm_text)
{
    assert (asm_text);

    OffsetTableDtor (asm_text->offset_table);

    asm_text->offset = -1;

    free (asm_text->text);
    free (asm_text->tabs);
    free (asm_text);

    return FUNC_SUCCESS;
}

// ================================================================================================
// unsafe (buffer overflow)
DefaultFuncRes AsmTextAddTab (AsmText* asm_text)
{
    assert (asm_text);

    if (asm_text->tabs[0] == 0)
    {
        asm_text->tabs[0] = '\t';

        return FUNC_SUCCESS;
    }

    char* last_tab = strrchr (asm_text->tabs, '\t');
    last_tab++;
    *last_tab = '\t';

    return FUNC_SUCCESS;
}

// ================================================================================================
// unsafe (buffer overflow)
DefaultFuncRes AsmTextRemoveTab (AsmText* asm_text)
{
    assert (asm_text);

    char* last_tab = strrchr (asm_text->tabs, '\t');
    *last_tab = 0;

    return FUNC_SUCCESS;
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

DefaultFuncRes WriteCondition (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char* comparator)
{
    assert (op_node);
    assert (asm_text);
    assert (nametable);
    assert (comparator);

    WRITE ("; CONDITION_%ld\n", asm_text->cond_count);
    TranslateASTSubtree (op_node->left, asm_text, nametable);
    TranslateASTSubtree (op_node->right, asm_text, nametable);

    AsmTextAddTab (asm_text);

    WRITE ("push    0\n");
    WRITE ("pop     rax\n");
    WRITE ("%s     end_condition_%ld\n", comparator, asm_text->cond_count);
    WRITE ("push    1\n");
    WRITE ("pop     rax\n");

    AsmTextRemoveTab (asm_text);

    WRITE ("end_condition_%ld:\n", asm_text->cond_count);
    WRITE ("push    rax\n");

    asm_text->cond_count++;

    return FUNC_SUCCESS;
}

// ================================================================================================

OffsetTable* OffsetTableCtor ()
{
    OffsetTable* offset_table = (OffsetTable*) calloc (1, sizeof (OffsetTable));
    offset_table->ram_tables  = (RamTable*)    calloc (MAX_SCOPE_DEPTH, sizeof (RamTable));
    offset_table->curr_table_index = 0;

    for (size_t i = 0; i < MAX_SCOPE_DEPTH; i++)
    {
        for (size_t j = 0; j < NAMETABLE_CAPACITY; j++)
        {
            offset_table->ram_tables[i].index_in_ram[j] = -1;
        }

        offset_table->ram_tables[i].free_ram_index = 0;
    }

    return offset_table;
}

// ================================================================================================

DefaultFuncRes OffsetTableDtor (OffsetTable* offset_table)
{
    assert (offset_table);

    offset_table->curr_table_index = 0;
    free (offset_table->ram_tables);
    free (offset_table);

    return FUNC_SUCCESS;
}

// ================================================================================================

DefaultFuncRes OffsetTableAddFrame (OffsetTable* offset_table)
{
    assert (offset_table);

    if (offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    {
        offset_table->curr_table_index++;

        return FUNC_SUCCESS;
    }

    return FUNC_ERROR;
}

// ================================================================================================

DefaultFuncRes OffsetTableDeleteFrame (OffsetTable* offset_table)
{
    assert (offset_table);

    if (0 < offset_table->curr_table_index &&
            offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    {
        for (size_t j = 0; j < NAMETABLE_CAPACITY; j++)
        {
            offset_table->ram_tables[offset_table->curr_table_index].index_in_ram[j] = -1;
        }

        offset_table->ram_tables[offset_table->curr_table_index].free_ram_index = 0;

        offset_table->curr_table_index--;

        return FUNC_SUCCESS;
    }

    return FUNC_ERROR;
}

// ================================================================================================

DefaultFuncRes OffsetTableAddVariable (OffsetTable* offset_table, size_t var_id)
{
    assert (offset_table);

    CURR_RAM_TABLE.index_in_ram[CURR_RAM_TABLE.free_ram_index] = var_id;
    CURR_RAM_TABLE.free_ram_index++;

    return FUNC_SUCCESS;
}

// ================================================================================================

int OffsetTableGetVarOffset (OffsetTable* offset_table, size_t var_id)
{
    assert (offset_table);
    assert (var_id < NAMETABLE_CAPACITY);

    for (size_t i = 0; i < sizeof (CURR_RAM_TABLE.index_in_ram); i++)
    {
        if (CURR_RAM_TABLE.index_in_ram[i] == (int) var_id)
            return i;
    }

    return -1; // return code - not found
}

// ================================================================================================

DefaultFuncRes OffsetTableAddFuncParams (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable)
{
    assert (offset_table);
    assert (func_id_node);

    for (int i = 0; i < nametable->n_params[VAL (func_id_node)]; i++)
        OffsetTableAddVariable (offset_table, nametable->params[VAL (func_id_node)][i]);

    return FUNC_SUCCESS;
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

DefaultFuncRes OffsetTableDump (const OffsetTable* offset_table, const NameTable* nametable)
{
    assert (offset_table);

    for (size_t curr_table_id = 0; curr_table_id < offset_table->curr_table_index + 1; curr_table_id++)
    {
        printf ("============================================ table %ld ============================================\n", curr_table_id);

        for (size_t i = 0; i < NAMETABLE_CAPACITY; i++)
            printf ("%6ld ", i);

        printf ("\n");

        for (size_t i = 0; i < NAMETABLE_CAPACITY; i++)
            printf ("%6d ", offset_table->ram_tables[curr_table_id].index_in_ram[i]);

        printf ("\n");

        if (nametable)
            for (size_t i = 0; i < NAMETABLE_CAPACITY; i++)
                if (offset_table->ram_tables[curr_table_id].index_in_ram[i] != -1)
                    printf ("%6.6s ", nametable->names[offset_table->ram_tables[curr_table_id].index_in_ram[i]]);

        printf ("\n\n");
    }

    return FUNC_SUCCESS;
}

