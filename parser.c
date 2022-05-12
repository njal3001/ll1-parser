#include "parser.h"
#include <string.h>

static void compute_nullable(parser *parser);
static void compute_first(parser *parser);
static void compute_follow(parser *parser);

static bool or_all(bool *src, bool *dst, size_t n);

void init_parser(parser *parser, grammar *grammar)
{
    parser->grammar = grammar;
    init_list(&parser->stack, 8, sizeof(symbol *));

    parser->nullable_rules = malloc(grammar->rules.count * sizeof(bool));
    parser->nullable_symbols = malloc(grammar->symbols.count * sizeof(bool));
    memset((void *)parser->nullable_rules, 0, sizeof(parser->nullable_rules));
    memset((void *)parser->nullable_symbols, 0, sizeof(parser->nullable_symbols));

    parser->rule_first_sets = malloc(grammar->rules.count * grammar->symbols.count * sizeof(bool));
    parser->symbol_first_sets = malloc(grammar->symbols.count * grammar->symbols.count * sizeof(bool));
    memset((void *)parser->rule_first_sets, 0, sizeof(parser->rule_first_sets));
    memset((void *)parser->symbol_first_sets, 0, sizeof(parser->symbol_first_sets));

    parser->symbol_follow_sets = malloc(grammar->symbols.count * grammar->symbols.count * sizeof(bool));
    memset((void *)parser->symbol_follow_sets, 0, sizeof(parser->symbol_follow_sets));

    parser->table = malloc(grammar->symbols.count * grammar->symbols.count * grammar->rules.count * sizeof(bool));
    memset((void *)parser->table, 0, sizeof(parser->table));
}

void compute_nullable(parser *parser)
{
    // Start by saying that empty symbol is nullable
    for (size_t i = 0; i < parser->grammar->symbols.count; i++)
    {
        symbol *symbol = get_list_element(&parser->grammar->symbols, i);
        if (is_empty_symbol(symbol))
        {
            parser->nullable_symbols[symbol->id] = true;
            break;
        }
    }

    size_t n_rules = parser->grammar->rules.count;
    bool changed;
    do
    {
        changed = false;
        for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
        {
            rule *rule = get_list_element(&parser->grammar->rules, rule_index);
            if (!parser->nullable_rules[rule->id])
            {
                // Rule is nullable if all rhs symbols are nullable
                bool nullable_rule = true;
                for (size_t rhs_index = 0; rhs_index < rule->rhs.count; rhs_index++)
                {
                    symbol *rhs_symbol = *(symbol **)get_list_element(&rule->rhs, rhs_index);
                    if (!parser->nullable_symbols[rhs_symbol->id])
                    {
                        nullable_rule = false;
                        break;
                    }
                }

                if (nullable_rule)
                {
                    parser->nullable_rules[rule->id] = true;
                    parser->nullable_symbols[rule->lhs->id] = true;
                    changed = true;
                }
            }
        }
    } while (changed);
}

void compute_first(parser *parser)
{
    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;

    // Start by saying that all terminals contain themself in
    // in their first set (except the empty symbol)
    for (size_t i = 0; i < parser->grammar->symbols.count; i++)
    {
        symbol *symbol = get_list_element(&parser->grammar->symbols, i);
        if (symbol->type == TERMINAL && !is_empty_symbol(symbol))
            parser->symbol_first_sets[symbol->id * n_symbols + symbol->id] = true;
    }

    bool changed;
    do
    {
        changed = false;
        for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
        {
            rule *rule = get_list_element(&parser->grammar->rules, rule_index);
            for (size_t rhs_index = 0; rhs_index < rule->rhs.count; rhs_index++)
            {
                // Add all elements from first set of production symbol
                symbol *rhs_symbol = *(symbol **)get_list_element(&rule->rhs, rhs_index);
                for (size_t symbol_index = 0; symbol_index < n_symbols; symbol_index++)
                {
                    size_t rhs_first_set_index = rhs_symbol->id * n_symbols + symbol_index;
                    size_t rule_first_set_index = rule->id * n_symbols + symbol_index;

                    if (parser->symbol_first_sets[rhs_first_set_index] &&
                        !parser->rule_first_sets[rule_first_set_index])
                    {
                        parser->rule_first_sets[rule_first_set_index] = true;
                        parser->symbol_first_sets[rule->lhs->id * n_symbols + symbol_index] = true;
                        changed = true;
                    }
                }

                // Only go to next production symbol if this one is nullable
                if (!parser->nullable_symbols[rhs_symbol->id])
                    break;
            }
        }
    } while (changed);
}

