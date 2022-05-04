#include "symbol.h"

int main()
{
    symbol_table table;
    init_symbol_table(&table, 16);

    if (!create_symbol_table_from_file(&table, stdin))
    {
        fputs("Error reading input\n", stderr);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < table.n_symbols; i++)
    {
        symbol *symbol = table.symbols[i];
        printf("Name: %s\n", symbol->name);
        printf("Type: %s\n", symbol->type == TERMINAL ? "TERMINAL" : "NONTERMINAL");
        printf("Rules: %ld\n\n", symbol->n_rules);
    }

    clear_symbol_table(&table);

    return EXIT_SUCCESS;
}
