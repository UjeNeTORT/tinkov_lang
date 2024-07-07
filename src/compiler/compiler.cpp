/*************************************************************************
 * (c) 2024 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/tinkov_lang
 *************************************************************************/

#include "compiler.h"

int main (int argc, char* argv[])
{
    if (argc < 2)
        RET_ERROR (1, "Filename not fiven!");

    return CompileTinkovProgram (argv[1]);
}

DefaultFuncRes CompileTinkovProgram (char* filename)
{
    assert (filename);

    if (!IsProgramTextFilenameCorrect(filename))
    {
        RET_ERROR (FUNC_ERROR, "Incorrect filename %s, extension .tnkff awaited\n", filename);
    }

    char* command = (char*) calloc (MAX_COMMAND, sizeof (char));

    sprintf (command, "./frontend %s\n", filename);
    assert (system (command) == 0 && "Failed to run frontend");

    sprintf (command, "./middleend ast.ast\n");
    assert (system (command) == 0 && "Failed to run middleend");

    sprintf (command, "./backend ast.ast\n");
    assert (system (command) == 0 && "Failed to backend");

    sprintf (command, "nasm -f elf64 -g out.s -o out.o\n");
    assert (system (command) == 0 && "Failed to generate binary");

    sprintf (command, "g++ -no-pie out.o -o exec\n");
    assert (system (command) == 0 && "Failed to generate executable");

    free (command);

    return FUNC_SUCCESS;
}

DefaultFuncRes IsProgramTextFilenameCorrect (const char* program_text_filename)
{
    assert (program_text_filename);

    while (*program_text_filename++ != '.')
        ;

    if (streq (program_text_filename, "tnkff"))
        return FUNC_ERROR;

    return FUNC_SUCCESS;
}
