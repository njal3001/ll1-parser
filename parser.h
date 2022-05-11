#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#include <stdlib.h>
#include "grammar.h"

typedef struct
{
    grammar *grammar;
    list stack;
    bool *nullable_rules;
    bool *nullable_symbols;
    bool *rule_first_sets;
    bool *symbol_first_sets;
    bool *symbol_follow_sets;
} parser;

void init_parser(parser *parser, grammar *grammar);
void build_parse_table(parser *parser);
void clear_parser(parser *parser);

#endif
