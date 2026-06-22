#ifndef CRUD
#define CRUD

#include <stdio.h>
#include <time.h>

#define LEN 256
#define RESLEN 1024

enum Operation { ADD, LIST, DELETE, UPDATE };

struct Reminder {
  int active;
  char message[LEN];
  struct tm timeInfo;
};

struct Request {
  enum Operation op;
  int index;
  char query[LEN];
  struct Reminder reminder;
};

struct Response {
  int status;
  char text[RESLEN];
};

int parseReminderLine(const char *line, struct Reminder *reminder);
int printReminderLine(FILE *file, const struct Reminder *reminder);
void crudInterpret(FILE *file, const struct Request *request,
                   struct Response *response);

#endif
