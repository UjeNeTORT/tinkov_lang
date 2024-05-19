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
    WRITE ("BITS 64;\n");

    WRITE ("INT_PRECISION_POW equ %u\n", INT_PRECISION_POW);
    WRITE ("INT_PRECISION equ ");
    if (INT_PRECISION_POW != 0)
    {
        WRITE_NO_TAB ("1");
        for (size_t i = 0; i < INT_PRECISION_POW; i++) WRITE_NO_TAB ("0");
    }
    else
    {
        WRITE_NO_TAB ("0");
    }
    WRITE_NO_TAB ("\n\n");

    WRITE ("SECTION .data\n\n");
    WRITE ("calc_stack times %ld db 0x00\t\t; calc stack (r15)\n\n", CALC_STACK_CAPACITY);

    WRITE ("%%include \"%s\"\n", DFLT_STDLIB_PATH);

    WRITE ("\nSECTION .text\n\n");

    WRITE ("; PROLOGUE\n");

    // todo stdlib, global, section text
    WRITE ("global main\n\n");
    WRITE ("main:\n\n");
    WRITE ("; ===== mapping of calc stack (CPUSH, CPOP) =====\n");
    MOV ("r15", "calc_stack");
    WRITE ("add r15, %ld\n", CALC_STACK_CAPACITY);
    WRITE ("; ===============================================\n");

    WRITE ("call %s\t\t; calling program entry point func\n\n",
                nametable->names[nametable->main_index]);

    WRITE ("; EPILOGUE\n");
    MOV ("rax", "1\t\t; syscall exit");
    XOR ("ebx", "ebx");
    WRITE ("int 0x80\t\t; syscall\n\n");

    WRITE ("ret\n");

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

            asm_text->funcs_count++;

            OFFSET_TABLE->n_params = N_PARAMS (func_id_node);
            OffsetTableAddFrame      (OFFSET_TABLE);
            OffsetTableAddFuncParams (OFFSET_TABLE, func_id_node, nametable);
            OffsetTableAddFuncLocals (OFFSET_TABLE, declr_node->left, nametable);

            WRITE ("%s:\t\t\t; function (id = %d, n_params = %d)\n",
                            NAME     (func_id_node),
                            VAL      (func_id_node),
                            N_PARAMS (func_id_node));

            DescribeCurrFunction (asm_text, nametable);

            size_t n_locals = OFFSET_TABLE->offset_table_ptr - N_PARAMS (func_id_node);

            AsmTextAddTab (asm_text);

            PUSH ("rbp");
            MOV ("rbp", "rsp");

            WRITE ("sub rsp, %ld\t\t\t; reserve space for locals\n", n_locals * QWORD_SIZE);

            TranslateASTSubtree (declr_node->left, asm_text, nametable);

            WRITE_NO_TAB ("func_end_%ld:\n", asm_text->funcs_count);
            WRITE ("add rsp, %ld\t\t\t; pop locals\n", n_locals * QWORD_SIZE);
            POP ("rbp");
            RET;
            WRITE_NEWLINE;

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
            // todo call my_std i/o func
            // todo test
            PUSH ("rax\t\t\t; begin SCAN imitation");
            MOV ("rax", "11");
            MOV ("QWORD [rbp - %d]", "rax", EFF_OFFSET (kw_node->right) * QWORD_SIZE);
            POP ("rax\t\t\t\t ; end  SCAN imitation");

            break;
        }

        case KW_PRINT:
        {
            // todo test
            WRITE_NEWLINE;

            switch (TYPE (kw_node->right))
            {
                case SEPARATOR:
                {
                    RET_ERROR (TRANSLATE_TYPE_NOT_MATCH, "Printing error: cannot print separator node");

                    break;
                }

                case INT_LITERAL:
                {
                    MOV ("rdi", "%d\t\t\t; PRINT immediate val (int64_t) begin\n",
                            VAL (kw_node->right));

                    break;
                }

                case IDENTIFIER:
                {
                    if (!IsFunction (kw_node->right, nametable)) // kw_node->right - var/par
                    {
                        MOV ("rdi", "[rbp - %d]\t\t; PRINT \"%s\" begin",
                                EFF_OFFSET (kw_node->right) * QWORD_SIZE,
                                NAME (kw_node->right));

                        break;
                    }
                    // if function - default case (compound statement)
                    // fall through
                    [[fallthrough]];
                }

                default:
                {
                    WRITE_NO_TAB ("\t\t\t\t\t; PRINT compound statement begin\n");
                    TranslateASTSubtree (kw_node->right, asm_text, nametable);  // cpush
                    CPOP ("rdi");

                    break;
                }
            }

            CALL ("print_int64_t\t\t; PRINT end");
            WRITE_NEWLINE;

            break;
        }

        case KW_RETURN:
        {
            TranslateASTSubtree (kw_node->left, asm_text, nametable);

            WRITE ("\t\t; rax = ret_val & return\n");
            CPOP ("rax");

            WRITE ("jmp func_end_%ld\t\t\t; rax = ret_val", asm_text->funcs_count);

            break;
        }

        case KW_IF:
        {
            // condition
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            CPOP ("rax");
            CMP ("rax", "0");   // rax = condition result (1 or 0)
            WRITE ("je else_%ld\n", IF_COUNT);

            // if - true:

            TranslateASTSubtree (kw_node->left->left, asm_text, nametable);

            WRITE ("jmp end_if_%ld\n", IF_COUNT);

            // else
            WRITE_NO_TAB ("else_%ld:\n", IF_COUNT);

            if (kw_node->left->right)
                TranslateASTSubtree (kw_node->left->right, asm_text, nametable);

            WRITE_NO_TAB ("end_if_%ld:\n\n", IF_COUNT);

            IF_COUNT++;

            break;
        }

        case KW_DO: // do if
        {
            // todo test
            // ignore condition (it is not even presented in ast)
            TranslateASTSubtree (kw_node->right, asm_text, nametable);

            break;
        }

        case KW_WHILE:
        {
            // todo test
            PUSH ("rax");

            WRITE ("jmp while_cond_%ld\n", WHILE_COUNT);

            WRITE_NO_TAB ("while_%ld:\n", WHILE_COUNT);

            TranslateASTSubtree (kw_node->left, asm_text, nametable);

            WRITE_NO_TAB ("while_cond_%ld:\n", WHILE_COUNT);
            TranslateASTSubtree (kw_node->right, asm_text, nametable); // cstack top = condition result
            CPOP ("rax");                                              // rax = condition result (1 or 0)
            WRITE ("cmp rax, 0\n");
            WRITE ("jne while_%ld", WHILE_COUNT);

            POP ("rax");

            WHILE_COUNT++;

            break;
        }

        default:
            return TRANSLATE_ERROR;
    }

    WRITE_NEWLINE;

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
            // rax = rvalue
            TranslateASTSubtree (op_node->right, asm_text, nametable);
            CPOP ("rax");

            // mov ... <- destination operand is a variable/parameter (mem only)
            WRITE ("mov ");

            // mov [rbp - 8 * -1], ... <- source operand is a variable/parameter/number ALWAYS in RAX
            WRITE_NO_TAB ("QWORD [rbp - %d], rax", EFF_OFFSET (op_node->left) * QWORD_SIZE);

            WRITE_NO_TAB ("\t; %s = rax\n", NAME (op_node->left));

            break;
        }

        case ADD:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15]\n");
            WRITE ("add rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8\t\t\t; push addition result instead of 2 operands\n");
            WRITE ("mov [r15], rax\n");
            POP ("rax");

            break;
        }

        case SUB:
        {
            TranslateASTSubtree (op_node->left, asm_text, nametable);
            TranslateASTSubtree (op_node->right, asm_text, nametable);

            PUSH ("rax");
            WRITE ("mov rax, QWORD [r15+8]\n");
            WRITE ("sub rax, QWORD [r15]\n");

            WRITE ("add r15, 8\t\t\t; push substitution result instead of 2 operands\n");
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
            WRITE ("imul rax, QWORD [r15+8]\n");

            WRITE ("add r15, 8\t\t\t; push multiplication result instead of 2 operands\n");
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
            WRITE ("mov rax, QWORD [r15+8]\n");
            WRITE ("idiv rax, QWORD [r15]\n");

            WRITE ("add r15, 8\t\t\t; push division result instead of 2 operands\n");
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

    WRITE_NEWLINE;

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

        TranslateFunctionCall (id_node, asm_text, nametable);

        #endif // FCALL_MEMOIZATION

        // todo move function call result to new local variable

        return TRANSLATE_SUCCESS;
    }
    else
    {
        // variable

        // PUSH ("rax");

        WRITE ("mov rax, "); // mov rax, ... <- here goes the second operand reg/mem

        int reg_id = VAR_NOT_IN_REGISTER; // = GetVarRegId (id_node); - not implemented yet

        if (reg_id == VAR_NOT_IN_REGISTER)
        {
            // parameter or local variable

            WRITE_NO_TAB ("QWORD [rbp - %d]\t; rax = %s\n", EFF_OFFSET (id_node) * QWORD_SIZE, NAME (id_node));
        }
        else
        {
            WRITE_NO_TAB ("%s\n", REG_NAMES[reg_id]);
        }

        CPUSH ("rax");

        // POP ("rax");

        return TRANSLATE_SUCCESS;
    }

    WRITE_NEWLINE;

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

    // todo
    return FUNC_SUCCESS;
}

