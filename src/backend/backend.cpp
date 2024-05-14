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
    if (!ast) RET_ERROR (1, "Could not read AST");

    TreeDotDump ("dump.html", ast);

    AsmText* asm_text = AsmTextCtor ();

    TranslateAST (ast->root, asm_text, ast->nametable);
    TreeDtor (ast);

    FILE* output_file = fopen ("out.s", "wb");
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
    // todo
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

            // OffsetTableAddVariable (OFFSET_TABLE, VAL (var_id_node)); ATTENTION !!!
            if (!OffsetTableGetVarOffset (OFFSET_TABLE, VAL (var_id_node)))
            {
                PRINTF_DEBUG ("%s not found in offset_table", NAME (var_id_node));
                return TRANSLATE_DECLR_ERROR;
            }

            TranslateASTSubtree (declr_node->left, asm_text, nametable);

            return TRANSLATE_SUCCESS;
        }

        case FUNC_DECLARATOR:
        {
            const TreeNode* func_id_node = declr_node->right->right;

            OffsetTableAddFrame      (OFFSET_TABLE, N_PARAMS (func_id_node));
            OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);
            OffsetTableAddFuncLocals (OFFSET_TABLE, declr_node->left, nametable);

            WRITE ("%s:\t\t; function (id = %d, n_params = %d)\n",
                            NAME     (func_id_node),
                            VAL      (func_id_node),
                            N_PARAMS (func_id_node));

            // TreeNode *param_node = declr_node->right->left;
            // for (size_t n_param = 0; n_param < N_PARAMS (func_id_node); n_param++)
            // {
                // int eff_offset = OffsetTableGetVarOffset (OFFSET_TABLE, VAL (param_node->right));
                // WRITE ("\t\t; param int64_t %s @ rbp+0x%x\n",
                        // NAME (param_node->right),
                        // (unsigned) (-eff_offset));
                // param_node = param_node->left;
            // }

            DescribeCurrFunction (asm_text, nametable);

            AsmTextAddTab (asm_text);

            PUSH ("rbp");
            MOV ("rbp", "rsp");

            TranslateASTSubtree (declr_node->left, asm_text, nametable);

            POP ("rbp");
            RET;

            AsmTextRemoveTab (asm_text);

            OffsetTableDeleteFrame (OFFSET_TABLE);

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
            // todo spu -> nasm
            WRITE ("in ; >> \"%s\"\n",
                NAME (kw_node->right));
            WRITE ("pop [rpx + %d]\n",
                OffsetTableGetVarOffset (OFFSET_TABLE, VAL (kw_node->right)));

            break;
        }

        case KW_PRINT:
        {
            // todo spu -> nasm
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            WRITE ("out\n");

            break;
        }

        case KW_RETURN:
        {
            // todo spu -> nasm
            TranslateASTSubtree (kw_node->left, asm_text, nametable);

            // MOV ("rax", ???);

            WRITE ("ret\n\n");
            break;
        }

        case KW_IF:
        {
            WRITE ("push rax\n");

            // condition
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            CPOP ("rax");
            WRITE ("cmp rax, 0\n");     // rax = condition result (1 or 0)
            WRITE ("je else#%ld\n", IF_COUNT);

            // if - true:

            TranslateASTSubtree (kw_node->left->left, asm_text, nametable);

            WRITE ("jmp end_if#%ld\n", IF_COUNT);

            // else
            WRITE_NO_TAB ("else#%ld:\n", IF_COUNT);

            if (kw_node->left->right)
                TranslateASTSubtree (kw_node->left->right, asm_text, nametable);

            WRITE_NO_TAB ("end_if#%ld:\n\n", IF_COUNT);

            WRITE ("pop rax\n");

            IF_COUNT++;

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
            PUSH ("rax");

            WRITE ("jmp while_cond#%ld\n", WHILE_COUNT);

            WRITE_NO_TAB ("while#%ld:\n", WHILE_COUNT);

            TranslateASTSubtree (kw_node->left, asm_text, nametable);

            WRITE_NO_TAB ("while_cond#%ld:\n", WHILE_COUNT);
            TranslateASTSubtree (kw_node->right, asm_text, nametable); // cstack top = condition result
            CPOP ("rax");                                              // rax = condition result (1 or 0)
            WRITE ("cmp rax, 0\n");
            WRITE ("jne while#%ld", WHILE_COUNT);

            POP ("rax");

            WHILE_COUNT++;

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

//* change spu->nasm
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
            PUSH ("rax");

            // rax = rvalue
            TranslateASTSubtree (op_node->right, asm_text, nametable);
            CPOP ("rax");

            // mov ... <- destination operand is a variable/parameter (mem only)
            WRITE ("mov ");

            int effective_offset = OffsetTableGetVarOffset (asm_text->offset_table, VAL (op_node->left));

            // mov [rbp - 8 * -1], ... <- source operand is a variable/parameter/number ALWAYS in RAX
            WRITE ("[rbp - 8 * %d], rax", effective_offset);

            WRITE ("\t; %s#%d = rax\n", (effective_offset > 0) ? "var" : "par", effective_offset);

            POP ("rax");

            break;
        }

        case ADD:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15]\n");
            WRITE ("add rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8              ; push addition result instead of 2 operands\n");
            WRITE ("mov [r15], rax\n");
            POP ("rax");

            break;
        }

        case SUB:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15]\n");
            WRITE ("sub rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8              ; push substitution result instead of 2 operands\n");
            WRITE ("mov [r15], rax\n");
            POP ("rax");

            break;
        }

        case MUL:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15]\n");
            WRITE ("mul rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8              ; push multiplication result instead of 2 operands\n");
            WRITE ("mov [r15], rax\n");
            POP ("rax");

            break;
        }

        case DIV:
        {
            //* not accurate
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15]\n");
            WRITE ("div rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8              ; push division result instead of 2 operands\n");
            WRITE ("mov [r15], rax\n");
            POP ("rax");

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
            TranslateCondition (op_node, asm_text, nametable, "jb");

            break;
        }

        case LESS_EQ:
        {
            TranslateCondition (op_node, asm_text, nametable, "jle");

            break;
        }

        case EQUAL:
        {
            TranslateCondition (op_node, asm_text, nametable, "je");

            break;
        }

        case MORE_EQ:
        {
            TranslateCondition (op_node, asm_text, nametable, "jae");

            break;
        }

        case MORE:
        {
            TranslateCondition (op_node, asm_text, nametable, "ja");

            break;
        }

        case UNEQUAL:
        {
            TranslateCondition (op_node, asm_text, nametable, "jne");

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
//* spu->nasm...
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

        #ifdef FCALL_MEMOIZATION

            // todo

        #else


        #endif // FCALL_MEMOIZATION

        // move function call result to new local variable

        return TRANSLATE_SUCCESS;
    }
    else
    {
        // variable

        PUSH ("rax");

        WRITE ("mov rax, "); // mov rax, ... <- here goes the second operand reg/mem

        int reg_id = VAR_NOT_IN_REGISTER; // = GetVarRegId (id_node); - not imlemented yet

        if (reg_id == VAR_NOT_IN_REGISTER)
        {
            // parameter or local variable

            // when function is called all its parameters are put into offset table

            int effective_offset = OffsetTableGetVarOffset (OFFSET_TABLE, VAL (id_node));

            WRITE ("QWORD [rbp + 8*%d]\n", effective_offset);
        }
        else
        {
            WRITE ("%s\n", REG_NAMES[reg_id]);
        }

        CPUSH ("rax");

        POP ("rax");

        return TRANSLATE_SUCCESS;
    }

    return TRANSLATE_ERROR;
}

