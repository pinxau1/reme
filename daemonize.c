#include "daemonize.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int daemonize() {

  int pid = fork();

  if (pid > 0) {
    // parent
    _exit(1);
  }

  if (pid == 0) {
    // child
  }

  if (pid == -1) {
    perror("daemon");
    return -1;
  }

  pid_t session = setsid();

  if (session == -1) {
    perror("session creation");
    return -1;
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);

  return 0;
}
