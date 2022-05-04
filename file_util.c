#include "file_util.h"

long file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

char *read_file(FILE *file)
{
    long size = file_size(stdin);
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
