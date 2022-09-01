#include "parser.h"
#include <string.h>

static void compute_nullable(parser *parser);
static void compute_first(parser *parser);
static void compute_follow(parser *parser);

static bool or_all(const bool *src, bool *dst, size_t n);
static bool *create_bool_arr(size_t size);

static rule *get_matching_rule(const parser *parser, const symbol *symbol, const char *token);

void init_parser(parser *parser, const grammar *grammar)
{
    parser->grammar = grammar;
    parser->nullable_rules = create_bool_arr(grammar->rules.count);
    parser->nullable_symbols = create_bool_arr(grammar->symbols.count);

    parser->rule_first_sets = create_bool_arr(grammar->rules.count * grammar->symbols.count);
    parser->symbol_first_sets = create_bool_arr(grammar->symbols.count * grammar->symbols.count);
    parser->symbol_follow_sets = create_bool_arr(grammar->symbols.count * grammar->symbols.count);

    parser->table = create_bool_arr(
            grammar->symbols.count * grammar->symbols.count * grammar->rules.count);
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
                if (parser->symbol_follow_sets[rule->lhs->id * n_symbols + symbol_index])
                {
                    parser->table[rule->lhs->id * n_symbols * n_rules + symbol_index * n_rules + rule->id] = true;
                }
            }
        }
    }
}

bool is_valid_grammar(const parser *parser)
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

bool is_valid_string(const parser *parser, const char *str)
{
    list stack;
    init_list(&stack, 16, sizeof(symbol*));

    // Add starting symbol to stack
    symbol **start_sym = new_list_element(&stack);
    *start_sym = get_list_element(&parser->grammar->symbols, 0);

    char *token = strdup(str);
    char *token_ref = token;
    strtok(token, " ");

    while (token || stack.count > 0)
    {
        symbol *sym = *(symbol**)get_list_element(&stack, 0);
        if (sym->type == TERMINAL)
        {
            if (!is_empty_symbol(sym) && !is_end_symbol(sym))
            {
                if (!token || strcmp(sym->name, token) != 0)
                {
                    break;
                }

                token = strtok(NULL, " ");
            }

            pop_front(&stack);
        }
        else
        {
            rule *rule;
            if (token)
                rule = get_matching_rule(parser, sym, token);
            else
                rule = get_matching_rule(parser, sym, "$");

            if (!rule) break;

            pop_front(&stack);
            for (int rhs_index = rule->rhs.count - 1; rhs_index >= 0; rhs_index--)
            {
                symbol **new_sym = push_front(&stack);
                *new_sym = *(symbol**)get_list_element(&rule->rhs, rhs_index);
            }
        }
    }

    bool success = !token && stack.count == 0;

    free(token_ref);
    clear_list(&stack);

    return success;
}

rule *get_matching_rule(const parser *parser, const symbol *symbol, const char *token)
{
    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;

    const struct symbol *token_symbol = find_symbol(parser->grammar, token);
    if (token_symbol)
    {
        for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
        {
            if (parser->table[symbol->id * n_symbols * n_rules +
                    token_symbol->id * n_rules + rule_index])
            {
                return get_list_element(&parser->grammar->rules, rule_index);
            }
        }
    }

    return NULL;
}

void clear_parser(parser *parser)
{
    free(parser->nullable_rules);
    free(parser->nullable_symbols);
    free(parser->rule_first_sets);
    free(parser->symbol_first_sets);
    free(parser->symbol_follow_sets);
    free(parser->table);
}

bool or_all(const bool *src, bool *dst, size_t n)
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

bool *create_bool_arr(size_t size)
{
    bool *arr = malloc(size * sizeof(bool));
    memset((void *)arr, 0, size * sizeof(bool));
    return arr;
}

void print_rule(const rule *rule)
{
    printf("\t%s ::=", rule->lhs->name);
    for (size_t j = 0; j < rule->rhs.count; j++)
    {
        symbol *rhs_symbol = *(symbol **)get_list_element(&rule->rhs, j);
        printf(" %s", rhs_symbol->name);
    }
}

void print_rules(const parser *parser)
{
    const grammar *grammar = parser->grammar;
    printf("Rules:\n");
    for (size_t i = 0; i < grammar->rules.count; i++)
    {
        rule *rule = get_list_element(&grammar->rules, i);
        print_rule(rule);
        putc('\n', stdout);
    }
}

void print_symbols(const parser *parser)
{
    const grammar *grammar = parser->grammar;
    printf("Symbols:\n");
    for (size_t i = 0; i < grammar->symbols.count; i++)
    {
        symbol *symbol = get_list_element(&grammar->symbols, i);
        printf("\tName: %s\n", symbol->name);
        printf("\t\tType: %s\n", symbol->type == TERMINAL ? "terminal" : "nonterminal");
        printf("\t\tNullable: %d\n", parser->nullable_symbols[symbol->id]);
        printf("\t\tFirst:");
        for (size_t first_index = 0; first_index < grammar->symbols.count; first_index++)
        {
            if (parser->symbol_first_sets[symbol->id * grammar->symbols.count + first_index])
            {
                struct symbol *first_symbol = get_list_element(&grammar->symbols, first_index);
                printf(" %s", first_symbol->name);
            }
        }

        printf("\n\t\tFollow:");
        for (size_t follow_index = 0; follow_index < grammar->symbols.count; follow_index++)
        {
            if (parser->symbol_follow_sets[symbol->id * grammar->symbols.count + follow_index])
            {
                struct symbol *follow_symbol = get_list_element(&grammar->symbols, follow_index);
                printf(" %s", follow_symbol->name);
            }
        }
        putc('\n', stdout);
    }

}

void print_table(const parser *parser)
{
    size_t n_symbols = parser->grammar->symbols.count;
    size_t n_rules = parser->grammar->rules.count;

    printf("Table:\n");

    for (size_t row = 0; row < n_symbols; row++)
    {
        const symbol *row_symbol = get_list_element(&parser->grammar->symbols, row);
        if (row_symbol->type != NONTERMINAL) continue;

        printf("\t%s:\n", row_symbol->name);
        for (size_t col = 0; col < n_symbols; col++)
        {
            const symbol *col_symbol = get_list_element(&parser->grammar->symbols, col);
            if (col_symbol->type != TERMINAL) continue;

            printf("\t\t%s: ", col_symbol->name);
            for (size_t rule_index = 0; rule_index < n_rules; rule_index++)
            {
                if (parser->table[row * n_symbols * n_rules + col * n_rules + rule_index])
                {
                    print_rule(get_list_element(&parser->grammar->rules, rule_index));
                    putc(' ', stdout);
                }
            }

            putc('\n', stdout);
        }
    }
}
