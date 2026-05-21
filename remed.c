#include "daemonize.h"
#include "socket.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

int checkNearestTrigger(FILE *file) {
  time_t now = time(NULL);
  time_t rem;
  struct tm *reminder;
  char buf[256];
  int hh, mm, month, day, year;
  int record[5];
  int lowest[5] = {0};

  while (NULL != fgets(buf, sizeof(buf), file)) {
    if (5 != sscanf(buf, "%*[^|]|%*[^|]|%*[^|]|%d:%d|%d/%d/%d", &record[0],
                    &record[1], &record[2], &record[3], &record[4])) {
      return -1;
    }

    if (lowest[4] == 0) {
      memcpy(lowest, record, sizeof(record));
    } else {
      if (record[4] < lowest[4]) {
        memcpy(lowest, record, sizeof(record));
      } else if (record[3] < lowest[3]) {
        memcpy(lowest, record, sizeof(record));
      } else if (record[2] < lowest[2]) {
        memcpy(lowest, record, sizeof(record));
      } else if (record[1] < lowest[1]) {
        memcpy(lowest, record, sizeof(record));
      } else if (record[0] < lowest[0]) {
        memcpy(lowest, record, sizeof(record));
      }
    }
  }

  reminder->tm_hour = record[0];
  reminder->tm_min = record[1];
  reminder->tm_mon = record[2] - 1;
  reminder->tm_mday = record[3];
  reminder->tm_year = record[4];

  rem = mktime(reminder);
  double secs = difftime(now, rem);
  if (secs > 0) {
    sleep(secs);
  }
  return 0;
}

int main() {

  daemonize();
  serverInit();

  return 0;
}
