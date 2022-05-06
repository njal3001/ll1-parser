#include "symbol.h"
#include "file_util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void compute_nullability(symbol_table *table);
static void compute_first(const symbol_table *table);
static void compute_follow(const symbol_table *table);

void init_rule(rule *rule)
{
    init_list(&rule->production, 4, sizeof(symbol *));
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

void init_symbol(symbol *symbol, char *name, int id)
{
    symbol->name = name;
    symbol->type = UNKNOWN;
    symbol->id = id;
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

void init_symbol_table(symbol_table *table, size_t start_size)
{
    init_list(&table->symbols, start_size, sizeof(symbol));
    table->nullable_list = NULL;
    table->first_sets = NULL;
    table->follow_sets = NULL;
}

symbol *add_new_symbol(symbol_table *table, char *name)
{
    int id = table->symbols.elem_count;
    symbol *new_symbol = new_list_element(&table->symbols);
    init_symbol(new_symbol, name, id);
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

static void compute_nullability(symbol_table *table)
{
    memset((void *)table->nullable_list, 0, sizeof(table->nullable_list));

    // Set empty terminal as nullable
    for (size_t sn = 0; sn < table->symbols.elem_count; sn++)
    {
        symbol *symbol = get_list_element(&table->symbols, sn);
        if (is_empty_symbol(symbol))
            table->nullable_list[symbol->id] = true;
    }

    bool changed;
    do
    {
        changed = false;
        for (size_t sn = 0; sn < table->symbols.elem_count; sn++)
        {
            symbol *symbol = get_list_element(&table->symbols, sn);
            if (symbol->type == NONTERMINAL && !table->nullable_list[symbol->id])
            {
                for (size_t rn = 0; rn < symbol->rules.elem_count; rn++)
                {
                    rule *rule = get_list_element(&symbol->rules, rn);
                    bool nullable_production = true;
                    for (size_t pn = 0; pn < rule->production.elem_count; pn++)
                    {
                        struct symbol *production_elem = *(struct symbol **)get_list_element(&rule->production, pn);
                        nullable_production &= table->nullable_list[production_elem->id];
                    }

                    if (nullable_production)
                    {
                        table->nullable_list[symbol->id] = true;
                        changed = true;
                        break;
                    }
                }
            }
        }

    } while (changed);
}

void compute_first(const symbol_table *table)
{
}

void compute_follow(const symbol_table *table)
{
}

void compute_table_values(symbol_table *table)
{
    table->nullable_list = malloc(table->symbols.elem_count * sizeof(int));
    table->first_sets = malloc(table->symbols.elem_count * sizeof(list));
    table->follow_sets = malloc(table->symbols.elem_count * sizeof(list));

    compute_nullability(table);
    compute_first(table);
    compute_follow(table);
}

void clear_symbol_table(symbol_table *table)
{
    for (size_t i = 0; i < table->symbols.elem_count; i++)
    {
        symbol *symbol = get_list_element(&table->symbols, i);
        clear_symbol(symbol);
    }

    clear_list(&table->symbols);

    if (table->nullable_list)
        free(table->nullable_list);
    if (table->first_sets)
    {
        clear_list(table->first_sets);
        free(table->first_sets);
    }
    if (table->follow_sets)
    {
        clear_list(table->follow_sets);
        free(table->follow_sets);
    }
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
