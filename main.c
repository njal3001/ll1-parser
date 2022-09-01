#include "grammar.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

void read_input(char *buffer, size_t n)
{
    memset(buffer, 0, n);
    fgets(buffer, n, stdin);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fputs("ERROR: wrong number of arguments\n", stderr);
        exit(EXIT_FAILURE);
    }

    FILE *grammar_file = fopen(argv[1], "r");
    if (!grammar_file)
    {
        fputs("ERROR: could not open file\n", stderr);
        exit(EXIT_FAILURE);
    }

    grammar grammar;
    init_grammar(&grammar, 16);

    if (!create_grammar_from_file(&grammar, grammar_file))
    {
        fclose(grammar_file);
        clear_grammar(&grammar);
        fputs("Error reading input\n", stderr);
        return EXIT_FAILURE;
    }

    fclose(grammar_file);

    parser parser;
    init_parser(&parser, &grammar);
    build_parse_table(&parser);

    print_rules(&parser);
    putc('\n', stdout);
    print_symbols(&parser);
    putc('\n', stdout);
    print_table(&parser);
    putc('\n', stdout);

    if (is_valid_grammar(&parser))
    {
        char input[1024];
        while (1)
        {
            printf("Your string: ");
            fflush(stdout);
            read_input(input, sizeof(input));

            if (input[0] == '\0') break;

            // Remove trailing newline
            input[strcspn(input, "\n")] = 0;

            if (is_valid_string(&parser, input))
                printf("Valid string\n");
            else
                printf("Invalid string\n");
        };
    }
    else
    {
        printf("Grammar is not LL(1)\n");
    }

    clear_parser(&parser);
    clear_grammar(&grammar);

    return EXIT_SUCCESS;
}
