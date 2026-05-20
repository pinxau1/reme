#include "crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *softDelete(FILE *file, char *targetIdx) {
  int goalIdx = atoi(targetIdx), ctr = 1;
  char buf[256];

  if (goalIdx <= 0) {
    return "Invalid Index";
  }
  rewind(file);
  while (ctr < goalIdx) {

    if (NULL == fgets(buf, sizeof(buf), file)) {
      return "Invalid Index";
    }

    if (buf[0] == 'a')
      ctr++;
  }

  fputc('d', file);
  return "Deleted Successfully!";
}

char *listProtocol(FILE *file, const char *string) {
  rewind(file);
  char holder[256], *response = NULL, temp[300];
  int i = 1;

  while (fgets(holder, sizeof(holder), file) != NULL) {
    if (strstr(holder, string)) {
      if (holder[0] == 'd') {
        continue;
      }
      char toShow[256];
      if (NULL == response) {
        response = malloc(1024);
      }
      sscanf(holder, "%*[^|]|%*[^|]|%255[^\n]", toShow);
      snprintf(temp, sizeof(temp), "%d -> %s\n", i++, toShow);
      strcat(response, temp);
    }
  }

  if (NULL == response)
    return "Pattern not Found";
  return response;
}

const char *crudInterpret(FILE *file, const char *buffer) {

  int op;
  sscanf(buffer, "%d", &op);
  printf("%d\n", op);
  switch (op) {
  case ADD: {
    char reminder[256], time[10], date[10];
    if (4 !=
        sscanf(buffer, "%d|%255[^|]|%[^|]|%s", &op, reminder, time, date)) {
      return "Lacking arguments!";
    }
    if (-1 == fprintf(file, "a|%s\n", buffer)) {
      return "Failed to print but interpreted";
    }
    fflush(file);
    return "Reminder added\n";
  }
  case LIST: {
    char target[256];
    if (1 != sscanf(buffer, "%*[^|]|%s", target)) {
      return "Lacking Arguments";
    }
    return listProtocol(file, target);
  }
  case UPDATE:
    return "Reminder updated\n";
  case DELETE: {
    char target[256];
    if (1 != sscanf(buffer, "%*[^|]|%s", target)) {
      return "Lacking Arguments";
    };
    return softDelete(file, target);
  }
  default:
    return "Failed to Interpret\n";
  }
}
