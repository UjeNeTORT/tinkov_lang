/*************************************************************************
 * (c) 2023 Tikhonov Yaroslav (aka UjeNeTORT)
 *
 * email:    tikhonovty@gmail.com
 * telegram: https://t.me/netortofficial
 * GitHub:   https://github.com/UjeNeTORT
 * repo:     https://github.com/UjeNeTORT/language
 *************************************************************************/

#include "compiler.h"

int main (int argc, char* argv[])
{
    if (argc < 2)
        RET_ERROR (1, "Filename not fiven!");

    return CompileTinkovProgram (argv[1]);
}

int CompileTinkovProgram (char* filename)
{
    assert (filename);

    char* command = (char*) calloc (MAX_COMMAND, sizeof (char));
    sprintf (command, "./frontend %s\n", filename);
    assert (system (command) == 0);

    sprintf (command, "./backend ast.ast\n");
    assert (system (command) == 0);

    sprintf (command, "./asm --finname out.tinkov\n");
    assert (system (command) == 0);

    sprintf (command, "./spu --finname translated.bin\n");
    assert (system (command) == 0);

    free (command);

    return 0;
}
