#include "crud.h"
#include "daemonize.h"
#include "sleeper.h"
#include "socket.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
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

int reminder_path_init(char *path_name, size_t size) {
  const char *home = getenv("HOME");
  int ret;

  if (home == NULL) {
    fputs("HOME is not set\n", stderr);
    return -1;
  }

  ret = snprintf(path_name, size, "%s/.local/share/reme/reminders.txt", home);
  if (ret < 0 || (size_t)ret >= size) {
    fputs("reminder path is too long\n", stderr);
    return -1;
  }

  return 0;
}

int directory_store_init(char *path) {
  char directory[255];
  char *truncate_location;

  if (strlen(path) >= sizeof(directory)) {
    fputs("reminder directory path is too long\n", stderr);
    return -1;
  }

  strcpy(directory, path);
  truncate_location = strrchr(directory, '/');
  if (truncate_location == NULL) {
    fputs("invalid reminder path\n", stderr);
    return -1;
  }
  *truncate_location = '\0';

  if (mkdir(directory, 0700) == -1 && errno != EEXIST) {
    perror("mkdir");
    return -1;
  }

  return 0;
}

int serverInit() {
  int ret = 0;
  int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
  int incomingSock;
  struct sockaddr_un address = {0};
  socklen_t addrlen = sizeof(address);
  char fileName[255];
  if (-1 == reminder_path_init(fileName, sizeof(fileName))) {
    return -1;
  }
  if (-1 == directory_store_init(fileName)) {
    return -1;
  }
  FILE *file = fopen(fileName, "r+");

  if (NULL == file) {
    file = fopen(fileName, "w+");
    if (NULL == file) {
      perror("fopen reminders");
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
