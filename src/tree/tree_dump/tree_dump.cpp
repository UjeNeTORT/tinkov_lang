/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "tree_dump.h"

// ============================================================================================

int TreeDotDump (const char* HTML_fname, const Tree* tree)
{
    assert (HTML_fname);
    assert (tree);

    srand(time(0));

    int dump_id = (int) time(NULL);

    char dot_path[MAX_PATH] = "";
    char detailed_dot_path[MAX_PATH] = "";

    FILE* dot_file          = InitDotDump (tree, dot_path,          SIMPLE_DUMP);
    FILE* detailed_dot_file = InitDotDump (tree, detailed_dot_path, DETAILED_DUMP);

    DotTreePrint         (dot_file,          tree);
    DotTreeDetailedPrint (detailed_dot_file, tree);

    ConcludeDotDump (dot_file);
    ConcludeDotDump (detailed_dot_file);

    CompileDot (dot_path,          dump_id, SIMPLE_DUMP);
    CompileDot (detailed_dot_path, dump_id, DETAILED_DUMP);

    WriteHTML(HTML_fname, dump_id);

    return dump_id;
}

// ============================================================================================

FILE* InitDotDump (const Tree* tree, char* dot_path, DotDumpType dump_type)
{
    assert (tree);
    assert (dot_path);
    if (!tree) RET_ERROR (NULL, "Tree null pointer");
    if (!dot_path) RET_ERROR (NULL, "Dot_path null pointer");
    if (dump_type != SIMPLE_DUMP && dump_type != DETAILED_DUMP)
                RET_ERROR (NULL, "Unknown dump type %d", dump_type);

    int dump_id = (int) time (NULL);

    if (dump_type == SIMPLE_DUMP)
        sprintf (dot_path, "expr_%d.dot", dump_id);
    else if (dump_type == DETAILED_DUMP)
        sprintf (dot_path, "detailed_expr_%d.dot", dump_id);

    char* temp_dot_path = GetFilePath (DOT_FILE_PATH, dot_path);
    strcpy (dot_path, temp_dot_path);
    free (temp_dot_path);

    FILE* dot_file = fopen (dot_path, "wb");

    if (dump_type == SIMPLE_DUMP)
        fprintf (dot_file, "digraph TREE {\n"
                       "bgcolor =\"%s\"\n", GRAPH_BGCLR);
    else if (dump_type == DETAILED_DUMP)
        fprintf (dot_file, "digraph DETAILED_TREE {\n"
                        "bgcolor =\"%s\"\n", GRAPH_BGCLR);

    return dot_file;
}

// ============================================================================================

DotTreePrintRes DotTreePrint (FILE* dot_file, const Tree* tree)
{
    assert (dot_file);
    assert (tree);
    if (!dot_file) RET_ERROR (DOT_PRINT_ERR, "Tex filename null pointer\n");
    if (!tree)     RET_ERROR (DOT_PRINT_ERR, "Tree null pointer\n");

    int node_id = 0;
    return DotSubtreePrint (dot_file, tree->root, tree, &node_id);
}

// ============================================================================================

int CompileDot (char* dot_path, int dump_id, DotDumpType dump_type)
{
    assert (dot_path);

    char command[COMMAND_BUF_SIZE] = "";

    if(dump_type == SIMPLE_DUMP)
        sprintf (command, "dot -Tsvg %s -o %sgraph_dump_%d.svg", dot_path, GRAPH_SVGS_PATH, dump_id);
    else if (dump_type == DETAILED_DUMP)
        sprintf (command, "dot -Tsvg %s -o %sdetailed_graph_dump_%d.svg", dot_path, GRAPH_SVGS_PATH, dump_id);

    system (command);

    return 0;
}

// ============================================================================================

