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
    list rules;
    int id;
};

struct rule
{
    list production;
};

typedef struct
{
    list symbols;
    int *nullable_list;
    list *first_sets;
    list *follow_sets;
} symbol_table;

void init_rule(rule *rule);
void add_production(rule *rule, symbol *symbol);
void clear_rule(rule *rule);

void init_symbol(symbol *symbol, char *name, int id);
rule *add_rule(symbol *symbol);
void clear_symbol(symbol *symbol);
bool is_empty_symbol(const symbol *symbol);

void init_symbol_table(symbol_table *table, size_t start_size);
symbol *add_new_symbol(symbol_table *table, char *name);
symbol *find_symbol(const symbol_table *table, char *name);
void compute_table_values(symbol_table *table);
void clear_symbol_table(symbol_table *table);
bool create_symbol_table_from_file(symbol_table *table, FILE *file);

#endif
