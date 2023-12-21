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

    if (VAL (declr_node) == VAR_DECLARATOR)
    {
        if (!declr_node->left && !declr_node->right)
            RET_ERROR (TRANSLATE_ERROR, "Declarator node has no children");

        // AddIdToRAM (VAL (declr_node->left->left), asm_text);

        return TranslateASTSubtree (declr_node->left, asm_text, ast);
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
    case KW_IF:
    {
        int written = 0;

        // condition
        TranslateASTSubtree (kw_node->right, asm_text, ast);
        sprintf (TEXT, "push 0\n"
                       "jne if_statement_%d\n"
                       "jmp else_statement_%d\n%n",
                asm_text->if_statements_count, asm_text->if_statements_count, &written);
        asm_text->offset += written;

        // if
        sprintf (TEXT, "if_statement_%d:\n%n", asm_text->if_statements_count, &written);
        asm_text->offset += written;

        TranslateASTSubtree (kw_node->left->left, asm_text, ast);

        sprintf (TEXT, "jmp end_if_statement_%d\n%n", asm_text->if_statements_count, &written);
        asm_text->offset += written;

        // else
        sprintf (TEXT, "else_statement_%d:\n%n", asm_text->if_statements_count, &written);
        asm_text->offset += written;

        if (kw_node->left->right)
            TranslateASTSubtree (kw_node->left->right, asm_text, ast);

        sprintf (TEXT, "end_if_statement_%d:\n%n", asm_text->if_statements_count, &written);
        asm_text->offset += written;

        asm_text->if_statements_count++;
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

    if (VAL (op_node) == ASSIGN)
    {
        TranslateASTSubtree (op_node->right, asm_text, ast);

        AddIdToRAM (VAL (op_node->left), asm_text);

        int written = 0;
        sprintf (TEXT, "pop [%d] ; assign var_%d\n%n", IN_RAM (op_node->left), VAL (op_node->left), &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == ADD)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "add\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == SUB)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "sub\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == MUL)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "mul\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == DIV)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "div\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == LESS)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "jb True             ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == LESS_EQ)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "jbe True            ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == EQUAL)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "je True             ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == MORE_EQ)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "jae True            ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == MORE)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "ja True             ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else if (VAL (op_node) == UNEQUAL)
    {
        TranslateASTSubtree (op_node->left, asm_text, ast);
        TranslateASTSubtree (op_node->right, asm_text, ast);

        int written = 0;
        sprintf (TEXT, "jmp False           ; set rax to 0\n"
                       "jne True            ; if jmp - set rax to 1\n"
                       "push rax            ; get true or false\n%n", &written);
        asm_text->offset += written;
    }

    else
    {
        ERROR ("Operator %d not supported, be smarter please", VAL (op_node));

        return TRANSLATE_ERROR;
    }

    int written = 0;
    sprintf (TEXT, "\n%n", &written);
    asm_text->offset += written;

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

TranslateRes TranslateIdentifier (const TreeNode* id_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (id_node) != IDENTIFIER)
        return TRANSLATE_TYPE_NOT_MATCH;

    if (IN_RAM (id_node) == -1)
        AddIdToRAM (VAL (id_node), asm_text);

    int written = 0;
    sprintf (TEXT, "push [%d] ; get value of var_%d\n%n", IN_RAM (id_node), VAL (id_node), &written);
    asm_text->offset += written;

    return TRANSLATE_SUCCESS;
}

// ============================================================================================

TranslateRes TranslateNumber (const TreeNode* num_node, AsmText* asm_text, const Tree* ast)
{
    ASSERT_BLOCK_FUNC_TRANSLATE;
    if (TYPE (num_node) != INT_LITERAL)
        return TRANSLATE_TYPE_NOT_MATCH;

    int written = 0;
    sprintf (TEXT, "push %d\n%n", VAL (num_node), &written);

    asm_text->offset += written;

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

    return asm_text;
}

// ============================================================================================

int AsmTextDtor (AsmText* asm_text)
{
    assert (asm_text);

    asm_text->offset = -1;

    free (asm_text->text);
    free (asm_text);

    return 0;
}
