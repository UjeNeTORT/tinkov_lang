/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/Differentiator
 *************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "tree_dump.h"

// ============================================================================================

int TreeTexDump (const Tree* tree)
{
    assert(tree);

    char tex_path[MAX_TREE_FNAME] = "";

    FILE* tex_file = InitTexDump(tree, tex_path);

    if (TexTreePrint(tex_file, tree) != TEX_PRINT_SUCCESS)
    {
        RET_ERROR(-1, "Tree wasnt printed");
    }

    ConcludeTexDump(tex_file);

    CompileLatex(tex_path);

    return 0;
}

// ============================================================================================

FILE* InitTexDump (const Tree* tree, char* tex_path)
{
    assert (tree);
    assert (tex_path);
    if (!tree) RET_ERROR(NULL, "Tree null pointer");
    if (!tex_path) RET_ERROR(NULL, "Tex_path null pointer");

    int dump_id = (int) time(NULL);

    sprintf(tex_path, "expr_%d.tex", dump_id); // stores filename
    char* temp_tex_path = GetFilePath (TEX_FILE_PATH, tex_path);
    strcpy(tex_path, temp_tex_path);
    free(temp_tex_path);

    time_t t = time (NULL);
    tm* loc_time = localtime (&t);

    FILE* tex_file = fopen (tex_path, "wb");

    fprintf (tex_file,  "\\documentclass[a4paper,12pt]{article}\n\n"
                        "\\usepackage[T2A]{fontenc}"
                        "\\usepackage[russian]{babel}"
                        "\\usepackage{amsmath}\n"
                        "\\DeclareMathOperator\\arcctan{arcctan}\n"
                        "\\title{EXPRESSION DUMP}\n"
                        "\\author{Tikhonov Yaroslav (aka UjeNeTORT)}\n"
                        "\\date{Date: %d.%d.%d, Time %d:%d:%d}\n"
                        "\\begin{document}\n"
                        "\\maketitle\n",
                        loc_time->tm_mday, loc_time->tm_mon + 1, loc_time->tm_year + 1900,
                        loc_time->tm_hour, loc_time->tm_min, loc_time->tm_sec);

    return tex_file;
}

// ============================================================================================

TexTreePrintRes TexTreePrint (FILE* tex_file, const Tree* tree)
{
    assert (tex_file);
    assert (tree);

    if (!tex_file) RET_ERROR(TEX_PRINT_ERR, "Tex file null pointer");
    if (!tree)     RET_ERROR(TEX_PRINT_ERR, "Tree null pointer");

    fprintf(tex_file, "$$  ");

    if (tree->root)
        TexSubtreePrint (tex_file, tree->root, tree->root->right, tree);
    else
    {
        // it is not in the beginning because we may still need other info about the tree
        // apart from tree being shown itself

        RET_ERROR (TEX_PRINT_ERR, "Tree root null pointer");
    }

    fprintf(tex_file, "  $$\n\n");

    return TEX_PRINT_SUCCESS;
}

// ============================================================================================

int CompileLatex(const char* tex_path)
{
    assert(tex_path);

    char command[COMMAND_BUF_SIZE] = "";

    sprintf(command, "pdflatex -output-directory=%s %s > /dev/null 2>&1", PDF_DUMPS_PATH, tex_path);
    system(command);

    sprintf(command, "	rm -f %s/*.aux"
	                 "  rm -f %s/*.log", PDF_DUMPS_PATH, PDF_DUMPS_PATH);
    system(command);

    return 0;
}

// ============================================================================================

int ConcludeTexDump (FILE* tex_file)
{
    assert(tex_file);

    fprintf (tex_file, "\\end{document}\n");
    fclose (tex_file);

    return 0;
}

// ============================================================================================

int TreeDotDump (const char* HTML_fname, const Tree* tree)
{
    assert (HTML_fname);
    assert(tree);

    srand(time(0));

    int dump_id = (int) time(NULL);

    char dot_path[MAX_PATH] = "";
    char detailed_dot_path[MAX_PATH] = "";

    FILE* dot_file          = InitDotDump (tree, dot_path, SIMPLE_DUMP);
    FILE* detailed_dot_file = InitDotDump (tree, detailed_dot_path, DETAILED_DUMP);

    DotTreePrint (dot_file, tree);
    DotTreeDetailedPrint (detailed_dot_file, tree);

    ConcludeDotDump (dot_file);
    ConcludeDotDump (detailed_dot_file);

    CompileDot (dot_path, dump_id, SIMPLE_DUMP);
    CompileDot (detailed_dot_path, dump_id, DETAILED_DUMP);

    WriteHTML(HTML_fname, dump_id);

    return dump_id;
}

