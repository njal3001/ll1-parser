#include "symbol.h"
#include <assert.h>
#include <string.h>
#include "file_util.h"
#include <stdlib.h>

void init_rule(rule *rule)
{
    init_list(&rule->production, 4, sizeof(symbol*));
}

void add_production(rule *rule, symbol *symbol)
{
    struct symbol **new_elem_ref = new_list_element(&rule->production);
    *new_elem_ref = symbol;
}

void clear_rule(rule *rule)
{
    clear_list(&rule->production);
}

void init_symbol(symbol *symbol, char *name)
{
    symbol->name = name;
    symbol->type = UNKNOWN;
    init_list(&symbol->rules, 0, sizeof(rule));
}

rule *add_rule(symbol *symbol)
{
    rule *rule = new_list_element(&symbol->rules);
    init_rule(rule);
    return rule;
}

void clear_symbol(symbol *symbol)
{
    free(symbol->name);
    for (size_t i = 0; i < symbol->rules.elem_count; i++)
        clear_rule(get_list_element(&symbol->rules, i));

    clear_list(&symbol->rules);
}

bool is_empty_symbol(const symbol *symbol)
{
    return strcmp(symbol->name, "\"") == 0;
}

// TODO: This breaks when handling recursive productions
bool is_nullable(const symbol *symbol)
{
    if (symbol->type == NONTERMINAL)
    {
        for (size_t i = 0; i < symbol->rules.elem_count; i++)
        {
            const rule *rule = get_list_element(&symbol->rules, i);

            bool nullable_production = true;
            for (size_t j = 0; j < rule->production.elem_count; j++)
            {
                struct symbol *rhs = *(struct symbol**)get_list_element(&rule->production, j);
                if (!is_nullable(rhs))
                    nullable_production = false;
            }

            if (nullable_production)
            {
                return true;
            }
        }

        return false;
    }
    else
    {
        return is_empty_symbol(symbol);
    }
}

void init_symbol_table(symbol_table *table, size_t start_size)
{
    init_list(&table->symbols, start_size, sizeof(symbol));
}

symbol *add_new_symbol(symbol_table *table, char *name)
{
    symbol *new_symbol = new_list_element(&table->symbols);
    init_symbol(new_symbol, name);
    return new_symbol;
}

symbol *find_symbol(const symbol_table *table, char *name)
{
    for (size_t i = 0; i < table->symbols.elem_count; i++)
    {
        symbol *symbol = get_list_element(&table->symbols, i);
        if (strcmp(symbol->name, name) == 0)
            return symbol;
    }

    return NULL;
}

void clear_symbol_table(symbol_table *table)
{
    for (size_t i = 0; i < table->symbols.elem_count; i++)
    {
        symbol *symbol = get_list_element(&table->symbols, i);
        clear_symbol(symbol);
    }

    clear_list(&table->symbols);
}

bool create_symbol_table_from_file(symbol_table *table, FILE *file)
{
    char *input_buffer = read_file(stdin);
    if (!input_buffer)
    {
        return false;
    }

    char *newline_split = input_buffer;
    char *save1 = NULL;
    strtok_r(newline_split, "\n", &save1);
    while (newline_split)
    {
        char *line_copy = strdup(newline_split);
        char *space_split = line_copy;
        char *save2 = NULL;
        size_t space_split_counter = 0;
        space_split = strtok_r(space_split, " ", &save2);

        symbol *lhs_symbol = NULL;
        rule *rule = NULL;
        while (space_split)
        {
            space_split_counter++;
            if (space_split_counter == 1)
            {
                // Left hand side

                // Check that name is not reserved
                if (strcmp(space_split, "\"") == 0 || strcmp(space_split, "$") == 0)
                {
                    free(line_copy);
                    free(input_buffer);
                    return false;
                }

                lhs_symbol = find_symbol(table, space_split);

                if (!lhs_symbol)
                {
                    lhs_symbol = add_new_symbol(table, strdup(space_split));
                }

                lhs_symbol->type = NONTERMINAL;
                rule = add_rule(lhs_symbol);
            }
            else if (space_split_counter == 2)
            {
                // Definition symbol
                if (strcmp(space_split, "::=") != 0)
                {
                    free(line_copy);
                    free(input_buffer);
                    return false;
                }
            }
            else
            {
                // Right hand side

                // Check that name is not reserved
                if (strcmp(space_split, "$") == 0)
                {
                    free(line_copy);
                    free(input_buffer);
                    return false;
                }

                symbol *rhs_symbol = find_symbol(table, space_split);

                if (!rhs_symbol)
                {
                    rhs_symbol = add_new_symbol(table, strdup(space_split));
                    rhs_symbol->type = TERMINAL;
                }

                add_production(rule, rhs_symbol);
            }

            space_split = strtok_r(NULL, " ", &save2);
        }

        free(line_copy);

        if (space_split_counter < 3)
        {
            free(input_buffer);
            return false;
        }

        newline_split = strtok_r(NULL, "\n", &save1);
    }

    free(input_buffer);

    // Add end of file symbol
    symbol *eof_symbol = add_new_symbol(table, strdup("$"));
    eof_symbol->type = TERMINAL;

    return true;
}
