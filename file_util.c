#include "file_util.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

long file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

char *read_file(FILE *file)
{
    long size = file_size(file);
    if (size <= 0)
    {
        return NULL;
    }

    char *buffer = malloc(sizeof(char) * (size + 1));
    size_t n_read = fread(buffer, sizeof(char), size, file);
    if (ferror(stdin))
    {
        free(buffer);
        return NULL;
    }

    buffer[n_read++] = '\0';
    return buffer;
}
