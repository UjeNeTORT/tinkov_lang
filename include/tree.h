/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
 *************************************************************************/

#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#include "common.h"
#include "colors.h"
#include "lang_syntax.h"

const size_t MAX_N_NODES = 50000;  // max number of nodes in AST
const size_t MAX_TREE    = 500000; // max len of a string-written tree in a file
const size_t MAX_OP      = 200;    // max len of an operator

const size_t NAMETABLE_CAPACITY = 1000;

const double EXPONENT = 2.718281828;
const double PI       = 3.141592654;

// ===================== DSL =====================

#define TYPE(node)      ((node)->data.type)
#define VAL(node)       ((node)->data.val)

#define NAME(node)      ((nametable)->names[VAL (node)])
#define N_PARAMS(node)  ((nametable)->n_params[VAL (node)])
#define PARAMS(node, i) ((nametable)->params[VAL (node)][i])

#define CHECK_VAL(node, val) (TYPE(node) == INT_LITERAL && VAL(node) == val)
// ===============================================

// ================ Return Values ================
typedef enum
{
    TREE_EVAL_SUCCESS    = 0,
    TREE_EVAL_ERR        = 1,
    TREE_EVAL_ERR_PARAMS = 2,
    TREE_EVAL_FORBIDDEN  = 3,
} TreeEvalRes;

typedef enum
{
    TREE_SIMPLIFY_SUCCESS    = 0,
    TREE_SIMPLIFY_ERR        = 1,
    TREE_SIMPLIFY_ERR_PARAMS = 2,
} TreeSimplifyRes;

typedef enum
{
    TREE_DTOR_SUCCESS    = 0,
    TREE_DTOR_ERR_PARAMS = 1,
} TreeDtorRes;

typedef enum
{
    NAMETABLE_CTOR_SUCCESS    = 0,
    NAMETABLE_CTOR_ERR        = 1,
    NAMETABLE_CTOR_ERR_PARAMS = 2,
} NameTableCtorRes;

typedef enum
{
    NAMETABLE_DTOR_SUCCESS    = 0,
    NAMETABLE_DTOR_ERR        = 1,
    NAMETABLE_DTOR_ERR_PARAMS = 2,
} NameTableDtorRes;

typedef enum
{
    NAMETABLE_COPY_SUCCESS    = 0,
    NAMETABLE_COPY_ERR_PARAMS = 1,
} NameTableCopyRes;

typedef enum
{
    LIFT_CHILD_TO_PARENT_SUCCESS    = 0,
    LIFT_CHILD_TO_PARENT_ERR        = 1,
} LiftChildToParentRes;

typedef enum
{
    SUBTR_TO_NUM_SUCCESS    = 0,
    SUBTR_TO_NUM_ERR        = 1,
    SUBTR_TO_NUM_ERR_PARAMS = 2,
} SubtreeToNumRes;

typedef enum
{
    TREE_NODE_DTOR_SUCCESS = 0,
} TreeNodeDtorRes;

typedef enum
{
    LEFT    = 0,
    RIGHT   = 1,
} NodeLocation;

typedef enum
{
    WRT_TREE_SUCCESS    = 0,
    WRT_TREE_ERR        = 1,
    WRT_TREE_ERR_PARAMS = 2,
} WriteTreeRes;

typedef enum
{
    READ_ASSIGN_ID_SUCCESS       = 0,
    READ_ASSIGN_ID_ERR_NOT_FOUND = 1,
    READ_ASSIGN_ID_ERR_PARAMS    = 2,
} ReadAssignIdentifierRes;

typedef enum
{
    READ_ASSIGN_DECLR_SUCCESS       = 0,
    READ_ASSIGN_DECLR_ERR_NOT_FOUND = 1,
    READ_ASSIGN_DECLR_ERR_PARAMS    = 2,
} ReadAssignDeclaratorRes;

typedef enum
{
    READ_ASSIGN_KW_SUCCESS       = 0,
    READ_ASSIGN_KW_ERR_NOT_FOUND = 1,
    READ_ASSIGN_KW_ERR_PARAMS    = 2,
} ReadAssignKeywordRes;

typedef enum
{
    READ_ASSIGN_SEP_SUCCESS       = 0,
    READ_ASSIGN_SEP_ERR_NOT_FOUND = 1,
    READ_ASSIGN_SEP_ERR_PARAMS    = 2,
} ReadAssignSeparatorRes;

typedef enum
{
    READ_ASSIGN_OP_SUCCESS       = 0,
    READ_ASSIGN_OP_ERR_NOT_FOUND = 1,
    READ_ASSIGN_OP_ERR_PARAMS    = 2,
} ReadAssignOperatorRes;