void compute_follow(parser *parser)
{
    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;

    bool changed;
    do
    {
        changed = false;
        for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
        {
            rule *rule = get_list_element(&parser->grammar->rules, rule_index);
            for (size_t rhs_index = 0; rhs_index < rule->rhs.count - 1; rhs_index++)
            {
                symbol *rhs_symbol = *(symbol **)get_list_element(&rule->rhs, rhs_index);
                if (rhs_symbol->type == TERMINAL)
                    continue;

                symbol *next_rhs_symbol = *(symbol **)get_list_element(&rule->rhs, rhs_index + 1);

                // Add all elements from first set of next symbol in production
                if (or_all(parser->symbol_first_sets + next_rhs_symbol->id * n_symbols,
                           parser->symbol_follow_sets + rhs_symbol->id * n_symbols, n_symbols))
                {
                    changed = true;
                }

                // If the next symbol is nullable, add it's follow elements
                if (parser->nullable_symbols[next_rhs_symbol->id])
                {
                    if (or_all(parser->symbol_follow_sets + next_rhs_symbol->id * n_symbols,
                               parser->symbol_follow_sets + rhs_symbol->id * n_symbols, n_symbols))
                    {
                        changed = true;
                    }
                }
            }

            // The elements following lhs of this rule will follow the last symbol
            // in the production
            symbol *last_rhs_symbol = *(symbol **)get_list_element(&rule->rhs, rule->rhs.count - 1);
            if (last_rhs_symbol->type == NONTERMINAL)
            {
                if (or_all(parser->symbol_follow_sets + rule->lhs->id * n_symbols,
                           parser->symbol_follow_sets + last_rhs_symbol->id * n_symbols, n_symbols))
                {
                    changed = true;
                }
            }
        }
    } while (changed);
}

void build_parse_table(parser *parser)
{
    compute_nullable(parser);
    compute_first(parser);
    compute_follow(parser);

    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;

    for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
    {
        rule *rule = get_list_element(&parser->grammar->rules, rule_index);
        for (size_t symbol_index = 0; symbol_index < n_symbols; symbol_index++)
        {
            if (parser->rule_first_sets[rule->id * n_symbols + symbol_index])
            {
                parser->table[rule->lhs->id * n_symbols * n_rules + symbol_index * n_rules + rule->id] = true;
            }
        }

        if (parser->nullable_rules[rule->id])
        {
            for (size_t symbol_index = 0; symbol_index < n_symbols; symbol_index++)
            {
                if (parser->symbol_follow_sets[rule->lhs->id * n_symbols])
                {
                    parser->table[rule->lhs->id * n_symbols * n_rules + symbol_index * n_rules + rule->id] = true;
                }
            }
        }
    }
}

bool is_valid_grammar(parser *parser)
{
    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;
    for (size_t row = 0; row < n_symbols; row++)
    {
        symbol *row_symbol = get_list_element(&parser->grammar->symbols, row);
        if (row_symbol->type == TERMINAL)
            continue;

        for (size_t col = 0; col < n_symbols; col++)
        {
            symbol *col_symbol = get_list_element(&parser->grammar->symbols, col);
            if (col_symbol->type == NONTERMINAL)
                continue;

            bool has_entry = false;
            for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
            {

                if (parser->table[row * n_symbols * n_rules + col * n_rules + rule_index])
                {
                    if (has_entry)
                        return false;

                    has_entry = true;
                }
            }
        }
    }

    return true;
}

void clear_parser(parser *parser)
{
    clear_list(&parser->stack);

    free(parser->nullable_rules);
    free(parser->nullable_symbols);
    free(parser->rule_first_sets);
    free(parser->symbol_first_sets);
    free(parser->symbol_follow_sets);
    free(parser->table);
}

bool or_all(bool *src, bool *dst, size_t n)
{
    bool changed = false;
    for (size_t i = 0; i < n; i++)
    {
        if (src[i] && !dst[i])
        {
            dst[i] = true;
            changed = true;
        }
    }

    return changed;
}