// ================================================================================================

TranslateRes TranslateFunctionCall (const TreeNode* func_id_node, AsmText* asm_text, const NameTable* nametable)
{
    assert (func_id_node);
    assert (asm_text);
    assert (nametable);

    if (TYPE (func_id_node) != IDENTIFIER)      return TRANSLATE_TYPE_NOT_MATCH;
    if (!IsFunction (func_id_node, nametable))  return TRANSLATE_ERROR;

    PUSH ("rax");

    TreeNode *func_param_container = func_id_node->left;

    while (func_param_container)
    {
        WRITE ("\t\t\t; begin transfer param %s\n", NAME (func_param_container->right));

        TranslateASTSubtree (func_param_container->right, asm_text, nametable);
        REPUSH;

        WRITE ("\t\t\t; end transfer param %s\n", NAME (func_param_container->right));

        func_param_container = func_param_container->left;
    }

    CALL ("%s", NAME (func_id_node));

    int n_params = N_PARAMS (func_id_node);
    if (n_params)
        WRITE ("add rsp, %d\t\t\t; pop %s function params\n", n_params * QWORD_SIZE, NAME (func_id_node));

    CPUSH ("rax\t\t; cpush ret val");

    POP ("rax\n");

    return TRANSLATE_SUCCESS;
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

    WRITE ("\t\t; cond_%ld\n", asm_text->cond_count);

    asm_text->cond_count++;

    return FUNC_SUCCESS;
}

