#include "frontend.h"


int main()
{
    char* txt = (char*) calloc(56, sizeof(char));
    strcpy (txt, "$ x + y $ / 60 + 2 сомнительно_но_окей");

    ProgText text = {txt, 0, 3};

    ProgCode* prog_code = LexicalAnalysisTokenize (&text);

    PRINTF_DEBUG("done");

    return 0;
}

TreeNode* GetNumber (ProgCode* code, Tree* tree)
{
    return NULL;
}

TreeNode* GetVar (ProgCode* code, Tree* tree)
{
    return NULL;
}

ProgCode* LexicalAnalysisTokenize (ProgText* text)
{
    assert(text);

    ProgCode* prog_code = ProgCodeCtor ();

    int n_readen = 0;

    char word[MAX_TOKEN] = "";

    while (sscanf(text->text + text->offset, "%s%n", word, &n_readen))
    {
        PRINTF_DEBUG("%s", text->text + text->offset);
        text->offset += n_readen;

        TreeNode* new_node = NULL;

        // the whole statement is quite unoptimal because many functions duplicate each other
        if (IsIdentifier (word))
        {
            PRINTF_DEBUG ("prog_code->nametable[%p]\n", prog_code->nametable);
            new_node = TreeNodeCtor (GetIdentifierIndex (word, prog_code->nametable), IDENTIFIER, NULL, NULL, NULL);
        }

        else if (IsKeyword (word)) // unoptimal, requires 2 cycles
        {
            new_node = TreeNodeCtor (GetKeywordIndex (word), KEYWORD, NULL, NULL, NULL);
        }

        else if (IsSeparator (word))
        {
            new_node = TreeNodeCtor (GetSeparatorIndex (word), SEPARATOR, NULL, NULL, NULL);
        }

        else if (IsOperator (word))
        {
            new_node = TreeNodeCtor (GetOperatorIndex (word), SEPARATOR, NULL, NULL, NULL);
        }

        else if (IsIntLiteral (word))
        {
            new_node = TreeNodeCtor (atoi (word), INT_LITERAL, NULL, NULL, NULL);
        }

        else
        {
            ProgCodeDtor(prog_code);

            RET_ERROR(NULL, "Unknown lexem \"%s\"", word);
        }

        prog_code->tokens[prog_code->size++] = new_node;
    }

    return prog_code;
}

int IsIdentifier (const char* word)
{
    assert (word);

    if (!isalpha(*word)) return 0;

    while (*++word)
        if (!isalnum(*word) && *word != '_') return 0;

    return 1;
}

int IsKeyword (const char* word)
{
    assert (word);

    for (int i = 0; i < N_KEYWORDS; i++)
    {
        if (streq (word, KEYWORDS[i]))
            return 1;
    }

    return 0;
}

int IsSeparator  (const char* word)
{
    assert (word);

    for (int i = 0; i < N_SEPARATORS; i++)
    {
        if (streq (word, SEPARATORS[i]))
            return 1;
    }

    return 0;
}

int IsOperator (const char* word)
{
    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (word, OPERATORS[i]))
            return 1;
    }

    return 0;
}

int IsIntLiteral (const char* word)
{
    assert (word);

    if (atoi (word) != 0)
        return 1;

    if (atoi (word) == 0 && word[0] == '0') // temporary, does not cover many cases
        return 1;

    return 0;
}

int GetIdentifierIndex (const char* identifier, NameTable* nametable)
{
    int id_index = FindVarInNametable (identifier, nametable);
    if (id_index  != -1)
        return id_index;

    return UpdNameTable (identifier, nametable);
}

int GetKeywordIndex (const char* keyword)
{
    assert (keyword);

    for (int i = 0; i < N_KEYWORDS; i++)
    {
        if (streq (keyword, KEYWORDS[i]))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

int GetSeparatorIndex  (const char* separator)
{
    assert (separator);

    for (int i = 0; i < N_SEPARATORS; i++)
    {
        if (streq (separator, SEPARATORS[i]))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

int GetOperatorIndex (const char* operator_)
{
    assert (operator_);

    for (int i = 0; i < N_OPERATORS; i++)
    {
        if (streq (operator_, OPERATORS[i]))
            return i;
    }

    return -1; // this is unlikely to happen, but if this happens it is not handled
}

ProgCode* ProgCodeCtor ()
{
    ProgCode* prog_code = (ProgCode*) calloc (1, sizeof(ProgCode));

    prog_code->nametable = NameTableCtor ();

    prog_code->tokens = (TreeNode**) calloc (MAX_N_NODES, sizeof(TreeNode*));
    prog_code->size = 0;


    return prog_code;
}

int ProgCodeDtor (ProgCode* prog_code)
{
    assert (prog_code);

    NameTableDtor(prog_code->nametable);

    for (int i = 0; prog_code->tokens[i] && i < prog_code->size; i++)
        free (prog_code->tokens[i]);


    free (prog_code);
}