// ============================================================================================

FILE* InitDotDump (const Tree* tree, char* dot_path, DotDumpType dump_type)
{
    assert (tree);
    assert (dot_path);
    if (!tree) RET_ERROR(NULL, "Tree null pointer");
    if (!dot_path) RET_ERROR(NULL, "Dot_path null pointer");
    if (dump_type != SIMPLE_DUMP && dump_type != DETAILED_DUMP)
                RET_ERROR(NULL, "Unknown dump type %d", dump_type);

    int dump_id = (int) time(NULL);

    if (dump_type == SIMPLE_DUMP)
        sprintf(dot_path, "expr_%d.dot", dump_id);
    else if (dump_type == DETAILED_DUMP)
        sprintf(dot_path, "detailed_expr_%d.dot", dump_id);

    char* temp_dot_path = GetFilePath (DOT_FILE_PATH, dot_path);
    strcpy(dot_path, temp_dot_path);
    free(temp_dot_path);

    FILE* dot_file = fopen (dot_path, "wb");

    if (dump_type == SIMPLE_DUMP)
        fprintf (dot_file, "digraph TREE {\n"
                       "bgcolor =\"%s\"", GRAPH_BGCLR);
    else if (dump_type == DETAILED_DUMP)
        fprintf (dot_file, "digraph DETAILED_TREE {\n"
                        "bgcolor =\"%s\"", GRAPH_BGCLR);

    return dot_file;
}

// ============================================================================================

DotTreePrintRes DotTreePrint (FILE* dot_file, const Tree* tree)
{
    assert(dot_file);
    assert(tree);
    if (!dot_file) RET_ERROR (DOT_PRINT_ERR, "Tex filename null pointer\n");
    if (!tree)     RET_ERROR (DOT_PRINT_ERR, "Tree null pointer\n");

    int node_id = 0;
    DotSubtreePrint (dot_file, tree->root, tree, &node_id);

    return DOT_PRINT_SUCCESS;
}

// ============================================================================================

int CompileDot (char* dot_path, int dump_id, DotDumpType dump_type)
{
    assert(dot_path);

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

    char* HTML_path = GetFilePath(HTML_DUMPS_PATH, HTML_fname);

    time_t t = time (NULL);

    tm* loc_time = localtime (&t);

    FILE* HTML_file = fopen (HTML_path, "wb"); // ! attention, deletion of old dumps
    fprintf(HTML_file, "<div graph_%d style=\"background-color: %s; color: %s;\">\n",
                                                        dump_id, GRAPH_BGCLR, GRAPH_TEXTCLR);

    fprintf (HTML_file, "<p style=\"color: %s; font-family:monospace; font-size: 20px\">[%s] TREE</p>",
                                                                            "#283D3B", asctime(loc_time));

    fprintf (HTML_file, "<img src=\"../../../../%sgraph_dump_%d.svg\">\n",
                                                    GRAPH_SVGS_PATH, dump_id);

    fprintf(HTML_file, "</div>\n");

    fprintf(HTML_file, "<div detailed_graph_%d style=\"background-color: %s; color: %s;\">\n",
                                                                dump_id, GRAPH_BGCLR, GRAPH_TEXTCLR);

    fprintf (HTML_file, "<p style=\"color: %s; font-family:monospace; font-size: 20px\">[%s] TREE DETAILED </p>",
                                                                            "#283D3B", asctime(loc_time));

    fprintf (HTML_file, "<img src=\"../../../../%sdetailed_graph_dump_%d.svg\">\n",
                                                    GRAPH_SVGS_PATH, dump_id);

    fprintf(HTML_file, "</div>\n");

    fprintf (HTML_file, "<hr> <!-- ============================================================================================================================ --> <hr>\n");
    fprintf (HTML_file, "\n");

    fclose (HTML_file);

    free(HTML_path);

    return 0;
}

// ============================================================================================

int ConcludeDotDump (FILE* dot_file)
{
    assert(dot_file);

    fprintf (dot_file, "}\n");

    fclose (dot_file);

    return 0;
}

// ============================================================================================

