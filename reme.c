#include "crud.h"
#include "socket.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

bool isMeridiem = false;

struct tm currentTime() {
  time_t now = time(NULL);
  struct tm *current = localtime(&now);

  return *current;
}

int parseTime(const char *time, struct tm *timeInfo) {
  int hour, min;
  int used = 0;
  char meridiem[3] = {0};

  if (isMeridiem) {
    if (3 != sscanf(time, "%d:%d%2s%n", &hour, &min, meridiem, &used) ||
        time[used] != '\0') {
      printf("Wrong format! Refer to reme -h\n");
      return -1;
    }
    if (0 != strcasecmp(meridiem, "am") && 0 != strcasecmp(meridiem, "pm")) {
      printf("Wrong format! Refer to reme -h\n");
      return -1;
    }
    if (hour < 1 || hour > 12 || min < 0 || min > 59) {
      printf("Wrong format! Refer to reme -h\n");
      return -1;
    }
    if (0 == strcasecmp(meridiem, "am") && hour == 12) {
      hour = 0;
    } else if (0 == strcasecmp(meridiem, "pm") && hour != 12) {
      hour += 12;
    }
  } else if (2 != sscanf(time, "%d:%d%n", &hour, &min, &used) ||
             time[used] != '\0') {
    printf("Wrong format! Refer to reme -h\n");
    return -1;
  } else if (hour < 0 || hour > 23 || min < 0 || min > 59) {
    printf("Wrong format! Refer to reme -h\n");
    return -1;
  }

  timeInfo->tm_hour = hour;
  timeInfo->tm_min = min;
  timeInfo->tm_sec = 0;
  timeInfo->tm_isdst = -1;

  return 0;
}

int parseDate(const char *date, struct tm *timeInfo) {
  int mon, day, year;
  int used = 0;

  if (3 != sscanf(date, "%d/%d/%d%n", &mon, &day, &year, &used) ||
      date[used] != '\0') {
    printf("Wrong format! Refer to reme -h\n");
    return -1;
  }
  if (mon < 1 || mon > 12 || day < 1 || day > 31 || year < 1900) {
    printf("Wrong format! Refer to reme -h\n");
    return -1;
  }

  timeInfo->tm_mon = mon - 1;
  timeInfo->tm_mday = day;
  timeInfo->tm_year = year - 1900;

  return 0;
}

int validateReminderTime(struct tm *timeInfo) {
  struct tm normalized = *timeInfo;
  time_t reminderTime = mktime(&normalized);
  time_t now = time(NULL);

  if (reminderTime == -1 || normalized.tm_year != timeInfo->tm_year ||
      normalized.tm_mon != timeInfo->tm_mon ||
      normalized.tm_mday != timeInfo->tm_mday ||
      normalized.tm_hour != timeInfo->tm_hour ||
      normalized.tm_min != timeInfo->tm_min) {
    printf("Wrong format! Refer to reme -h\n");
    return -1;
  }
  if (reminderTime < now) {
    printf("Time is in the past!\n");
    return -1;
  }

  *timeInfo = normalized;
  return 0;
}

int sendDelete(const char *toDelNum) {
  struct Request request = {0};

  request.op = DELETE;
  request.index = atoi(toDelNum);

  return sockSend(&request);
}

int sendList(const char *reminderToDelete) {
  struct Request request = {0};

  request.op = LIST;
  snprintf(request.query, sizeof(request.query), "%s", reminderToDelete);

  return sockSend(&request);
}

int sendEdit() { return 0; }

int exitCode(int ret) {
  if (ret < 0) {
    return 1;
  }

  return ret;
}

int sendAppend(char *message, const char *time, char *date) {
  struct Request request = {0};

  request.op = ADD;
  request.reminder.active = 1;
  request.reminder.timeInfo = currentTime();
  snprintf(request.reminder.message, sizeof(request.reminder.message), "%s",
           message);

  if (-1 == parseTime(time, &request.reminder.timeInfo)) {
    return -1;
  }
  if (date != NULL && -1 == parseDate(date, &request.reminder.timeInfo)) {
    return -1;
  }
  if (-1 == validateReminderTime(&request.reminder.timeInfo)) {
    return -1;
  }

  return sockSend(&request);
}

int main(int argc, char *argv[]) {

  int check;
  while ((check = getopt(argc, argv, "ldeath")) != -1) {
    switch (check) {
    case 'l':
      return sendList(argc - optind < 1 ? "" : argv[optind]);
    case 'd':
      if (argc - optind < 1) {
        fputs("USAGE: reme -d \"REMINDER\"\n", stdout);
        fputs("-h for help\n", stdout);
        return 1;
      }
      return sendDelete(argv[optind]);
    case 'e':
      sendEdit();
      return 0;
    case 't':
      isMeridiem = true;
      break;
    case 'h':
      fputs("\nreme [option] \"REMINDER\" TIME DATE\n\n", stdout);
      fputs("REMINDER\treminder message\n", stdout);
      fputs("TIME\t\tHH:MM format\n", stdout);
      fputs("-t TIME\t\tHH:MMAM or HH:MMPM format\n", stdout);
      fputs("DATE\t\tMM/DD/YYYY format\n", stdout);
      fputs("-l [REMINDER]\tlist reminders, optionally filtered by message\n", stdout);
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
    return exitCode(sendAppend(argv[optind], argv[optind + 1], NULL));
  } else if (argc - optind == 3) {
    return exitCode(sendAppend(argv[optind], argv[optind + 1], argv[optind + 2]));
  }

  return 0;
}