// ================================================================================================

//* spu->nasm
TranslateRes TranslateNumber (const TreeNode* num_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (asm_text);
    assert (asm_text->text);
    assert (nametable);

    if (TYPE (num_node) != INT_LITERAL)
        return TRANSLATE_TYPE_NOT_MATCH;

    CPUSH ("%d", VAL (num_node));

    return TRANSLATE_SUCCESS;
}

// ================================================================================================
/**
 ** LEGACY
*/
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

    OffsetTableAddFrame      (OFFSET_TABLE, N_PARAMS (func_id_node));
    OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);

    PopParamsToRAM (func_id_node, asm_text, nametable);
    WRITE ("push    rax ; recover previous rpx from rax\n");

    return FUNC_SUCCESS;
}

// ================================================================================================
/**
 ** LEGACY
*/
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
/**
 ** LEGACY
*/
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
/**
 ** LEGACY
*/
DefaultFuncRes WriteCondition (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, OperatorCode comparator)
{
    assert (op_node);
    assert (asm_text);
    assert (nametable);
    assert (comparator);

    WRITE ("\t\t; cond#%ld\n", asm_text->cond_count);

    asm_text->cond_count++;

    return FUNC_SUCCESS;
}

DefaultFuncRes TranslateCondition (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char *jmp_comparator)
{
    assert (op_node);
    assert (asm_text);
    assert (nametable);
    assert (jmp_comparator);

    WRITE ("\t\t; cond#%ld\n", COND_COUNT);

    WRITE ("push rax\n");

    TranslateASTSubtree (op_node->left, asm_text, nametable);   // cpush
    TranslateASTSubtree (op_node->right, asm_text, nametable);  // cpush

    WRITE ("cmp [r15], [r15 + 8]\n");
    WRITE ("%s cpush_true#%ld\n", jmp_comparator, COND_COUNT);

    WRITE_NO_TAB ("cpush_false#%ld:\n", COND_COUNT);
    WRITE ("mov rax, 0\n");

    WRITE ("jmp end_cond#%ld\n", COND_COUNT);

    WRITE_NO_TAB ("cpush_true#%ld:\n", COND_COUNT);
    WRITE ("mov rax, 1\n");

    WRITE_NO_TAB ("end_cond#%ld:\n", COND_COUNT);
    CPUSH ("rax");

    WRITE ("pop rax\n");

    COND_COUNT++;

    return FUNC_SUCCESS;
}

