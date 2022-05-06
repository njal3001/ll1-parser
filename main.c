#include "symbol.h"
#include <stdlib.h>

int main()
{
    symbol_table table;
    init_symbol_table(&table, 16);

    if (!create_symbol_table_from_file(&table, stdin))
    {
        clear_symbol_table(&table);
        fputs("Error reading input\n", stderr);
        return EXIT_FAILURE;
    }

    compute_table_values(&table);

    for (size_t i = 0; i < table.symbols.elem_count; i++)
    {
        symbol *symbol = get_list_element(&table.symbols, i);
        printf("Name: %s\n", symbol->name);
        printf("Type: %s\n", symbol->type == TERMINAL ? "TERMINAL" : "NONTERMINAL");
        printf("Rules: %ld\n", symbol->rules.elem_count);
        printf("Nullable: %s\n", table.nullable_list[symbol->id] ? "YES" : "NO");

        if (symbol->type == NONTERMINAL)
        {
            size_t first_count = 0;
            for (size_t j = 0; j < table.symbols.elem_count; j++)
                first_count += table.first_sets[table.symbols.elem_count * i + j];

            printf("First: ");
            for (size_t j = 0; j < table.symbols.elem_count; j++)
            {
                if (table.first_sets[table.symbols.elem_count * i + j])
                {
                    struct symbol *in_first = get_list_element(&table.symbols, j);
                    printf("%s", in_first->name);

                    first_count--;
                    if (first_count > 0)
                    {
                        printf(", ");
                    }
                }
            }

            printf("\n");
        }

        printf("\n");
    }

    clear_symbol_table(&table);

    return EXIT_SUCCESS;
}
