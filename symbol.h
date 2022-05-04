#ifndef SYMBOL_H 
#define SYMBOL_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

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
    rule *rules;
    size_t n_rules;
};

struct rule
{
    const symbol **production;
    size_t n_symbols;
};

typedef struct
{
    symbol **symbols;
    size_t size;
    size_t n_symbols;

} symbol_table;

void init_rule(rule *rule);
void add_production(rule *rule, const symbol *symbol);
void clear_rule(rule *rule);

void init_symbol(symbol *symbol, char *name);
rule *add_rule(symbol *symbol);
void clear_symbol(symbol *symbol);
bool is_empty_symbol(const symbol *symbol);
bool is_nullable(const symbol *symbol);

void init_symbol_table(symbol_table *table, size_t start_size);
void add_symbol(symbol_table *table, symbol *symbol);
symbol *find_symbol(const symbol_table *table, char *name);
void clear_symbol_table(symbol_table *table);
bool create_symbol_table_from_file(symbol_table *table, FILE *file);

#endif