int WriteHTML (const char* HTML_fname, int dump_id)
{
    assert (HTML_fname);

    char* HTML_path = GetFilePath (HTML_DUMPS_PATH, HTML_fname);

    time_t t = time (NULL);

    tm* loc_time = localtime (&t);

    FILE* HTML_file = fopen (HTML_path, "wb"); // ! attention, deletion of old dumps
    fprintf (HTML_file, "<div graph_%d style=\"background-color: %s; color: %s;\">\n",
                                                        dump_id, GRAPH_BGCLR, GRAPH_TEXTCLR);

    fprintf (HTML_file, "<p style=\"color: %s; font-family:monospace; font-size: 20px\">[%s] TREE</p>",
                                                                            "#283D3B", asctime(loc_time));

    fprintf (HTML_file, "<img src=\"../../../../../%sgraph_dump_%d.svg\">\n",
                                                    GRAPH_SVGS_PATH, dump_id);

    fprintf (HTML_file, "</div>\n");

    fprintf (HTML_file, "<div detailed_graph_%d style=\"background-color: %s; color: %s;\">\n",
                                                                dump_id, GRAPH_BGCLR, GRAPH_TEXTCLR);

    fprintf (HTML_file, "<p style=\"color: %s; font-family:monospace; font-size: 20px\">[%s] TREE DETAILED </p>",
                                                                            "#283D3B", asctime(loc_time));

    fprintf (HTML_file, "<img src=\"../../../../../%sdetailed_graph_dump_%d.svg\">\n",
                                                    GRAPH_SVGS_PATH, dump_id);

    fprintf (HTML_file, "</div>\n");

    fprintf (HTML_file, "<hr> <!-- ============================================================================================================================ --> <hr>\n");
    fprintf (HTML_file, "\n");

    fclose (HTML_file);

    free (HTML_path);

    return 0;
}

// ============================================================================================

int ConcludeDotDump (FILE* dot_file)
{
    assert (dot_file);

    fprintf (dot_file, "}\n");

    fclose (dot_file);

    return 0;
}

// ============================================================================================

