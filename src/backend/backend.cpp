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

    printf ("%s\n", asm_text->text);

    AsmTextDtor (asm_text);

    return 0;
}

// ============================================================================================

AsmText* TranslateAST (const Tree* ast)
{
    assert (ast);

    // TODO  move this block outside the func (to main maybe)
    AsmText* asm_text = AsmTextCtor();

    WRITE ("; this program was written in tinkov language, mne poxyi ya v americu\n\n%n");

    WRITE ("push 1 ; default main() parameter like argc\n"
           "call function_%d ; calling main function\n%n", ast->nametable->main_index);
    WRITE ("out\n%n");
    WRITE ("hlt\n\n\n%n");

    TranslateASTSubtree (ast->root, asm_text, ast);

    return asm_text;
}

// ============================================================================================

TranslateRes TranslateASTSubtree (const TreeNode* node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;

    if (!node)
    {
        WARN ("[%d] node (nil)", asm_text->offset);

        return TRANSLATE_SUCCESS;
    }

    if (TranslateDeclarator (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateKeyword (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateSeparator (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateOperator (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateIdentifier (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    if (TranslateNumber (node, asm_text, ast) == TRANSLATE_SUCCESS)
        return TRANSLATE_SUCCESS;

    ERROR ("Couldnt translate node:\n"
           "[type = %d]\n"
           "[val  = %d]\n", TYPE (node), VAL (node));

    return TRANSLATE_ERROR;
}

// ============================================================================================

TranslateRes TranslateDeclarator (const TreeNode* declr_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (declr_node) != DECLARATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    switch (VAL (declr_node))
    {
        case VAR_DECLARATOR:
        {
            if (!declr_node->left && !declr_node->right)
                RET_ERROR (TRANSLATE_ERROR, "Declarator node has no children");

            // AddIdToRAM (VAL (declr_node->left->left), asm_text);

            return TranslateASTSubtree (declr_node->left, asm_text, ast);
        }

        case FUNC_DECLARATOR:
        {
            if (!declr_node->left || !declr_node->right)
                RET_ERROR (TRANSLATE_ERROR, "Func declarator node has no children");

            WRITE ("%sfunction_%d:\n%n", TABS, VAL (declr_node->right->right));
            AsmTextAddTab (asm_text);

            WRITE ("%s; parameter\n%n", TABS);

            AddIdToRAM (VAL (declr_node->right->left->right), asm_text);

            WRITE ("%spop [%d]\n\n%n", TABS,
                            IN_RAM (declr_node->right->left->right));

            WRITE ("%s; body\n%n", TABS);
            TranslateRes ret_val = TranslateASTSubtree (declr_node->left, asm_text, ast);

            AsmTextRemoveTab (asm_text);

            return ret_val;
        }

        default:
            return TRANSLATE_ERROR;
    }

    return TRANSLATE_ERROR;
}

// ============================================================================================

TranslateRes TranslateKeyword (const TreeNode* kw_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;

    if (TYPE (kw_node) != KEYWORD)
        return TRANSLATE_TYPE_NOT_MATCH;

    switch (VAL (kw_node))
    {
        case KW_RETURN:
        {
            if (TYPE (kw_node->left) == INT_LITERAL)
            {
                WRITE ("%spush %d\n%n", TABS, VAL (kw_node->left));
            }
            else
            {
                WRITE ("%spush [%d]\n%n", TABS, IN_RAM (kw_node->left));
            }

            WRITE ("%sret\n\n%n", TABS);
            break;
        }

        case KW_IF:
        {
            // condition
            TranslateASTSubtree (kw_node->right, asm_text, ast);
            WRITE ("%spush 0\n"
                   "%sjne if_%d\n"
                   "%sjmp else_%d\n%n",
                    TABS, TABS, asm_text->if_statements_count,
                    TABS, asm_text->if_statements_count )

            // if
            WRITE ("%sif_%d:\n%n", TABS, asm_text->if_statements_count );

            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->left->left, asm_text, ast);

            WRITE ("%sjmp end_if_%d\n%n",
                    TABS, asm_text->if_statements_count );

            AsmTextRemoveTab (asm_text);

            // else
            WRITE ("%selse_%d:\n%n", TABS, asm_text->if_statements_count );

            AsmTextAddTab (asm_text);

            if (kw_node->left->right)
                TranslateASTSubtree (kw_node->left->right, asm_text, ast);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_if_%d:\n\n%n", TABS, asm_text->if_statements_count);

            asm_text->if_statements_count++;
            break;
        }

        case KW_WHILE:
        {
            WRITE ("%swhile_%d:\n%n", TABS, asm_text->while_statements_count );
            AsmTextAddTab (asm_text);

            TranslateASTSubtree (kw_node->right, asm_text, ast);
            WRITE ("%s; CONDITION\n%n", TABS);
            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%sje end_while_%d\n\n%n", TABS, asm_text->while_statements_count);

            TranslateASTSubtree (kw_node->left, asm_text, ast);
            WRITE ("%sjmp while_%d\n%n", TABS, asm_text->while_statements_count );

            AsmTextRemoveTab (asm_text);
            WRITE ("%send_while_%d:\n\n%n", TABS, asm_text->while_statements_count );

            asm_text->while_statements_count++;

            break;
        }

        default:
            return TRANSLATE_ERROR;
    }

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

TranslateRes TranslateSeparator (const TreeNode* sep_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (sep_node) != SEPARATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (VAL (sep_node) == END_STATEMENT)
    {
        if (sep_node->left)
            if (TranslateASTSubtree (sep_node->left, asm_text, ast) != TRANSLATE_SUCCESS)
                return TRANSLATE_ERROR;

        if (sep_node->right)
            return TranslateASTSubtree (sep_node->right, asm_text, ast);

        return TRANSLATE_ERROR;
    }

    else if (VAL (sep_node) == ENCLOSE_STATEMENT_BEGIN)
    {
        if (sep_node->left)
            return TranslateASTSubtree (sep_node->left, asm_text, ast);

        return TRANSLATE_ERROR;
    }

    return TRANSLATE_ERROR;
}

// ============================================================================================

TranslateRes TranslateOperator (const TreeNode* op_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (op_node) != OPERATOR)
        return TRANSLATE_TYPE_NOT_MATCH;

    switch (VAL (op_node))
    {
        case ASSIGN:
        {
            TranslateASTSubtree (op_node->right, asm_text, ast);

            AddIdToRAM (VAL (op_node->left), asm_text);

            WRITE ("%spop [%d]             ; assign var_%d\n%n", TABS, IN_RAM (op_node->left), VAL (op_node->left));

            break;
        }

        case ADD:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);

            WRITE ("%sadd\n%n", TABS);

            break;
        }

        case SUB:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);

            WRITE ("%ssub\n%n", TABS);

            break;
        }

        case MUL:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);

            WRITE ("%smul\n%n", TABS);

            break;
        }

        case DIV:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);

            WRITE ("%sdiv\n%n", TABS);

            break;
        }

        case LESS:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sjae end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        case LESS_EQ:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sja end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        case EQUAL:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sjne end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        case MORE_EQ:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sjb end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        case MORE:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sjbe end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        case UNEQUAL:
        {
            TranslateASTSubtree (op_node->left, asm_text, ast);
            TranslateASTSubtree (op_node->right, asm_text, ast);
            WRITE ("%s; CONDITION_%d\n%n", TABS, asm_text->cond_count);
            AsmTextAddTab (asm_text);

            WRITE ("%spush 0\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);
            WRITE ("%sje end_condition_%d\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush 1\n%n", TABS);
            WRITE ("%spop rax\n%n", TABS);

            AsmTextRemoveTab (asm_text);

            WRITE ("%send_condition_%d:\n%n", TABS, asm_text->cond_count);
            WRITE ("%spush rax\n%n", TABS);
            asm_text->cond_count++;

            break;
        }

        default:
        {
            ERROR ("Operator %d not supported, be smarter please", VAL (op_node));

            return TRANSLATE_ERROR;
        }
    }

    WRITE ("\n%n");

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

