#include "crud.h"
#include "daemonize.h"
#include "sleeper.h"
#include "socket.h"
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <unistd.h>

#define PATH "/tmp/reme.sock"
#define LOCK_PATH "/tmp/reme.lock"

int singleInstanceLock() {
  int fd = open(LOCK_PATH, O_CREAT | O_RDWR, 0600);

  if (fd == -1) {
    perror(LOCK_PATH);
    return -1;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
    if (errno == EWOULDBLOCK) {
      fputs("Remed is already walking!\n", stderr);
    } else {
      perror(LOCK_PATH);
    }
    close(fd);
    return -1;
  }

  return fd;
}

int serverInit() {
  int ret = 0;
  int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
  int incomingSock;
  struct sockaddr_un address = {0};
  socklen_t addrlen = sizeof(address);
  FILE *file = fopen("reminders.txt", "r+");

  if (NULL == file) {
    file = fopen("reminders.txt", "w+");
    if (NULL == file) {
      perror("reminders.txt");
      return -1;
    }
  }

  if (serverSock == -1) {
    perror("socket");
    ret = -1;
    goto close_server;
  }

  strncpy(address.sun_path, PATH, sizeof(address.sun_path) - 1);
  address.sun_family = AF_UNIX;

  unlink("/tmp/reme.sock");

  if (bind(serverSock, (struct sockaddr *)&address, sizeof(address)) == -1) {
    perror("bind");
    ret = -1;
    goto close_server;
  }

  if (listen(serverSock, 5) == -1) {
    perror("listen");
    ret = -1;
    goto close_server;
  }

  if (-1 == schedulerStart(file)) {
    ret = -1;
    goto close_server;
  }

  while (1) {
    if ((incomingSock =
             accept(serverSock, (struct sockaddr *)&address, &addrlen)) == -1) {
      return 1;
    }

    while (1) {
      struct Request request;
      struct Response response = {0};

      if (-1 == readAll(incomingSock, &request, sizeof(request))) {
        break;
      }

      reminderLock();
      crudInterpret(file, &request, &response);
      reminderUnlock();
      schedulerChanged();

      writeAll(incomingSock, &response, sizeof(response));
    }

    close(incomingSock);
  }
  ret = 0;

close_server:
  close(serverSock);
  fclose(file);

  return ret;
}

int main(int argc, char *argv[]) {
  bool verbose = false;
  int check;

  opterr = 0;
  while ((check = getopt(argc, argv, "v")) != -1) {
    switch (check) {
    case 'v':
      verbose = true;
      break;
    default:
      fputs("USAGE: remed [-v]\n", stdout);
      return 1;
    }
  }

  if (optind != argc) {
    fputs("USAGE: remed [-v]\n", stdout);
    return 1;
  }

  if (singleInstanceLock() == -1) {
    return 1;
  }

  if (!verbose && -1 == daemonize()) {
    return 1;
  }

  return serverInit();
}