DefaultFuncRes TranslateCondition (const TreeNode* op_node, AsmText* asm_text, const NameTable* nametable, const char *jmp_comparator)
{
    assert (op_node);
    assert (asm_text);
    assert (nametable);
    assert (jmp_comparator);

    WRITE ("\t\t; cond_%ld\n", COND_COUNT);

    PUSH ("rax");

    TranslateASTSubtree (op_node->left, asm_text, nametable);   // cpush
    TranslateASTSubtree (op_node->right, asm_text, nametable);  // cpush

    MOV ("rax", "QWORD [r15 + 8]");
    CMP ("rax", "QWORD [r15]");

    WRITE ("%s cpush_true_%ld\n", jmp_comparator, COND_COUNT);

    WRITE_NO_TAB ("cpush_false_%ld:\n", COND_COUNT);
    MOV ("rax", "0");

    WRITE ("jmp end_cond_%ld\n", COND_COUNT);

    WRITE_NO_TAB ("cpush_true_%ld:\n", COND_COUNT);
    MOV ("rax", "1");

    WRITE_NO_TAB ("end_cond_%ld:\n", COND_COUNT);

    WRITE ("add r15, 16\t\t\t; cpop compared vals\n");

    CPUSH ("rax");

    POP ("rax");

    COND_COUNT++;

    return FUNC_SUCCESS;
}

// ================================================================================================

OffsetTable* OffsetTableCtor ()
{
    OffsetTable* offset_table = (OffsetTable*) calloc (1, sizeof (OffsetTable));

    offset_table->n_params          = 0;
    offset_table->offset_table      = (int*) calloc (OFFSET_TABLE_CAPACITY, sizeof (int));
    offset_table->offset_table_base = 0;
    offset_table->offset_table_ptr  = 0;

    offset_table->base_stack = StackLightCtor (MAX_SCOPE_DEPTH);

    return offset_table;
}

// ================================================================================================

DefaultFuncRes OffsetTableDtor (OffsetTable* offset_table)
{
    assert (offset_table);

    StackLightDtor (offset_table->base_stack);
    free (offset_table->offset_table);
    free (offset_table);

    return FUNC_SUCCESS;
}

// ================================================================================================

DefaultFuncRes OffsetTableAddFrame (OffsetTable* offset_table)
{
    assert (offset_table);

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

    int eff_offset = 0; // error code - not found

    for (size_t i = 0; i < offset_table->offset_table_ptr; i++)
    {
        if (offset_table->offset_table[i] == (int) var_id)
        {
            eff_offset = i - offset_table->n_params;
            if (eff_offset >= 0) eff_offset++; // to avoid [rbp - 0]
            else eff_offset--;                 // to avoid [rbp + 8] - ret addr

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

    int eff_offset = -OFFSET_TABLE->n_params - 1; // -1 to avoid [rbp + 8] - ret addr

    for (size_t i = 0; i < OFFSET_TABLE->offset_table_ptr; i++, eff_offset++)
    {
        if (eff_offset == 0) eff_offset += 2; // to avoid [rbp + 0],
                                              // eff_offset = 0 - is bad and means "val not found"
        if (eff_offset < 0)
        {
            WRITE ("\t\t\t; par int64_t %6s @ rbp+0x%x\n",
                        nametable->names[OFFSET_TABLE->offset_table[i]],
                        (unsigned) (-eff_offset * QWORD_SIZE));
        }
        else
        {
            WRITE ("\t\t\t; var int64_t %6s @ rbp-0x%x\n",
                        nametable->names[OFFSET_TABLE->offset_table[i]],
                        eff_offset * QWORD_SIZE);
        }
    }

    WRITE_NEWLINE;

    return 0;
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
        OffsetTableAddFrame (offset_table);

    if (TYPE (func_body_node) == DECLARATOR && VAL (func_body_node) == VAR_DECLARATOR)
        OffsetTableAddVariable (offset_table, VAL (func_body_node->left->left));

    OffsetTableAddFuncLocals (offset_table, func_body_node->right, nametable);

    return FUNC_SUCCESS;
}

// ================================================================================================

int OffsetTableGetCurrFrameWidth (OffsetTable* offset_table)
{
    assert (offset_table);

    int frame_width = offset_table->offset_table_ptr - offset_table->offset_table_base;

    return frame_width;
}


