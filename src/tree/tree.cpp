/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/Differentiator
 *************************************************************************/

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "tree.h"

// ================================================================================================

TreeEvalRes TreeEval (const Tree* tree, int* result)
{
    assert (tree);
    assert (result);
    if (!tree)   RET_ERROR (TREE_EVAL_ERR_PARAMS, "Tree null pointer");
    if (!result) RET_ERROR (TREE_EVAL_ERR_PARAMS, "Result null pointer");

    return SubtreeEval (tree->root, tree, result);
}

// ================================================================================================

TreeEvalRes SubtreeEval (const TreeNode* node, const Tree* tree, int* result)
{
    assert (tree);
    assert (result);
    if (!tree)   RET_ERROR (TREE_EVAL_ERR_PARAMS, "Tree null pointer");
    if (!result) RET_ERROR (TREE_EVAL_ERR_PARAMS, "Result null pointer");

    if (!node) return TREE_EVAL_SUCCESS;

    int left  = 0;
    int right = 0;

    TreeEvalRes ret_val = TREE_EVAL_SUCCESS;

    switch (TYPE(node))
    {
    case INT_LITERAL:
        *result = VAL(node);

        break;

    case IDENTIFIER:
        break;

    case OPERATOR:
        ret_val = SubtreeEval (node->left, tree, &left);
        if (ret_val != TREE_EVAL_SUCCESS)
            RET_ERROR (TREE_EVAL_ERR, "Previous function returned error code");

        ret_val = SubtreeEval (node->right, tree, &right);
        if (ret_val != TREE_EVAL_SUCCESS)
            RET_ERROR (TREE_EVAL_ERR, "Previous function returned error code");

        ret_val = SubtreeEvalBiOp (node, left, right, result);

        break;

    default:
        RET_ERROR (TREE_EVAL_FORBIDDEN, "Unknown node type %d", TYPE(node));
    }

    return ret_val;
}

// ================================================================================================

TreeEvalRes SubtreeEvalBiOp (const TreeNode* node, int left, int right, int* result)
{
    assert (result);
    if (!result) return TREE_EVAL_ERR_PARAMS;
    if (TYPE(node) != OPERATOR) RET_ERROR (TREE_EVAL_ERR_PARAMS, "You can not pass anything but OPERATOR node to this function");

    if (!node) return TREE_EVAL_SUCCESS;

    switch (VAL(node))
    {
    case ADD:
        *result = left + right;
        break;

    case SUB:
        *result = left - right;
        break;

    case MUL:
        *result = left* right;
        break;

    case DIV:
        *result = left / right;
        break;

    case POW:
        *result = pow(left,right);
        break;

    default:
        RET_ERROR (TREE_EVAL_ERR, "Unknown operator: %d", VAL(node));
    }

    return TREE_EVAL_SUCCESS;
}

// ================================================================================================

TreeSimplifyRes TreeSimplify (Tree* tree)
{
    assert (tree);
    if (!tree) RET_ERROR (TREE_SIMLIFY_ERR_PARAMS, "Tree null pointer");

    return SubtreeSimplify(tree->root);
}

// ================================================================================================

TreeSimplifyRes SubtreeSimplify (TreeNode* node)
{
    assert (node);
    if (!node) return TREE_SIMPLIFY_SUCCESS;

    TreeSimplifyRes ret_val = TREE_SIMPLIFY_SUCCESS;

    int tree_changed_flag = 0;

    do
    {
        tree_changed_flag = 0;

        ret_val = SubtreeSimplifyConstants (node, &tree_changed_flag);
        if (ret_val != TREE_SIMPLIFY_SUCCESS) break;

        ret_val = SubtreeSimplifyNeutrals  (node, &tree_changed_flag);
        if (ret_val != TREE_SIMPLIFY_SUCCESS) break;
    } while (tree_changed_flag);

    return ret_val;
}

// ================================================================================================