TexSubtreePrintRes TexSubtreePrint (FILE* tex_file, const TreeNode* prev,
                                    const TreeNode* node, const Tree* tree)
{
    assert(tex_file);

    #ifdef WARNINGS
        if (!prev) WARN("Prev null pointer (braces wont be printed)\n");
    #endif // WARNINGS

    if (!node)
        return TEX_SUBT_PRINT_SUCCESS;

    const char* op_name = NULL;

    int opnum = 0;
    int print_parenthesis = 0;

    switch(TYPE(node))
    {
        case ERR:
            RET_ERROR(TEX_SUBT_PRINT_ERR, "Printer received error node");

            break;

        case NUM:
            fprintf(tex_file, " {%.3lf} ", VAL(node));

            break;

        case VAR:
            fprintf(tex_file, " {%s} ", tree->nametable->names[(int) VAL(node)]);

            break;

        case UN_OP:
            opnum = FindOperation((int) VAL(node));
            op_name = OPERATIONS[opnum].name;

            if (!streq(op_name, "="))
            {
                fprintf(tex_file, " \\%s ", op_name);
                TexSubtreePrint(tex_file, node, node->right, tree);
            }
            else
            {
                TexSubtreePrint(tex_file, node, node->right, tree);
            }

            break;

        case BI_OP:
            if ((opnum = FindOperation((int) VAL(node))) == ILL_OPNUM)
            {
                RET_ERROR(TEX_SUBT_PRINT_ERR, "No such operation (%d)\n", (int) VAL(node));
            }

            if (prev && OPERATIONS[FindOperation((int) VAL(prev))].priority >
                        OPERATIONS[FindOperation((int) VAL(node))].priority) //! here we assume that prev is operation node
            {
                print_parenthesis = 1;
            }

            if (print_parenthesis)
                fprintf(tex_file, " ( ");

            if (OPERATIONS[opnum].opcode == DIV)
                fprintf(tex_file, "\\frac");

            fprintf(tex_file, " { ");
            TexSubtreePrint(tex_file, node, node->left, tree);
            fprintf(tex_file, " } ");

            if (OPERATIONS[opnum].opcode != DIV)
                fprintf(tex_file, " %s ", OPERATIONS[opnum].name);

            fprintf(tex_file, " { ");
            TexSubtreePrint(tex_file, node, node->right, tree);
            fprintf(tex_file, " } ");

            if (print_parenthesis)
                fprintf(tex_file, " ) ");

            break;

        default:
            RET_ERROR(TEX_SUBT_PRINT_ERR, "Invalid node type \"%d\"\n", TYPE(node));

            break;
    }

    return TEX_SUBT_PRINT_SUCCESS;
}

// ============================================================================================

