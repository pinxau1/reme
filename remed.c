#include "daemonize.h"
#include "socket.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

char *extractDetails(const char *string) {
  char *retString = malloc(strlen(string));
  int i = 0;

  while (string[i++] != '|') {
  }

  memcpy(retString, string + i, strlen(string + i));
  retString[strlen(string + i)] = '\0';
  return retString;
}

int main() {

  daemonize();
  serverInit();

  return 0;
}
