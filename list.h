#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>

typedef struct
{
    void *head;
    size_t elem_byte_size;
    size_t elem_count;
    size_t size;
} list;

void init_list(list *list, size_t start_size, size_t elem_byte_size);
void *new_list_element(list *list);
void *get_list_element(const list *list, size_t index);
void clear_list(list *list);

#endif
