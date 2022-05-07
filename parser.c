#include "parser.h"

bool init_parser(parser *parser, symbol_table *table)
{
    if (!is_parsable(table))
        return false;

    parser->state = 0;
    parser->state_index = 0;
    parser->table = table;

    return true;
}

bool is_parsable(symbol_table *table)
{
    for (size_t i = 0; i < table->symbols.elem_count; i++)
    {
        symbol *symbol = get_list_element(&table->symbols, i);
        if (symbol->type == NONTERMINAL && table->nullable_list[symbol->id])
        {
            for (size_t j = 0; j < table->symbols.elem_count; j++)
            {
                size_t index = table->symbols.elem_count * symbol->id + j;
                if (table->first_sets[index] && table->follow_sets[index])
                    return false;
            }
        }
    }

    return true;
}
