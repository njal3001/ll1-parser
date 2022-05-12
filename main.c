#include "grammar.h"
#include "parser.h"
#include <stdlib.h>

int main()
{
    grammar grammar;
    init_grammar(&grammar, 16);

    if (!create_grammar_from_file(&grammar, stdin))
    {
        clear_grammar(&grammar);
        fputs("Error reading input\n", stderr);
        return EXIT_FAILURE;
    }

    parser parser;
    init_parser(&parser, &grammar);
    build_parse_table(&parser);
    
    printf("Rules:\n");
    for (size_t i = 0; i < grammar.rules.count; i++)
    {
        rule *rule = get_list_element(&grammar.rules, i);
        printf("\t%s ::=", rule->lhs->name);
        for (size_t j = 0; j < rule->rhs.count; j++)
        {
            symbol *rhs_symbol = *(symbol **)get_list_element(&rule->rhs, j);
            printf(" %s", rhs_symbol->name);
        }
        printf("\n");
    }

    printf("\nSymbols:\n");
    for (size_t i = 0; i < grammar.symbols.count; i++)
    {
        symbol *symbol = get_list_element(&grammar.symbols, i);
        printf("\tName: %s\n", symbol->name);
        printf("\t\tType: %s\n", symbol->type == TERMINAL ? "terminal" : "nonterminal");
        printf("\t\tNullable: %d\n", parser.nullable_symbols[symbol->id]);
        printf("\t\tFirst:");
        for (size_t first_index = 0; first_index < grammar.symbols.count; first_index++)
        {
            if (parser.symbol_first_sets[symbol->id * grammar.symbols.count + first_index])
            {
                struct symbol *first_symbol = get_list_element(&grammar.symbols, first_index);
                printf(" %s", first_symbol->name);
            }
        }

        printf("\n\t\tFollow:");
        for (size_t follow_index = 0; follow_index < grammar.symbols.count; follow_index++)
        {
            if (parser.symbol_follow_sets[symbol->id * grammar.symbols.count + follow_index])
            {
                struct symbol *follow_symbol = get_list_element(&grammar.symbols, follow_index);
                printf(" %s", follow_symbol->name);
            }
        }
        printf("\n");
    }

    if (!is_valid_grammar(&parser))
        printf("Grammar is not LL(1)\n");

    clear_parser(&parser);
    clear_grammar(&grammar);

    return EXIT_SUCCESS;
}
