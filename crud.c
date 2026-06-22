#include "crud.h"
#include <stdio.h>
#include <string.h>

int parseReminderLine(const char *line, struct Reminder *reminder) {
  char state;
  int hour, min, mon, day, year;

  if (7 != sscanf(line, "%c|%255[^|]|%d:%d|%d/%d/%d", &state,
                  reminder->message, &hour, &min, &mon, &day, &year)) {
    return -1;
  }

  reminder->active = state == 'a';
  reminder->timeInfo = (struct tm){0};
  reminder->timeInfo.tm_hour = hour;
  reminder->timeInfo.tm_min = min;
  reminder->timeInfo.tm_mon = mon - 1;
  reminder->timeInfo.tm_mday = day;
  reminder->timeInfo.tm_year = year - 1900;
  reminder->timeInfo.tm_isdst = -1;

  return 0;
}

int printReminderLine(FILE *file, const struct Reminder *reminder) {
  char state = reminder->active ? 'a' : 'd';

  return fprintf(file, "%c|%s|%02d:%02d|%d/%d/%d\n", state,
                 reminder->message, reminder->timeInfo.tm_hour,
                 reminder->timeInfo.tm_min, reminder->timeInfo.tm_mon + 1,
                 reminder->timeInfo.tm_mday, reminder->timeInfo.tm_year + 1900);
}

static void responseSet(struct Response *response, int status,
                        const char *text) {
  response->status = status;
  snprintf(response->text, sizeof(response->text), "%s", text);
}

static void softDelete(FILE *file, int targetIdx, struct Response *response) {
  int idx = 1;
  char line[LEN + 64];
  long pos;

  if (targetIdx <= 0) {
    responseSet(response, 1, "Invalid Index");
    return;
  }

  rewind(file);
  while ((pos = ftell(file)) != -1 && fgets(line, sizeof(line), file)) {
    struct Reminder reminder;

    if (-1 == parseReminderLine(line, &reminder)) {
      continue;
    }
    if (!reminder.active) {
      continue;
    }
    if (idx == targetIdx) {
      fseek(file, pos, SEEK_SET);
      fputc('d', file);
      fflush(file);
      responseSet(response, 0, "Deleted Successfully!");
      return;
    }
    idx++;
  }

  responseSet(response, 1, "Invalid Index");
}

static void listProtocol(FILE *file, const char *query,
                         struct Response *response) {
  char line[LEN + 64];
  char temp[LEN + 128];
  int idx = 1;

  response->text[0] = '\0';
  rewind(file);
  while (fgets(line, sizeof(line), file)) {
    struct Reminder reminder;

    if (-1 == parseReminderLine(line, &reminder)) {
      continue;
    }
    if (!reminder.active || NULL == strstr(reminder.message, query)) {
      continue;
    }

    snprintf(temp, sizeof(temp), "%d -> %s|%02d:%02d|%d/%d/%d\n", idx++,
             reminder.message, reminder.timeInfo.tm_hour,
             reminder.timeInfo.tm_min, reminder.timeInfo.tm_mon + 1,
             reminder.timeInfo.tm_mday, reminder.timeInfo.tm_year + 1900);
    strncat(response->text, temp,
            sizeof(response->text) - strlen(response->text) - 1);
  }

  if (response->text[0] == '\0') {
    responseSet(response, 1, "Pattern not Found");
    return;
  }

  response->status = 0;
}

void crudInterpret(FILE *file, const struct Request *request,
                   struct Response *response) {
  switch (request->op) {
  case ADD:
    if (-1 == printReminderLine(file, &request->reminder)) {
      responseSet(response, 1, "Failed to print but interpreted");
      return;
    }
    fflush(file);
    responseSet(response, 0, "Reminder added\n");
    return;
  case LIST:
    listProtocol(file, request->query, response);
    return;
  case UPDATE:
    responseSet(response, 1, "Reminder updated\n");
    return;
  case DELETE:
    softDelete(file, request->index, response);
    return;
  default:
    responseSet(response, 1, "Failed to Interpret\n");
    return;
  }
}