// ================================================================================================

OffsetTable* OffsetTableCtor ()
{
    OffsetTable* offset_table = (OffsetTable*) calloc (1, sizeof (OffsetTable));
    offset_table->ram_tables  = (RamTable*)    calloc (MAX_SCOPE_DEPTH, sizeof (RamTable));
    offset_table->curr_table_index = 0;

    offset_table->n_params          = 0;
    offset_table->offset_table      = (int*) calloc (OFFSET_TABLE_CAPACITY, sizeof (int));
    offset_table->offset_table_base = 0;
    offset_table->offset_table_ptr  = 0;

    offset_table->base_stack = StackLightCtor (MAX_SCOPE_DEPTH);

    for (size_t i = 0; i < MAX_SCOPE_DEPTH; i++)
    {
        for (size_t j = 0; j < OFFSET_TABLE_CAPACITY; j++)
        {
            offset_table->ram_tables[i].index_in_ram[j] = -1;
        }

        offset_table->ram_tables[i].free_ram_index = 0;
        offset_table->ram_tables[i].n_params       = 0;
    }

    return offset_table;
}

// ================================================================================================

DefaultFuncRes OffsetTableDtor (OffsetTable* offset_table)
{
    assert (offset_table);

    offset_table->curr_table_index = 0;
    StackLightDtor (offset_table->base_stack);
    free (offset_table->ram_tables);
    free (offset_table->offset_table);
    free (offset_table);

    return FUNC_SUCCESS;
}

// ================================================================================================

DefaultFuncRes OffsetTableAddFrame (OffsetTable* offset_table, size_t n_params)
{
    assert (offset_table);

    // if (offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    // {
    //     offset_table->curr_table_index++;
    //     CURR_RAM_TABLE.n_params = n_params;

    //     return FUNC_SUCCESS;
    // }

    // better alternative:
    if (offset_table->offset_table_ptr < MAX_SCOPE_DEPTH)
    {
        StackLightPush (offset_table->base_stack, offset_table->offset_table_base);

        offset_table->offset_table_base = offset_table->offset_table_ptr;

        return FUNC_SUCCESS;
    }

    return FUNC_ERROR;
}

// ================================================================================================

DefaultFuncRes OffsetTableDeleteFrame (OffsetTable* offset_table)
{
    assert (offset_table);

    // if (0 < offset_table->curr_table_index &&
    //         offset_table->curr_table_index < MAX_SCOPE_DEPTH)
    // {
    //     for (size_t j = 0; j < NAMETABLE_CAPACITY; j++)
    //     {
    //         offset_table->ram_tables[offset_table->curr_table_index].index_in_ram[j] = -1;
    //     }

    //     offset_table->ram_tables[offset_table->curr_table_index].free_ram_index = 0;

    //     offset_table->curr_table_index--;

    //     return FUNC_SUCCESS;
    // }

    // better alternative
    if (0 < offset_table->offset_table_ptr &&
            offset_table->offset_table_ptr < MAX_SCOPE_DEPTH)
    {
        for (size_t i = offset_table->offset_table_base; i < offset_table->offset_table_ptr; i++)
            offset_table->offset_table[i] = 0;

        offset_table->offset_table_ptr = offset_table->offset_table_base;
        if (offset_table->offset_table_base)
            offset_table->offset_table_base = StackLightPop (offset_table->base_stack);

        return FUNC_SUCCESS;
    }

    return FUNC_ERROR;
}

// ================================================================================================

DefaultFuncRes OffsetTableAddVariable (OffsetTable* offset_table, size_t var_id)
{
    assert (offset_table);

    // CURR_RAM_TABLE.index_in_ram[CURR_RAM_TABLE.free_ram_index] = var_id;
    // CURR_RAM_TABLE.free_ram_index++;

    // better alternative
    offset_table->offset_table[offset_table->offset_table_ptr++] = var_id;

    return FUNC_SUCCESS;
}

// ================================================================================================

