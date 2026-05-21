#include "daemonize.h"
#include "socket.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {

  daemonize();
  serverInit();

  return 0;
}
