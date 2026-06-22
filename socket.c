#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PATH "/tmp/reme.sock"

int writeAll(int fd, const void *data, size_t size) {
  const char *ptr = data;
  size_t done = 0;

  while (done < size) {
    ssize_t ret = write(fd, ptr + done, size - done);
    if (ret <= 0) {
      return -1;
    }
    done += ret;
  }

  return 0;
}

int readAll(int fd, void *data, size_t size) {
  char *ptr = data;
  size_t done = 0;

  while (done < size) {
    ssize_t ret = read(fd, ptr + done, size - done);
    if (ret <= 0) {
      return -1;
    }
    done += ret;
  }

  return 0;
}

int clientInit(const struct Request *request) {
  int ret = 0;
  int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr_un serverAddr = {0};
  struct Response response = {0};

  if (clientSock == -1) {
    return -1;
  }

  strncpy(serverAddr.sun_path, PATH, sizeof(serverAddr.sun_path) - 1);
  serverAddr.sun_family = AF_UNIX;

  if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
    printf("Failed to connect to reme!\n");
    ret = -1;
    goto close_client;
  }

  if (-1 == writeAll(clientSock, request, sizeof(*request))) {
    printf("Failed to send reminder!\n");
    ret = -1;
    goto close_client;
  }

  if (-1 == readAll(clientSock, &response, sizeof(response))) {
    perror("recv");
    ret = -1;
    goto close_client;
  }

  printf("%s\n", response.text);

close_client:
  close(clientSock);

  return ret;
}

int sockSend(const struct Request *request) {

  return clientInit(request);
}