TreeSimplifyRes SubtreeSimplifyConstants (TreeNode* node, int* tree_changed_flag)
{
    assert (tree_changed_flag);
    if (!tree_changed_flag)
               RET_ERROR (TREE_SIMLIFY_ERR_PARAMS, "flag null pointer");

    if (!node) return TREE_SIMPLIFY_SUCCESS;
    if (TYPE(node) == INT_LITERAL) return TREE_SIMPLIFY_SUCCESS;
    if (TYPE(node) == IDENTIFIER) return TREE_SIMPLIFY_SUCCESS;

    TreeSimplifyRes ret_val = TREE_SIMPLIFY_SUCCESS;

    ret_val = SubtreeSimplifyConstants (node->left, tree_changed_flag);
    if (ret_val != TREE_SIMPLIFY_SUCCESS)
        RET_ERROR (TREE_SIMPLIFY_ERR, "Previous function returned error code %d", ret_val);

    ret_val = SubtreeSimplifyConstants (node->right, tree_changed_flag);
    if (ret_val != TREE_SIMPLIFY_SUCCESS)
        RET_ERROR (TREE_SIMPLIFY_ERR, "Previous function returned error code %d", ret_val);

    if (TYPE(node) == OPERATOR && TYPE(node->left) == INT_LITERAL && TYPE(node->right) == INT_LITERAL)
    {
        if (SubtreeEvalBiOp (node, VAL(node->left), VAL(node->right), &VAL(node)) != TREE_EVAL_SUCCESS)
            return TREE_SIMPLIFY_ERR;

        TYPE(node) = INT_LITERAL;

        TreeNodeDtor (node->left);
        TreeNodeDtor (node->right);
        node->left  = NULL;
        node->right = NULL;

        *tree_changed_flag = 1;

        return TREE_SIMPLIFY_SUCCESS;
    }

    return TREE_SIMPLIFY_SUCCESS;
}

// ================================================================================================

