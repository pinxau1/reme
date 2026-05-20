#ifndef CRUD
#define CRUD

#include <stdio.h>

enum Operation { ADD, LIST, DELETE, UPDATE };

const char *crudInterpret(FILE *file, const char *buffer);

#endif