DotTreePrintRes DotSubtreePrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id)
{
    assert (dot_file);
    assert (tree);
    assert (node_id);
    if (!dot_file) RET_ERROR (DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree)     RET_ERROR (DOT_PRINT_ERR_PARAMS, "Tree null pointer");
    if (!node_id)  RET_ERROR (DOT_PRINT_ERR_PARAMS, "Node id null pointer");

    if (!node)
    {
        WARN ("node null");
        *node_id = 0;

        return DOT_PRINT_SUCCESS;
    }

    *node_id = rand();

    const char* color = "";
    char node_data[MAX_OP] = "";

    int opnum = 0;

    switch (TYPE(node))
    {
    case ERROR:
        color = GRAPH_ERRCLR;
        sprintf(node_data, "ERR");

        break;

    case IDENTIFIER:
        color = GRAPH_IDCLR;
        sprintf (node_data, "%s", tree->nametable->names[VAL (node)]); // get varname from nametable

        break;

    case DECLARATOR:
        color = GRAPH_DECCLR;
        sprintf (node_data, "%s", DECLARATORS[VAL (node)].name);

        break;

    case KEYWORD:
        color = GRAPH_KWCLR;
        sprintf (node_data, "%s", KEYWORDS[VAL (node)].name);

        break;

    case SEPARATOR:
        color = GRAPH_SEPCLR;
        sprintf (node_data, "%s", SEPARATORS[VAL (node)].name);

        break;

    case OPERATOR:
        color = GRAPH_OPCLR;

        sprintf (node_data, "%s", OPERATORS[VAL (node)].name);

        break;

    case INT_LITERAL:
        color = GRAPH_INTCLR;
        sprintf (node_data, "%d", VAL (node));

        break;

    default:
        RET_ERROR (DOT_PRINT_ERR, "Unknown node type (%d)", TYPE(node));

        break;
    }

    fprintf (dot_file, "\tnode_%d [style = \"filled, rounded\", shape = rectangle, label = \"%s\", fillcolor = \"%s\", fontcolor = \"%s\"];\n", *node_id, node_data, color, GRAPH_TEXTCLR);

    int left_subtree_id  = 0;
    int mid_subtree_id   = 0;
    int right_subtree_id = 0;

    if (DotSubtreePrint (dot_file, (const TreeNode *) node->left, tree, &left_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreePrint (dot_file, (const TreeNode *) node->mid, tree, &mid_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreePrint (dot_file, (const TreeNode *) node->right, tree, &right_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (left_subtree_id != 0)
        fprintf (dot_file, "\tnode_%d -> node_%d;\n", *node_id, left_subtree_id);

    if (mid_subtree_id != 0)
        fprintf (dot_file, "\tnode_%d -> node_%d;\n", *node_id, mid_subtree_id);

    if (right_subtree_id != 0)
        fprintf (dot_file, "\tnode_%d -> node_%d;\n", *node_id, right_subtree_id);

    return DOT_PRINT_SUCCESS;
}

// ============================================================================================

DotTreePrintRes DotTreeDetailedPrint (FILE* dot_file, const Tree* tree)
{
    assert (dot_file);
    assert (tree);
    if (!dot_file) RET_ERROR (DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree) RET_ERROR (DOT_PRINT_ERR_PARAMS, "Tree null pointer");

    int node_id = 0;

    return DotSubtreeDetailedPrint (dot_file, (const TreeNode *) tree->root, tree, &node_id);
}

// ============================================================================================

DotTreePrintRes DotSubtreeDetailedPrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id)
{
    assert (dot_file);
    assert (tree);
    assert (node_id);
    if (!dot_file) RET_ERROR (DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree)     RET_ERROR (DOT_PRINT_ERR_PARAMS, "Tree null pointer");
    if (!node_id)  RET_ERROR (DOT_PRINT_ERR_PARAMS, "Node id null pointer");

    if (!node)
    {
        *node_id = 0;

        return DOT_PRINT_SUCCESS;
    }

    *node_id = rand();

    const char* color = "";

    switch (TYPE(node))
    {
    case ERROR:
        color = GRAPH_ERRCLR;
        break;

    case IDENTIFIER:
        color = GRAPH_IDCLR;
        break;

    case DECLARATOR:
        color = GRAPH_DECCLR;
        break;

    case KEYWORD:
        color = GRAPH_KWCLR;
        break;

    case SEPARATOR:
        color = GRAPH_SEPCLR;
        break;

    case OPERATOR:
        color = GRAPH_OPCLR;
        break;

    case INT_LITERAL:
        color = GRAPH_INTCLR;
        break;

    default:
        RET_ERROR (DOT_PRINT_ERR, "Unknown node type (%d)", TYPE(node));

        break;
    }

    fprintf (dot_file, "\tdetailed_node_%d [style = filled, shape = record, fillcolor = \"%s\", fontcolor = \"%s\"];\n", *node_id, color, GRAPH_TEXTCLR);
    fprintf (dot_file, "\tdetailed_node_%d [label = \"{type = %d | val = %d}\"];\n", *node_id, TYPE(node), VAL(node));

    int left_subtree_id  = 0;
    int mid_subtree_id  = 0;
    int right_subtree_id = 0;

    if (DotSubtreeDetailedPrint (dot_file, (const TreeNode *) node->left, tree, &left_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreeDetailedPrint (dot_file, (const TreeNode *) node->mid, tree, &mid_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreeDetailedPrint (dot_file, (const TreeNode *) node->right, tree, &right_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR (DOT_PRINT_ERR, "Previous function returned error code");

    if (left_subtree_id != 0)
        fprintf (dot_file, "\tdetailed_node_%d -> detailed_node_%d;\n", *node_id, left_subtree_id);

    if (mid_subtree_id != 0)
        fprintf (dot_file, "\tdetailed_node_%d -> detailed_node_%d;\n", *node_id, mid_subtree_id);
    if (right_subtree_id != 0)
        fprintf (dot_file, "\tdetailed_node_%d -> detailed_node_%d;\n", *node_id, right_subtree_id);

    return DOT_PRINT_SUCCESS;
}

// ============================================================================================

char* GetFilePath(const char* path, const char* fname)
{
    assert (path);
    assert (fname);

    char* fpath = (char *) calloc (MAX_PATH, sizeof(char));

    strcat (fpath, path); // path must have / in the end
    strcat (fpath, fname);

    return fpath;
}
