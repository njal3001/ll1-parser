#include "grammar.h"
#include "file_util.h"
#include <stdlib.h>
#include <string.h>

void init_rule(rule *rule, symbol *lhs, int id)
{
    rule->lhs = lhs;
    rule->id = id;
    init_list(&rule->rhs, 4, sizeof(symbol *));
}

void add_production(rule *rule, symbol *symbol)
{
    struct symbol **new_elem_ref = new_list_element(&rule->rhs);
    *new_elem_ref = symbol;
}

void clear_rule(rule *rule)
{
    clear_list(&rule->rhs);
}

void init_symbol(symbol *symbol, char *name, int id)
{
    symbol->name = name;
    symbol->type = UNKNOWN;
    symbol->id = id;
}

void clear_symbol(symbol *symbol)
{
    free(symbol->name);
}

bool is_empty_symbol(const symbol *symbol)
{
    return strcmp(symbol->name, "\"") == 0;
}

void init_grammar(grammar *grammar, size_t start_size)
{
    init_list(&grammar->symbols, start_size, sizeof(symbol));
    init_list(&grammar->rules, start_size, sizeof(rule));
}

symbol *add_new_symbol(grammar *grammar, char *name)
{
    int id = grammar->symbols.count;
    symbol *new_symbol = new_list_element(&grammar->symbols);
    init_symbol(new_symbol, name, id);
    return new_symbol;
}

rule *add_new_rule(grammar *grammar, symbol *lhs)
{
    int id = grammar->rules.count;
    rule *new_rule = new_list_element(&grammar->rules);
    init_rule(new_rule, lhs, id);
    return new_rule;
}

symbol *find_symbol(const grammar *grammar, char *name)
{
    for (size_t i = 0; i < grammar->symbols.count; i++)
    {
        symbol *symbol = get_list_element(&grammar->symbols, i);
        if (strcmp(symbol->name, name) == 0)
            return symbol;
    }

    return NULL;
}

void clear_grammar(grammar *grammar)
{
    for (size_t i = 0; i < grammar->symbols.count; i++)
    {
        symbol *symbol = get_list_element(&grammar->symbols, i);
        clear_symbol(symbol);
    }

    clear_list(&grammar->symbols);

    for (size_t i = 0; i < grammar->rules.count; i++)
    {
        rule *rule = get_list_element(&grammar->rules, i);
        clear_rule(rule);
    }

    clear_list(&grammar->rules);
}

bool create_grammar_from_file(grammar *grammar, FILE *file)
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
                if (strcmp(space_split, "\"") == 0)
                {
                    free(line_copy);
                    free(input_buffer);
                    return false;
                }

                lhs_symbol = find_symbol(grammar, space_split);

                if (!lhs_symbol)
                {
                    lhs_symbol = add_new_symbol(grammar, strdup(space_split));
                }

                lhs_symbol->type = NONTERMINAL;
                rule = add_new_rule(grammar, lhs_symbol);
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
                symbol *rhs_symbol = find_symbol(grammar, space_split);

                if (!rhs_symbol)
                {
                    rhs_symbol = add_new_symbol(grammar, strdup(space_split));
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

    return true;
}
