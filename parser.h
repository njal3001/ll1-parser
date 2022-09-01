#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include "grammar.h"

typedef struct
{
    const grammar *grammar;
    bool *nullable_rules;
    bool *nullable_symbols;
    bool *rule_first_sets;
    bool *symbol_first_sets;
    bool *symbol_follow_sets;
    bool *table;
} parser;

void init_parser(parser *parser, const grammar *grammar);
void build_parse_table(parser *parser);
bool is_valid_grammar(const parser *parser);
bool is_valid_string(const parser *parser, const char *str);
void clear_parser(parser *parser);

void print_rules(const parser *parser);
void print_symbols(const parser *parser);
void print_table(const parser *parser);

#endif