TreeSimplifyRes SubtreeSimplifyNeutrals  (TreeNode* node, int* tree_changed_flag)
{
    assert (tree_changed_flag);

    if (!node) return TREE_SIMPLIFY_SUCCESS;
    if (TYPE(node) == INT_LITERAL) return TREE_SIMPLIFY_SUCCESS;
    if (TYPE(node) == IDENTIFIER) return TREE_SIMPLIFY_SUCCESS;


    TreeSimplifyRes ret_val = TREE_SIMPLIFY_SUCCESS;

    ret_val = SubtreeSimplifyNeutrals(node->left,  tree_changed_flag);
    if (ret_val != TREE_SIMPLIFY_SUCCESS)
        RET_ERROR (TREE_SIMPLIFY_ERR, "Previous function returned error code %d", ret_val);

    ret_val = SubtreeSimplifyNeutrals(node->right, tree_changed_flag);
    if (ret_val != TREE_SIMPLIFY_SUCCESS)
        RET_ERROR (TREE_SIMPLIFY_ERR, "Previous function returned error code %d", ret_val);

    if (TYPE(node) != OPERATOR)
        return TREE_SIMPLIFY_SUCCESS; // only binary operators simplification is supported

    switch (VAL(node))
    {
    case ADD:
        if (CHECK_VAL(node->left, 0))
        {
            if (LiftChildToParent (node, RIGHT) != LIFT_CHILD_TO_PARENT_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Lift child to parent function failed");

            *tree_changed_flag = 1;
        }
        else if (CHECK_VAL(node->right, 0))
        {
            if (LiftChildToParent (node, LEFT) != LIFT_CHILD_TO_PARENT_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Lift child to parent function failed");

            *tree_changed_flag = 1;
        }

        break;

    case SUB:
        if (CHECK_VAL(node->right, 0))
        {
            if (LiftChildToParent (node, LEFT) != LIFT_CHILD_TO_PARENT_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Lift child to parent function failed");

            *tree_changed_flag = 1;
        }

        break;

    case MUL:

        if (CHECK_VAL(node->left, 1))
        {
            if (LiftChildToParent (node, RIGHT) != LIFT_CHILD_TO_PARENT_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Lift child to parent function failed");

            *tree_changed_flag = 1;
        }
        else if (CHECK_VAL(node->right, 1))
        {
            if (LiftChildToParent (node, LEFT) != LIFT_CHILD_TO_PARENT_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Lift child to parent function failed");

            *tree_changed_flag = 1;
        }
        else if (CHECK_VAL(node->left, 0))
        {
            if (SubtreeToNum(node, 0) != SUBTR_TO_NUM_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Subtree to num function failed");

            *tree_changed_flag = 1;
        }
        else if (CHECK_VAL(node->right, 0))
        {
            if (SubtreeToNum(node, 0) != SUBTR_TO_NUM_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Subtree to num function failed");

            *tree_changed_flag = 1;
        }

        break;

    case DIV:
        if (CHECK_VAL(node->left, 0))
        {
            if (SubtreeToNum(node, 0) != SUBTR_TO_NUM_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Subtree to num function failed");

            *tree_changed_flag = 1;
        }

        break;

    case POW:
        if (CHECK_VAL(node->right, 1))
        {
            LiftChildToParent (node, LEFT);

            *tree_changed_flag = 1;
        }
        else if (CHECK_VAL(node->right, 0))
        {
            if (SubtreeToNum(node, 1) != SUBTR_TO_NUM_SUCCESS)
                RET_ERROR (TREE_SIMPLIFY_ERR, "Subtree to num function failed");

            *tree_changed_flag = 1;
        }

        break;

    default:
        // other simplifications are not supported
        break;
    }

    return ret_val;
}

// ================================================================================================

TreeNode* TreeNodeCtor (int val, NodeType type, TreeNode* prev, TreeNode* left, TreeNode* right)
{
    TreeNode* new_node = (TreeNode *) calloc (1, sizeof (TreeNode));

    new_node->data  = {type, val};
    new_node->prev  = prev;
    new_node->left  = left;
    new_node->right = right;

    return new_node;
}

// ================================================================================================

int TreeNodeDtor (TreeNode* node)
{
    free (node);

    return 0; // return value in most cases is ignored
}

// ================================================================================================

int SubtreeDtor (TreeNode* node)
{
    if (!node) return 0;

    SubtreeDtor (node->left);
    SubtreeDtor (node->right);

    TreeNodeDtor (node);

    return 0; // return value in most cases is ignored
}

// ================================================================================================

Tree* TreeCtor ()
{
    Tree* tree = (Tree*) calloc (1, sizeof (Tree));

    tree->nametable = NameTableCtor();
    if (!tree->nametable)
    {
        TreeDtor(tree);

        RET_ERROR (NULL, "Nametable allocation error, tree constructor failed");
    }

    return tree;
}

// ================================================================================================

TreeDtorRes TreeDtor (Tree* tree)
{
    assert (tree);
    if (!tree) RET_ERROR (TREE_DTOR_ERR_PARAMS, "Tree null pointer");

    SubtreeDtor (tree->root);

    NameTableDtor (tree->nametable);

    free (tree);

    return TREE_DTOR_SUCCESS;
}

// ================================================================================================

NameTable* NameTableCtor ()
{
    NameTable* nametable = (NameTable*) calloc (1, sizeof (NameTable));
    if (!nametable) RET_ERROR (NULL, "nametable allocation error");

    for(size_t i = 0; i < NAMETABLE_CAPACITY; i++)
    {
        nametable->names[i] = (char *) calloc (MAX_OP, sizeof(char));
        if (!nametable->names[i]) RET_ERROR (NULL, "names[%d] allocation error", i);

        nametable->params[i]   = NULL;
        nametable->n_params[i] = -1;
    }

    nametable->main_index = -1;
    nametable->free  = 1;   // id = 0 not allowed

    return nametable;
}

// ================================================================================================

NameTableDtorRes NameTableDtor (NameTable* nametable)
{
    assert (nametable);
    if (!nametable) RET_ERROR (NAMETABLE_DTOR_ERR_PARAMS, "Nametable null pointer");

    for(size_t i = 0; i < NAMETABLE_CAPACITY; i++)
    {
        free (nametable->names[i]);
        free (nametable->params[i]);
    }

    free (nametable);

    return NAMETABLE_DTOR_SUCCESS;
}

// ================================================================================================

int NameTableUpdFuncParams (const TreeNode* node, NameTable* nametable)
{
    assert (nametable);

    if (!node) return 0;

    if (TYPE (node) == DECLARATOR && VAL (node) == FUNC_DECLARATOR)
    {
        int n_params = CountFuncParams (node);

        nametable->n_params[VAL (node->right->right)] = n_params;

        nametable->params[VAL (node->right->right)] = (int *) calloc (n_params, sizeof (int));

        const TreeNode* curr_param = node->right->left;

        for (int i = 0; i < n_params; i++)
        {
            // params are inverted so in nametable they are in correct order
            nametable->params[VAL (node->right->right)][n_params - 1 - i] = VAL (curr_param->right);
            curr_param = curr_param->left;
        }

        return 0;
    }

    NameTableUpdFuncParams (node->left, nametable);
    NameTableUpdFuncParams (node->right, nametable);

    return 0;
}

// ================================================================================================

int CountFuncParams (const TreeNode* func_declr_node)
{
    assert (func_declr_node);

    int n_params = 0;

    TreeNode* curr_param = func_declr_node->right->left;

    while (curr_param)
    {
        n_params++;
        curr_param = curr_param->left;
    }

    return n_params;
}

// ================================================================================================

Tree* TreeCopyOf (const Tree* tree)
{
    assert (tree);
    if (!tree) RET_ERROR (NULL, "Tree null pointer");

    Tree* copied = TreeCtor ();
    copied->root = SubtreeCopyOf (tree->root);

    if (NameTableCopy (copied->nametable, tree->nametable) != NAMETABLE_COPY_SUCCESS)
    {
        TreeDtor(copied);

        RET_ERROR (NULL, "Nametable allocation error, tree copying failed");
    }

    return copied;
}

// ================================================================================================

TreeNode* SubtreeCopyOf (const TreeNode* node)
{
    if (!node) return NULL;

    TreeNode* copied = TreeNodeCtor(VAL(node), TYPE(node), node->prev,
                                        SubtreeCopyOf(node->left), SubtreeCopyOf(node->right));

    return copied;
}

// ================================================================================================

NameTableCopyRes NameTableCopy (NameTable* dst, const NameTable* src)
{
    assert (dst);
    assert (src);
    if (!dst) RET_ERROR (NAMETABLE_COPY_ERR_PARAMS, "Destination nametable null pointer");
    if (!dst) RET_ERROR (NAMETABLE_COPY_ERR_PARAMS, "Source nametable null pointer");

    for (size_t i = 0; i < NAMETABLE_CAPACITY; i++)
    {
        memcpy (dst->names[i], src->names[i], strlen(src->names[i]));
    }

    dst->free  = src->free;

    return NAMETABLE_COPY_SUCCESS;
}

// ================================================================================================

LiftChildToParentRes LiftChildToParent (TreeNode* node, NodeLocation child_location)
{
    if (!node) return LIFT_CHILD_TO_PARENT_SUCCESS;

    TreeNode* child_node = child_location == LEFT ? node->left : node->right;

    if (!child_node)
        RET_ERROR (LIFT_CHILD_TO_PARENT_ERR, "Nothing to lift, child null pointer");

    VAL(node)  = VAL(child_node);
    TYPE(node) = TYPE(child_node);

    if (child_node->left)  child_node->left->prev  = node;
    if (child_node->right) child_node->right->prev = node;

    if (child_node == node->left)
        TreeNodeDtor (node->right);
    else
        TreeNodeDtor(node->left);

    node->left  = child_node->left;
    node->right = child_node->right;

    TreeNodeDtor(child_node);

    return LIFT_CHILD_TO_PARENT_SUCCESS;
}

// ================================================================================================

SubtreeToNumRes SubtreeToNum (TreeNode* node, int val)
{
    assert (node);
    if (!node) RET_ERROR (SUBTR_TO_NUM_ERR_PARAMS, "Node null pointer");

    VAL(node)  = val;
    TYPE(node) = INT_LITERAL;

    SubtreeDtor (node->left);
    SubtreeDtor (node->right);

    node->left  = NULL;
    node->right = NULL;

    return SUBTR_TO_NUM_SUCCESS;
}

// ================================================================================================

Tree* ReadTree (FILE* stream)
{
    assert (stream);

    char* infix_tree = (char *) calloc (MAX_TREE, sizeof(char));

    fread (infix_tree, MAX_TREE, sizeof (char), stream);

    infix_tree[strcspn (infix_tree, "\r\n")] = 0;

    Tree* readen = ReadTree ((const char *) infix_tree);

    free (infix_tree);

    return readen;
}

// ================================================================================================

Tree* ReadTree (const char* infix_tree)
{
    assert (infix_tree);

    int offset = 0;

    Tree* tree = TreeCtor();

    tree->root = ReadSubtree (infix_tree, tree, &offset);

    NameTableUpdFuncParams (tree->root, tree->nametable);

    return tree;
}

// ================================================================================================

TreeNode* ReadSubtree (const char* infix_tree, const Tree* tree, int* offset)
{
    assert (infix_tree);
    assert (tree);
    assert (offset);

    SkipSpaces (infix_tree, offset);

    if (infix_tree[*offset] == '*')
    {
        *offset += 1;

        return NULL;
    }

    if (infix_tree[*offset] != '(')
    {
        fprintf (stderr, "ReadSubTree: unknown action symbol %c (%d)\n", infix_tree[*offset], infix_tree[*offset]);

        return NULL;
    }

    TreeNode* node = TreeNodeCtor (0, INT_LITERAL, NULL, NULL, NULL);

    *offset += 1; // skip (

    node->left = ReadSubtree (infix_tree, tree, offset);
    if (node->left) node->left->prev = node;

    node->data = ReadNodeData (infix_tree, tree, offset);

    node->right = ReadSubtree (infix_tree, tree, offset);
    if (node->right) node->right->prev = node;

    while(infix_tree[*offset] != ')')
    {
        (*offset)++;
    }

    (*offset)++;

    return node;
}

// ================================================================================================

NodeData ReadNodeData (const char* infix_tree, const Tree* tree, int* offset)
{
    assert (infix_tree);
    assert (tree);
    assert (offset);

    NodeData data = {ERROR, 0};

    SkipSpaces(infix_tree, offset);

    char word[MAX_OP] = "";

    int addition = 0;

    sscanf((infix_tree + *offset), "%s%n", word, &addition); // todo what if var name longer than MAX_OP

    *offset += addition;

    if (ReadAssignNumber (&data, word) == READ_ASSIGN_NUM_SUCCESS)
        return data;

    if (ReadAssignDeclarator (&data, word) == READ_ASSIGN_DECLR_SUCCESS)
        return data;

    if (ReadAssignKeyword (&data, word) == READ_ASSIGN_KW_SUCCESS)
        return data;

    if (ReadAssignSeparator (&data, word) == READ_ASSIGN_SEP_SUCCESS)
        return data;

    if (ReadAssignOperator (&data, word) == READ_ASSIGN_OP_SUCCESS)
        return data;

    if (ReadAssignIdentifier (&data, word, tree) == READ_ASSIGN_ID_SUCCESS)
        return data;

    return data; // error NodeType by default
}

// ================================================================================================

ReadAssignNumberRes ReadAssignNumber (NodeData* data, char* word)
{
    assert (data);
    assert (word);
    if (!data) RET_ERROR (READ_ASSIGN_NUM_ERR_PARAMS, "data null pointer");
    if (!word) RET_ERROR (READ_ASSIGN_NUM_ERR_PARAMS, "word null pointer");

    if (atoi (word) != 0 || word[0] == '0') // ! may be unsafe
    {
        data->type = INT_LITERAL;
        data->val  = atoi (word);

        return READ_ASSIGN_NUM_SUCCESS; // assigned num to data
    }

    return READ_ASSIGN_NUM_ERR; // didnt assign num to data
}

// ================================================================================================

ReadAssignDeclaratorRes ReadAssignDeclarator (NodeData* data, char* word)
{
    assert (data);
    assert (word);
    if (!data) RET_ERROR (READ_ASSIGN_DECLR_ERR_PARAMS, "data null pointer");
    if (!word) RET_ERROR (READ_ASSIGN_DECLR_ERR_PARAMS, "word null pointer");

    for (size_t i = 0; i < N_DECLARATORS; i++)
    {
        if (streq (word, DECLARATORS[i].name))
        {
            data->type = DECLARATOR;
            data->val  = DECLARATORS[i].dclr_code;

            return READ_ASSIGN_DECLR_SUCCESS;
        }
    }

    return READ_ASSIGN_DECLR_ERR_NOT_FOUND;
}

// ================================================================================================

ReadAssignKeywordRes ReadAssignKeyword (NodeData* data, char* word)
{
    assert (data);
    assert (word);
    if (!data) RET_ERROR (READ_ASSIGN_KW_ERR_PARAMS, "data null pointer");
    if (!word) RET_ERROR (READ_ASSIGN_KW_ERR_PARAMS, "word null pointer");

    for (size_t i = 0; i < N_KEYWORDS; i++)
    {
        if (streq (word, KEYWORDS[i].name))
        {
            data->type = KEYWORD;
            data->val  = KEYWORDS[i].kw_code;

            return READ_ASSIGN_KW_SUCCESS;
        }
    }

    return READ_ASSIGN_KW_ERR_NOT_FOUND;
}

// ================================================================================================

ReadAssignSeparatorRes ReadAssignSeparator (NodeData* data, char* word)
{
    assert (data);
    assert (word);
    if (!data) RET_ERROR (READ_ASSIGN_SEP_ERR_PARAMS, "data null pointer");
    if (!word) RET_ERROR (READ_ASSIGN_SEP_ERR_PARAMS, "word null pointer");

    for (size_t i = 0; i < N_SEPARATORS; i++)
    {
        if (streq (word, SEPARATORS[i].name))
        {
            data->type = SEPARATOR;
            data->val  = SEPARATORS[i].sep_code;

            return READ_ASSIGN_SEP_SUCCESS;
        }
    }

    return READ_ASSIGN_SEP_ERR_NOT_FOUND;
}

// ================================================================================================

ReadAssignOperatorRes ReadAssignOperator (NodeData* data, char* word)
{
    assert (data);
    assert (word);
    if (!data) RET_ERROR (READ_ASSIGN_OP_ERR_PARAMS, "data null pointer");
    if (!word) RET_ERROR (READ_ASSIGN_OP_ERR_PARAMS, "word null pointer");

    for (size_t i = 0; i < N_OPERATORS; i++)
    {
        if (streq(word, OPERATORS[i].name))
        {
            data->type = OPERATOR;
            data->val  = OPERATORS[i].op_code;

            return READ_ASSIGN_OP_SUCCESS;
        }
    }

    return READ_ASSIGN_OP_ERR_NOT_FOUND;
}

// ================================================================================================

ReadAssignIdentifierRes ReadAssignIdentifier (NodeData* data, char* id_name, const Tree* tree)
{
    assert (data);
    assert (id_name);

    if (!data) RET_ERROR (READ_ASSIGN_ID_ERR_PARAMS, "data null pointer");
    if (!id_name) RET_ERROR (READ_ASSIGN_ID_ERR_PARAMS, "var_name null pointer");

    int var_id = FindInNametable (id_name, tree->nametable);

    if (var_id == -1) // not found in nametable
        var_id = UpdNameTable (id_name, tree->nametable);

    data->type = IDENTIFIER;
    data->val  = var_id;

    if (streq (id_name, "ЦАРЬ"))
        tree->nametable->main_index = data->val;

    return READ_ASSIGN_ID_SUCCESS;
}

// ================================================================================================

WriteTreeRes WriteTree(FILE* stream, const Tree* tree)
{
    assert (stream);
    assert (tree);
    if (!stream) RET_ERROR (WRT_TREE_ERR_PARAMS, "Stream null pointer");
    if (!tree)   RET_ERROR (WRT_TREE_ERR_PARAMS, "Tree null pointer");

    WriteTreeRes ret_val = WriteSubtree (stream, tree->root, tree);

    fprintf (stream, "\n");

    return ret_val;
}

// ================================================================================================

WriteTreeRes WriteSubtree(FILE* stream, const TreeNode* node, const Tree* tree)
{
    assert (stream);
    if (!stream) RET_ERROR (WRT_TREE_ERR_PARAMS, "Stream null pointer");

    if (!node)
    {
        fprintf(stream, "* ");

        return WRT_TREE_SUCCESS;
    }

    fprintf(stream, "( ");

    WriteSubtree (stream, node->left, tree);
    WriteTreeRes ret_val = WriteNodeData (stream, node->data, tree->nametable);
    WriteSubtree (stream, node->right, tree);

    fprintf(stream, ") ");

    return ret_val;
}

// ================================================================================================

WriteTreeRes WriteNodeData (FILE* stream, NodeData data, const NameTable* nametable)
{
    assert (stream);
    assert (nametable);
    if (!nametable) RET_ERROR (WRT_TREE_ERR_PARAMS, "Nametable null pointer");
    if (!stream)    RET_ERROR (WRT_TREE_ERR_PARAMS, "Word null pointer");

    switch (data.type)
    {
        case IDENTIFIER:
        {
            fprintf (stream, "%s ", nametable->names[data.val]);

            return WRT_TREE_SUCCESS;
        }

        case DECLARATOR:
        {
            int declr_index = FindDeclarator (data.val);
            if (declr_index != ILL_OPNUM)
                fprintf (stream, "%s ", DECLARATORS[declr_index].name);
            else
                fprintf (stream, "UNKNOWN DECLARATOR");

            return WRT_TREE_SUCCESS;
        }

        case KEYWORD:
        {
            int kw_index = FindKeyword (data.val);
            if (kw_index != ILL_OPNUM)
                fprintf (stream, "%s ", KEYWORDS[kw_index].name);
            else
                fprintf (stream, "UNKNOWN KEYWORD");

            return WRT_TREE_SUCCESS;
        }

        case SEPARATOR:
        {
            int sep_index = FindSeparator (data.val);
            if (sep_index != ILL_OPNUM)
                fprintf(stream, "%s ", SEPARATORS[sep_index].name);
            else
                fprintf(stream, "UNKNOWN SEPARATOR");

            return WRT_TREE_SUCCESS;
        }

        case OPERATOR:
        {
            int op_index = FindOperator (data.val);
            if (op_index != ILL_OPNUM)
                fprintf(stream, "%s ", OPERATORS[op_index].name);
            else
                fprintf(stream, "UNKNOWN OPERATOR");

            return WRT_TREE_SUCCESS;
        }

        case INT_LITERAL:
        {
            fprintf(stream, "%d ", data.val);

            return WRT_TREE_SUCCESS;
        }

        default:
            return WRT_TREE_ERR;
    }

    return WRT_TREE_ERR;
}

// ================================================================================================

int FindInNametable (const char* word, const NameTable* nametable)
{
    assert (nametable);
    assert (word);
    if (!nametable) RET_ERROR (ILL_OPNUM, "Nametable null pointer");
    if (!word)      RET_ERROR (ILL_OPNUM, "Word null pointer");

    for (size_t i = 0; i < NAMETABLE_CAPACITY; i++)
    {
        if (streq (nametable->names[i], word))
            return i; // duplicate id
    }

    return -1; // no duplicates
}

// ================================================================================================

int UpdNameTable (const char* word, NameTable* nametable)
{
    assert (nametable);
    assert (word);
    if (!nametable) RET_ERROR (-1, "Nametable null pointer");
    if (!word)      RET_ERROR (-1, "Word null pointer");

    strcpy (nametable->names[nametable->free], word);

    return nametable->free++;
}

// ================================================================================================

int IsVarNameCorrect (const char* word)
{
    assert (word);

    if (!isalpha (*word++))
        return 1;

    while (*word)
    {
        if (!isalnum (*word) && *word != '_')
            return 1;

        word++;
    }

    return 0;
}

// ================================================================================================

int FindOperator (int op_code)
{
    for (int i = 0; i < N_OPERATORS; i++)
        if (op_code == OPERATORS[i].op_code)
            return i;

    return ILL_OPNUM;
}

// ================================================================================================

int FindKeyword (int kw_code)
{
    for (int i = 0; i < N_OPERATORS; i++)
        if (kw_code == KEYWORDS[i].kw_code)
            return i;

    return ILL_OPNUM;
}

// ================================================================================================

int FindDeclarator (int declr_code)
{
    for (int i = 0; i < N_OPERATORS; i++)
        if (declr_code == DECLARATORS[i].dclr_code)
            return i;

    return ILL_OPNUM;
}

// ================================================================================================

int FindSeparator  (int sep_code)
{
    for (int i = 0; i < N_OPERATORS; i++)
        if (sep_code == SEPARATORS[i].sep_code)
            return i;

    return ILL_OPNUM;
}

// ================================================================================================

int IsDouble (char* word)
{
    // although it covers most cases, it
    // may be unsafe, not tested properly

    assert (word);
    if (!word) RET_ERROR (0, "Word null pointer");

    if (dbleq ( atof(word), 0) && *word != '0' && *word != '.')
        return 0;

    return 1;
}

// ================================================================================================

SkipSpacesRes SkipSpaces (const char* string, int* offset)
{
    assert (string);
    assert (offset);
    if (!string) RET_ERROR (SKIP_SPACES_ERR_PARAMS, "string null pointer");
    if (!offset) RET_ERROR (SKIP_SPACES_ERR_PARAMS, "offset null pointer");

    while (isspace (string[*offset]))
        (*offset)++;

    return SKIP_SPACES_SUCCESS;
}
