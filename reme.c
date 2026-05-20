#include "crud.h"
#include "socket.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define LEN 256

// goal: simple CRUD. create time, read time, update time, delete time.

// important notes:
// struct tm expects a year that is { year - 1900}
// struct tm month expects a month that is { month - 1 }

bool isMeridiem = false; // A true meridiem would mean using the am pm format

struct reminder {
  char messageOfReminder[LEN];
  struct tm timeInfo;
};

struct tm *currentTime() {
  struct tm *current;
  time_t now = time(NULL);
  current = localtime(&now);

  return current;
}

int sendDelete(const char *toDelNum) {
  char buf[LEN];
  int chk = snprintf(buf, sizeof(buf), "%d|%s", DELETE, toDelNum);
  if (chk < 0 || chk > LEN) {
    perror("snprintf");
    return -1;
  }

  sockSend(buf);
  return 0;
}
int sendList(const char *reminderToDelete) {
  char buf[LEN];
  int chk = snprintf(buf, sizeof(buf), "%d|%s", LIST, reminderToDelete);
  if (chk < 0 || chk > LEN) {
    perror("snprintf");
    return -1;
  }

  sockSend(buf);
  return 0;
}

int sendEdit() { return 0; }

int sendAppend(char *reminder, const char *time, char *date) {
  // look here
  char buf[LEN];
  if (date == NULL) {
    date = "null";
  }
  int chk =
      snprintf(buf, sizeof(buf), "%d|%s|%s|%s", ADD, reminder, time, date);
  if (chk < 0 || chk > LEN) {
    perror("snprintf");
    return -1;
  }
  printf("%s\n", buf);
  sockSend(buf);
  return 0;
}

int main(int argc, char *argv[]) {

  int check;
  while ((check = getopt(argc, argv, "ldeath")) != -1) {
    switch (check) {
    case 'l':
      if (argc - optind < 1) {
        fputs("USAGE: reme -l \"REMINDER\"\n", stdout);
        fputs("-h for help\n", stdout);
        return 1;
      }
      sendList(argv[optind]);
      // format reme -d "remi"
      // shows list
      // respond with number
      return 0;
    case 'd':
      if (argc - optind < 1) {
        fputs("USAGE: reme -d \"REMINDER\"\n", stdout);
        fputs("-h for help\n", stdout);
        return 1;
      }
      sendDelete(argv[optind]);
      return 0;
    case 'e':
      sendEdit();
      return 0;
    case 't':
      isMeridiem = true;
      return 0;
    case 'h':
      fputs("\nreme [option] \"REMINDER\" TIME DATE\n\n", stdout);
      fputs("REMINDER\treminder message\n", stdout);
      fputs("TIME\t\tHH:MM format\n", stdout);
      fputs("DATE\t\tMM/DD/YYYY format\n", stdout);
      fputs("-t\t\tchange format to AM / PM\n", stdout);
      fputs("-d\t\tdelete a reminder\n", stdout);
      fputs("-e\t\tedit a reminder\n", stdout);
      return 0;
    }
  }

  if (argc - optind < 2) {
    fputs("USAGE: reme [OPTION]... \"REMINDER\" TIME DATE\n", stdout);
    fputs("-h for help\n", stdout);
    return 1;
  }

  if (argc - optind == 2) {
    sendAppend(argv[optind], argv[optind + 1], NULL);
  } else if (argc - optind == 3) {
    sendAppend(argv[optind], argv[optind + 1], argv[optind + 2]);
  }

  return 0;
}
