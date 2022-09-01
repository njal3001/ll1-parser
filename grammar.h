#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>
#include <stdio.h>
#include "list.h"

typedef enum
{
    UNKNOWN,
    TERMINAL,
    NONTERMINAL
} symbol_type;

typedef struct symbol symbol;
typedef struct rule rule;

struct symbol
{
    char *name;
    symbol_type type;
    int id;
};

struct rule
{
    symbol *lhs;
    list rhs;
    int id;
};

typedef struct
{
    list symbols;
    list rules;
} grammar;

void init_rule(rule *rule, symbol *lhs, int id);
void add_production(rule *rule, symbol *symbol);
void clear_rule(rule *rule);

void init_symbol(symbol *symbol, char *name, int id);
void clear_symbol(symbol *symbol);
bool is_empty_symbol(const symbol *symbol);
bool is_end_symbol(const symbol *symbol);

void init_grammar(grammar *grammar, size_t start_size);
symbol *add_new_symbol(grammar *grammar, char *name);
rule *add_new_rule(grammar *grammar, symbol *lhs);
symbol *find_symbol(const grammar *grammar, const char *name);
void clear_grammar(grammar *table);
bool create_grammar_from_file(grammar *grammar, FILE *file);

#endif
