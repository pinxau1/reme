#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int changed = 0;
time_t lowest = 0;

void* sleeper(void *arg){
  struct timespec explode = {lowest, 0};
  pthread_mutex_lock(&mutex);
  if(changed == 1) {
    pthread_cond_timedwait(&cond, &mutex, &explode);
    changed = 0;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* checkNearestTrigger(void *file) {
  time_t now = time(NULL);
  time_t rem;
  struct tm reminder;
  char buf[256];

  while (NULL != fgets(buf, sizeof(buf), file)) {
    if (5 != sscanf(buf, "%*[^|]|%*[^|]|%*[^|]|%d:%d|%d/%d/%d",
                    &reminder.tm_hour, &reminder.tm_min, &reminder.tm_mon, &reminder.tm_mday, &reminder.tm_year)) {
      return NULL;
    }
  }
  reminder.tm_mon--;
  rem = mktime(&reminder);
  int secs = rem - now;
  if (secs > 0) {
    if(lowest == 0 || lowest > secs){
      lowest = secs;
      pthread_mutex_lock(&mutex);
      changed = 1;
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
    }
  }
  return NULL;
}


int threadCreate(FILE *file){
  pthread_t t1;
  pthread_t t2;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  pthread_create(&t1, NULL, checkNearestTrigger, file);
  pthread_create(&t2, NULL, sleeper, NULL);

  return 0;
}


