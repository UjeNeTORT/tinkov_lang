/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#include "middleend.h"

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

    TreeDotDump ("dump_simplified.html", ast);

    if (TreeSimplify (ast) != TREE_SIMPLIFY_SUCCESS)
    {
        WARN ("MIDDLEEND FAILED! UNSIMPLIFIED TREE IS BEING USED...");
        TreeDtor (ast);

        return 0;
    }

    FILE* simplified_tree_file = fopen (ast_name, "wb");
    WriteTree (simplified_tree_file, ast);
    fclose (simplified_tree_file);

    TreeDtor (ast);

    PRINTF_DEBUG ("MIDDLEEND ok");

    return 0;
}
