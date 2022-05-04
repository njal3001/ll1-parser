#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdio.h>
#include <stdlib.h>

long file_size(FILE *file);
char *read_file(FILE *file);

#endif
