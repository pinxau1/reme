#ifndef SLEEPER
#define SLEEPER

#include <stdio.h>

int schedulerStart(FILE *file);
void schedulerChanged();
void reminderLock();
void reminderUnlock();

#endif
