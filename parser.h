#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#include <stdlib.h>
#include "symbol.h"

typedef struct
{
    size_t state;
    size_t state_index;
    symbol_table *table;
} parser;

bool init_parser(parser *parser, symbol_table *table);
bool is_parsable(symbol_table *table);
void clear_parser(parser *parser);

#endif
