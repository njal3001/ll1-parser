#include "list.h"
#include <string.h>

void init_list(list *list, size_t start_size, size_t elem_byte_size)
{
    if (start_size == 0)
    {
        list->head = NULL;
    }
    else
    {
        list->head = malloc(start_size * elem_byte_size);
    }

    list->size = start_size;
    list->elem_byte_size = elem_byte_size;
}

void *new_list_element(list *list)
{
    if (list->elem_count == list->size)
    {
        list->size = (list->size + 1) * 2;
        list->head = realloc(list->head, list->size * list->elem_byte_size);
    }

    void *element = get_list_element(list, list->elem_count);
    list->elem_count++;
    return element;
}

void *get_list_element(const list *list, size_t index)
{
    return ((char*)list->head) + index * list->elem_byte_size;
}

void clear_list(list *list)
{
    free(list->head);

    list->head = NULL;
    list->elem_count = 0;
    list->size = 0;
}
