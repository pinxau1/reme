#ifndef CLIENT
#define CLIENT

#include "crud.h"

int clientInit(const char *toSend);
int serverInit();
int sockSend(const char *string);

#endif