TranslateRes TranslateIdentifier (const TreeNode* id_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (id_node) != IDENTIFIER)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (!id_node->left)
    {
        if (IN_RAM (id_node) == -1)
            AddIdToRAM (VAL (id_node), asm_text);

        WRITE ("%spush [%d]            ; get value of var_%d\n%n", TABS, IN_RAM (id_node), VAL (id_node));

        return TRANSLATE_SUCCESS;
    }

    // function call
    WRITE ("%s; function call\n%n", TABS);
    WRITE ("%spush [%d]\n%n", TABS, IN_RAM (id_node->left->right));
    WRITE ("%scall function_%d\n\n%n", TABS, VAL (id_node));

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

TranslateRes TranslateNumber (const TreeNode* num_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (num_node) != INT_LITERAL)
        return TRANSLATE_TYPE_NOT_MATCH;

    WRITE ("%spush %d\n%n", TABS, VAL (num_node) );

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

int AddIdToRAM (int identifier_index, AsmText* asm_text)
{
    /**
     * | id_1 | id_2 | id_3 |
     * ----------------------
     * | rm_2 | rm_1 |  -1  |
    */

    assert (-1 < identifier_index);
    assert (asm_text);

    if (asm_text->ram_table.index_in_ram[identifier_index] == -1)
        asm_text->ram_table.index_in_ram[identifier_index] = asm_text->ram_table.free_ram_index;

    return asm_text->ram_table.free_ram_index++;
}

// ============================================================================================

AsmText* AsmTextCtor ()
{
    AsmText* asm_text = (AsmText*) calloc (1, sizeof (AsmText));

    asm_text->text = (char*) calloc (MAX_ASM_PROGRAM_SIZE, sizeof (char));
    asm_text->offset = 0;

    for (int i = 0; i < NAMETABLE_CAPACITY; i++)
        asm_text->ram_table.index_in_ram[i] = -1;

    asm_text->ram_table.free_ram_index = 0;

    asm_text->if_statements_count    = 0;
    asm_text->while_statements_count = 0;
    asm_text->funcs_count            = 0;
    asm_text->cond_count             = 0;
    asm_text->tabs  = (char*) calloc (MAX_SCOPE_DEPTH, sizeof (char));

    return asm_text;
}

// ============================================================================================

int AsmTextDtor (AsmText* asm_text)
{
    assert (asm_text);

    asm_text->offset = -1;

    free (asm_text->text);
    free (asm_text->tabs);
    free (asm_text);

    return 0;
}

// ============================================================================================
// unsafe (buffer overflows)
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

// ============================================================================================
// unsafe (buffer overflow)
int AsmTextRemoveTab (AsmText* asm_text)
{
    assert (asm_text);

    char* last_tab = strrchr (asm_text->tabs, '\t');
    *last_tab = 0;

    return 0;
}

// ============================================================================================

int ConvertFuncNameToFuncId (char* func_name)
{
    assert (func_name);

    int res = 0;

    while (*func_name)
        res += *func_name++;

    return res;
}