DotTreePrintRes DotSubtreePrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id)
{
    assert(dot_file);
    assert(tree);
    assert(node_id);
    if (!dot_file) RET_ERROR(DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree)     RET_ERROR(DOT_PRINT_ERR_PARAMS, "Tree null pointer");
    if (!node_id)  RET_ERROR(DOT_PRINT_ERR_PARAMS, "Node id null pointer");

    if (!node)
    {
        *node_id = 0;
        return DOT_PRINT_SUCCESS;
    }

    *node_id = rand();

    const char* color = "";
    char node_data[MAX_OP] = "";

    int opnum = 0;

    switch (TYPE(node))
    {
    case ERR:
        color = GRAPH_ERRCLR;
        sprintf(node_data, "ERR");

        break;

    case ROOT: // pinch of copypaste
        color = GRAPH_ERRCLR;
        opnum = FindOperation((int) VAL(node));
        sprintf(node_data, "%s", OPERATIONS[opnum].name);

        break;

    case NUM:
        color = GRAPH_NUMCLR;
        sprintf(node_data, "%.2lf", VAL(node));

        break;

    case VAR:
        color = GRAPH_VARCLR;
        sprintf (node_data, "%s", tree->nametable->names[(int) VAL(node)]); // get varname from nametable

        break;

    case UN_OP:
        color = GRAPH_OPCLR;

        opnum = FindOperation((int) VAL(node));
        sprintf(node_data, "%s", OPERATIONS[opnum].name);

        break;

    case BI_OP:
        color = GRAPH_OPCLR;

        opnum = FindOperation((int) VAL(node));
        sprintf(node_data, "%s", OPERATIONS[opnum].name);

        break;

    default:
        RET_ERROR(DOT_PRINT_ERR, "Unknown node type (%d)", TYPE(node));

        break;
    }

    fprintf (dot_file, "\tnode_%d [style = filled, shape = circle, label = \"%s\", fillcolor = \"%s\", fontcolor = \"%s\"];\n", *node_id, node_data, color, GRAPH_TEXTCLR);

    int left_subtree_id  = 0;
    int right_subtree_id = 0;

    if (DotSubtreePrint (dot_file, (const TreeNode *) node->left, tree, &left_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR(DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreePrint (dot_file, (const TreeNode *) node->right, tree, &right_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR(DOT_PRINT_ERR, "Previous function returned error code");

    if (left_subtree_id != 0)
        fprintf (dot_file, "\tnode_%d -> node_%d;\n", *node_id, left_subtree_id);

    if (right_subtree_id != 0)
        fprintf (dot_file, "\tnode_%d -> node_%d;\n", *node_id, right_subtree_id);

    return DOT_PRINT_SUCCESS;
}

// ============================================================================================

DotTreePrintRes DotTreeDetailedPrint (FILE* dot_file, const Tree* tree)
{
    assert(dot_file);
    assert(tree);
    if (!dot_file) RET_ERROR(DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree) RET_ERROR(DOT_PRINT_ERR_PARAMS, "Tree null pointer");

    int node_id = 0;

    return DotSubtreeDetailedPrint (dot_file, (const TreeNode *) tree->root, tree, &node_id);
}

// ============================================================================================

DotTreePrintRes DotSubtreeDetailedPrint (FILE* dot_file, const TreeNode* node, const Tree* tree, int* node_id)
{
    assert(dot_file);
    assert(tree);
    assert(node_id);
    if (!dot_file) RET_ERROR(DOT_PRINT_ERR_PARAMS, "Dot file null pointer");
    if (!tree)     RET_ERROR(DOT_PRINT_ERR_PARAMS, "Tree null pointer");
    if (!node_id)  RET_ERROR(DOT_PRINT_ERR_PARAMS, "Node id null pointer");

    if (!node)
    {
        *node_id = 0;

        return DOT_PRINT_SUCCESS;
    }

    *node_id = rand();

    const char* color = "";

    switch (TYPE(node))
    {
    case ERR:
        color = GRAPH_ERRCLR;
        break;

    case ROOT:
        color = GRAPH_ERRCLR;
        break;

    case NUM:
        color = GRAPH_NUMCLR;
        break;

    case VAR:
        color = GRAPH_VARCLR;
        break;

    case UN_OP:
        color = GRAPH_OPCLR;
        break;

    case BI_OP:
        color = GRAPH_OPCLR;
        break;

    default:
        RET_ERROR(DOT_PRINT_ERR, "Unknown node type (%d)", TYPE(node));

        break;
    }

    fprintf (dot_file, "\tdetailed_node_%d [style = filled, shape = record, fillcolor = \"%s\", fontcolor = \"%s\"];\n", *node_id, color, GRAPH_TEXTCLR);
    fprintf (dot_file, "\tdetailed_node_%d [label = \"{type = %d | val = %.3lf}\"];\n", *node_id, TYPE(node), VAL(node));

    int left_subtree_id  = 0;
    int right_subtree_id = 0;

    if (DotSubtreeDetailedPrint (dot_file, (const TreeNode *) node->left, tree, &left_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR(DOT_PRINT_ERR, "Previous function returned error code");

    if (DotSubtreeDetailedPrint (dot_file, (const TreeNode *) node->right, tree, &right_subtree_id) != DOT_PRINT_SUCCESS)
        RET_ERROR(DOT_PRINT_ERR, "Previous function returned error code");

    if (left_subtree_id != 0)
        fprintf (dot_file, "\tdetailed_node_%d -> detailed_node_%d;\n", *node_id, left_subtree_id);

    if (right_subtree_id != 0)
        fprintf (dot_file, "\tdetailed_node_%d -> detailed_node_%d;\n", *node_id, right_subtree_id);

    return DOT_PRINT_SUCCESS;
}

// ============================================================================================

char* GetFilePath(const char* path, const char* fname)
{
    assert(path);
    assert(fname);

    char* fpath = (char *) calloc(MAX_PATH, sizeof(char));

    strcat(fpath, path); // path must have / in the end
    strcat(fpath, fname);

    return fpath;
}