/**************************************************************************************************
 * @brief get effective variable offset within its scope
 *
 * @param [offset_table] offset table
 * @param [var_id] id of variable/parameter to look for
 *
 * @details parameters go first, followed by local variables.
 *          Parameter eff_offset = index - n_params
 *          Loc.var.  eff_offset = index - n_params + 1 // to avoid addressing [rbp + 0]
 *
 * @return 0 - not found
**************************************************************************************************/
int OffsetTableGetVarOffset (OffsetTable* offset_table, size_t var_id)
{
    assert (offset_table);
    assert (var_id < NAMETABLE_CAPACITY);

    // int eff_offset = 0; // error code - not found

    // for (size_t i = 0; i < sizeof (CURR_RAM_TABLE.index_in_ram); i++)
    //     if (CURR_RAM_TABLE.index_in_ram[i] == (int) var_id)
    //     {
    //         eff_offset = i - CURR_RAM_TABLE.n_params;
    //         if (eff_offset >= 0) eff_offset++; // to avoid [rbp + 0],
    //                                            // eff_offset = 0 - is bad and means "val not found"
    //         break;
    //     }

    // better alternative
    int eff_offset = 0; // error code - not found

    for (size_t i = 0; i < offset_table->offset_table_ptr; i++)
    {
        if (offset_table->offset_table[i] == var_id)
        {
            eff_offset = i - offset_table->n_params;
            if (eff_offset >= 0) eff_offset++; // to avoid [rbp + 0],

            break;
        }
    }

    return eff_offset;
}

// ================================================================================================


/**************************************************************************************************
 * @brief
 *
 * @param [offset_table] offset table
 *
 *
 * @details
 *
 * @return
**************************************************************************************************/
int DescribeCurrFunction (AsmText *asm_text, const NameTable *nametable)
{
    assert (asm_text);
    assert (nametable);

    int eff_offset = -OFFSET_TABLE->n_params;

    for (size_t i = 0; i < OFFSET_TABLE->offset_table_ptr; i++)
    {
        if (eff_offset == 0) eff_offset++; // to avoid [rbp + 0],
                                           // eff_offset = 0 - is bad and means "val not found"
        if (eff_offset < 0)
        {
            WRITE ("\t\t; par int64_t %6s @ rbp+0x%x\n",
                        nametable->names[OFFSET_TABLE->offset_table[i]],
                        (unsigned) (-eff_offset));
        }
        else
        {
            WRITE ("\t\t; var int64_t %6s @ rbp-0x%x\n",
                        nametable->names[OFFSET_TABLE->offset_table[i]],
                        eff_offset);
        }

        eff_offset++; // grows along with i
    }

    return eff_offset;
}

// ================================================================================================

/**
 * @brief add each parameter of function with id node = func_id_node to offset table
*/
DefaultFuncRes OffsetTableAddFuncParams (OffsetTable* offset_table, const TreeNode* func_id_node, const NameTable* nametable)
{
    assert (offset_table);
    assert (func_id_node);

    offset_table->n_params = N_PARAMS (func_id_node);

    for (int i = 0; i < N_PARAMS (func_id_node); i++)
        OffsetTableAddVariable (offset_table, PARAMS (func_id_node, i));

    return FUNC_SUCCESS;
}

// ================================================================================================

/**
 * @brief add each local variable of function with body node = func_body_node
 *
 * @bug   scoping does not work, local variables are visible from outside of their scope
*/
DefaultFuncRes OffsetTableAddFuncLocals (OffsetTable* offset_table, const TreeNode* func_body_node, const NameTable* nametable)
{
    assert (offset_table);
    if (!func_body_node) return FUNC_SUCCESS;

    OffsetTableAddFuncLocals (offset_table, func_body_node->left, nametable);

    if (TYPE (func_body_node) == SEPARATOR && VAL (func_body_node) == ENCLOSE_STATEMENT_BEGIN)
        OffsetTableAddFrame (offset_table, offset_table->n_params);

    if (TYPE (func_body_node) == DECLARATOR && VAL (func_body_node) == VAR_DECLARATOR)
        OffsetTableAddVariable (offset_table, VAL (func_body_node->left->left));

    OffsetTableAddFuncLocals (offset_table, func_body_node->right, nametable);

    return FUNC_SUCCESS;
}

// ================================================================================================

int OffsetTableGetCurrFrameWidth (OffsetTable* offset_table)
{
    assert (offset_table);

    // int frame_width = 0;
//
    // while (CURR_RAM_TABLE.index_in_ram[frame_width] != -1)
        // frame_width++;

    // better alternative
    int frame_width = offset_table->offset_table_ptr - offset_table->offset_table_base;

    return frame_width;
}

// ================================================================================================
/**
 ** LEGACY
*/
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

