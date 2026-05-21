#include "socket.h"
#include "crud.h"
#include "remed.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PATH "/tmp/reme.sock"

int clientInit(const char *toSend) {
  int ret = 0;
  int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);

  if (clientSock == -1) {
    ret = -1;
    goto close_client;
  }

  struct sockaddr_un serverAddr;

  strncpy(serverAddr.sun_path, PATH, sizeof(serverAddr.sun_path) - 1);
  serverAddr.sun_family = AF_UNIX;

  if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
    ret = -1;
    goto close_client;
  }

  if (send(clientSock, toSend, strlen(toSend), 0) == -1) {
    ret = -1;
    goto close_client;
  }

  ret = 0;
  char buf[1024];

  int len = recv(clientSock, buf, 1024, 0);
  if (len <= 0) {
    perror("recv");
    ret = -1;
    goto close_client;
  }
  buf[len] = '\0';
  printf("%s\n", buf);

close_client:
  close(clientSock);

  return ret;
}

int serverInit() {
  int ret = 0;
  int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
  int incomingSock;
  struct sockaddr_un address;
  socklen_t addrlen = sizeof(address);
  int bufCount = 256;
  char buffer[bufCount];
  FILE *file = fopen("reminders.txt", "r+");

  if (NULL == file) {
    file = fopen("reminders.txt", "w");
    if (NULL == file) {
      return -1;
    }
  }

  if (serverSock == -1) {
    ret = -1;
    goto close_server;
  }

  strncpy(address.sun_path, PATH, sizeof(address.sun_path) - 1);
  address.sun_family = AF_UNIX;

  unlink("/tmp/reme.sock");

  if (bind(serverSock, (struct sockaddr *)&address, sizeof(address)) == -1) {
    ret = -1;
    goto close_server;
  }

  if (listen(serverSock, 5) == -1) {
    ret = -1;
    goto close_server;
  }

  while (1) {
    if ((incomingSock =
             accept(serverSock, (struct sockaddr *)&address, &addrlen)) == -1) {
      return 1;
    }

    while (1) {
      char response[1024];

      ssize_t readingReturn = read(incomingSock, buffer, bufCount - 1);

      if (readingReturn <= 0) {
        break;
      }

      buffer[readingReturn] = '\0';
      strcpy(response, crudInterpret(file, buffer));

      if (-1 == checkNearestTrigger(file)) {
        strcpy(response, "Failure in checking!");
      }
      send(incomingSock, response, strlen(response), 0);
    }

    close(incomingSock);
  }
  ret = 0;

close_server:
  close(serverSock);
  fclose(file);

  return ret;
}

int sockSend(const char *string) {

  clientInit(string);

  return 0;
}