typedef enum
{
    READ_ASSIGN_NUM_SUCCESS     = 0,
    READ_ASSIGN_NUM_ERR         = 1,
    READ_ASSIGN_NUM_ERR_PARAMS  = 2,
} ReadAssignNumberRes;

typedef enum
{
    SKIP_SPACES_SUCCESS    = 0,
    SKIP_SPACES_ERR        = 1,
    SKIP_SPACES_ERR_PARAMS = 2,
} SkipSpacesRes;

struct NameTable
{
    char* names    [NAMETABLE_CAPACITY]; // variable names
    int*  params   [NAMETABLE_CAPACITY]; // array of parameters of function
    int   n_params [NAMETABLE_CAPACITY];
    int   main_index;
    int   free;
};

struct NodeData
{
    NodeType type;
    int      val;
};

struct TreeNode
{
    NodeData data;

    TreeNode* prev;

    TreeNode* left;
    TreeNode* right;
};

struct Tree
{
    TreeNode*  root;
    NameTable* nametable;
};

typedef long long TreeErrorVector;
typedef int (* NodeAction_t) (TreeNode* node);

TreeEvalRes             TreeEval                 (const Tree* tree, int* result);
TreeEvalRes             SubtreeEval              (const TreeNode* node, const Tree* tree, int* result);
TreeEvalRes             SubtreeEvalUnOp          (const TreeNode* node, int right, int* result);
TreeEvalRes             SubtreeEvalBiOp          (const TreeNode* node, int left, int right, int* result);

TreeSimplifyRes         TreeSimplify             (Tree* tree);
TreeSimplifyRes         SubtreeSimplify          (TreeNode* node);
TreeSimplifyRes         SubtreeSimplifyConstants (TreeNode* node, int* tree_changed_flag);
TreeSimplifyRes         SubtreeSimplifyNeutrals  (TreeNode* node, int* tree_changed_flag);

TreeNode*               TreeNodeCtor             (int val, NodeType type, TreeNode* prev, TreeNode* left, TreeNode* right);
int                     TreeNodeDtor             (TreeNode* node);
int                     SubtreeDtor              (TreeNode* node);

Tree*                   TreeCtor                 ();
TreeDtorRes             TreeDtor                 (Tree* tree);

NameTable*              NameTableCtor            ();
NameTableDtorRes        NameTableDtor            (NameTable* nametable);
int                     NameTableUpdFuncParams   (const TreeNode* node, NameTable* nametable);

int                     CountFuncParams          (const TreeNode* func_declr_node);

Tree*                   TreeCopyOf               (const Tree* tree);
TreeNode*               SubtreeCopyOf            (const TreeNode* node);
NameTableCopyRes        NameTableCopy            (NameTable* dst, const NameTable* src);

LiftChildToParentRes    LiftChildToParent        (TreeNode* node, NodeLocation child_location);
SubtreeToNumRes         SubtreeToNum             (TreeNode* node, int val);

Tree*                   ReadTree                 (FILE* stream);
Tree*                   ReadTree                 (const char* infix_tree);
TreeNode*               ReadSubtree              (const char* infix_tree, const Tree* tree, int* offset);
NodeData                ReadNodeData             (const char* infix_tree, const Tree* tree, int* offset);

ReadAssignNumberRes     ReadAssignNumber          (NodeData* data, char* word);
ReadAssignDeclaratorRes ReadAssignDeclarator      (NodeData* data, char* word);
ReadAssignKeywordRes    ReadAssignKeyword         (NodeData* data, char* word);
ReadAssignSeparatorRes  ReadAssignSeparator       (NodeData* data, char* word);
ReadAssignOperatorRes   ReadAssignOperator        (NodeData* data, char* word);
ReadAssignIdentifierRes ReadAssignIdentifier      (NodeData* data, char* word, const Tree* tree);

WriteTreeRes            WriteTree                 (FILE* stream, const Tree* tree);
WriteTreeRes            WriteSubtree              (FILE* stream, const TreeNode* node, const Tree* tree);
WriteTreeRes            WriteNodeData             (FILE* stream, NodeData data, const NameTable* nametable);

int FindInNametable (const char* word, const NameTable* nametable);
int UpdNameTable       (const char* word, NameTable* nametable);
int IsVarNameCorrect   (const char* word);

int FindDeclarator (int declr_code);
int FindKeyword    (int kw_code);
int FindSeparator  (int sep_code);
int FindOperator   (int op_code);

int SubtreeHasVars (const TreeNode* node, const NameTable* nametable);

int IsDouble (char* word); // ! WARNING cructh function

SkipSpacesRes SkipSpaces (const char* string, int* offset);

#endif // TREE_H
