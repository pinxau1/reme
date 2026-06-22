#ifndef CLIENT
#define CLIENT

#include "crud.h"
#include <stddef.h>

int clientInit(const struct Request *request);
int writeAll(int fd, const void *data, size_t size);
int readAll(int fd, void *data, size_t size);
int sockSend(const struct Request *request);

#endif
