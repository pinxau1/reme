#include "crud.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static FILE *reminderFile;

void reminderLock() { pthread_mutex_lock(&mutex); }

void reminderUnlock() { pthread_mutex_unlock(&mutex); }

void schedulerChanged() {
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

static void notifySend(const char *message) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return;
  }

  if (pid == 0) {
    pid_t notifyPid = fork();

    if (notifyPid < 0) {
      perror("fork");
      _exit(1);
    }

    if (notifyPid == 0) {
      execlp("notify-send", "notify-send", "-a", "reme", "Reme", message,
             NULL);
      perror("execlp");
      _exit(1);
    }

    _exit(0);
  }

  while (waitpid(pid, NULL, 0) == -1) {
    if (errno != EINTR) {
      perror("waitpid");
      return;
    }
  }
}

static void notifyReminder(const struct Reminder *reminder) {
  printf("Reminder: %s\n", reminder->message);
  fflush(stdout);

  notifySend(reminder->message);
}

static int fireDueReminders() {
  char line[LEN + 64];
  long pos;
  int fired = 0;
  time_t now = time(NULL);

  rewind(reminderFile);
  while ((pos = ftell(reminderFile)) != -1 &&
         fgets(line, sizeof(line), reminderFile)) {
    struct Reminder reminder;
    time_t when;

    if (-1 == parseReminderLine(line, &reminder) || !reminder.active) {
      continue;
    }

    when = mktime(&reminder.timeInfo);
    if (when <= now) {
      notifyReminder(&reminder);
      fseek(reminderFile, pos, SEEK_SET);
      fputc('d', reminderFile);
      fseek(reminderFile, pos + strlen(line), SEEK_SET);
      fired = 1;
    }
  }

  if (fired) {
    fflush(reminderFile);
  }

  return fired;
}

static time_t nearestReminder() {
  char line[LEN + 64];
  time_t now = time(NULL);
  time_t nearest = 0;

  rewind(reminderFile);
  while (fgets(line, sizeof(line), reminderFile)) {
    struct Reminder reminder;
    time_t when;

    if (-1 == parseReminderLine(line, &reminder) || !reminder.active) {
      continue;
    }

    when = mktime(&reminder.timeInfo);
    if (when > now && (nearest == 0 || when < nearest)) {
      nearest = when;
    }
  }

  return nearest;
}

static int waitForNextReminder(time_t nearest) {
  time_t now = time(NULL);
  struct timespec currentTime;
  struct timespec waitUntil;
  int ret;

  if (nearest <= now) {
    return 0;
  }

  if (-1 == clock_gettime(CLOCK_REALTIME, &currentTime)) {
    perror("clock_gettime");
    waitUntil.tv_sec = nearest;
    waitUntil.tv_nsec = 0;
  } else {
    waitUntil.tv_sec = currentTime.tv_sec + (nearest - now);
    waitUntil.tv_nsec = currentTime.tv_nsec;
  }

  ret = pthread_cond_timedwait(&cond, &mutex, &waitUntil);
  if (ret != 0 && ret != ETIMEDOUT) {
    errno = ret;
    perror("pthread_cond_timedwait");
  }

  return ret;
}

static void *scheduler(void *arg) {
  (void)arg;

  pthread_mutex_lock(&mutex);
  while (1) {
    time_t nearest;

    fireDueReminders();
    nearest = nearestReminder();

    if (nearest == 0) {
      pthread_cond_wait(&cond, &mutex);
    } else {
      waitForNextReminder(nearest);
    }
  }
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int schedulerStart(FILE *file) {
  pthread_t thread;

  reminderFile = file;
  if (0 != pthread_create(&thread, NULL, scheduler, NULL)) {
    return -1;
  }
  pthread_detach(thread);

  return 0;
}
