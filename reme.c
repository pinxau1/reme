#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<ctype.h>
#include<sys/wait.h>
#include<stdbool.h>
#include<stdlib.h>
//simple CRUD. create time, read time, update time, delete time.
//important learnings
//struct tm expects a year that is { year - 1900}
//struct tm month expects a month that is { month - 1 }

bool isMeridiem = false; //A true meridiem would mean using the am pm format

struct reminder{
  char messageOfReminder[256];
  struct tm timeInfo;
};



void daemonize(){
  pid_t pid = fork();

  if(pid == -1){
    perror("fork failed!\n");
    return;
  }

  if(pid > 0){
    exit(EXIT_SUCCESS);
  }

  if(setsid() == -1){
    perror("setsid failed!\n");
    return;
  }

  fclose(stdin);
  fclose(stdout);
  fclose(stderr);

  return;
}
int triggerReminder (const char *reminderMessage) {
  pid_t pid = fork();

  if( pid < 0){
    perror("Fork Failed!\n");
    return 0;
  }

  if(pid == 0){
    execlp("notify-send", "notify-send", "-a", "reme-display", "-u", "critical",
            reminderMessage, NULL);
    perror("execlp failed!\n");
    _exit(1);
  }

  if(pid > 0){
    exit(EXIT_SUCCESS);
  }

  if(waitpid(pid, NULL, 0)== -1){
    perror("waitpid failed!\n");
    return 0;
  }

  return 1;
}

int setTime(char* reminderMessage, char* timeString, char* dateString){
  struct tm *current = NULL;
  struct tm userInput = {0};

  time_t now = time(NULL);
  current = localtime(&now);

  if(!dateString){
    userInput.tm_mon = current->tm_mon;
    userInput.tm_mday = current->tm_mday;
    userInput.tm_year = current->tm_year;
  } else {
    if(sscanf(dateString, "%d/%d/%d", &userInput.tm_mon, &userInput.tm_mday, &userInput.tm_year) != 3){
      fputs("Lacking Date Arguments!\n", stdout);
      return 0;
    }
    userInput.tm_mon -= 1;
    userInput.tm_year -= 1900;
  }


  if (isMeridiem){

    char ampm_checker[2];
    if(sscanf(timeString, "%d:%d%2s", &userInput.tm_hour, &userInput.tm_min, ampm_checker) != 3){
      fputs("Lacking Arguments!\n", stdout);
      return 0;
    } else if ((userInput.tm_hour > 12 || userInput.tm_hour < 1) || (userInput.tm_min > 59 || userInput.tm_min < 0)){
      fputs("Invalid Time Arguments!", stdout);
      return 0;
    }

    for(int i = 0; i < 2; i++){
      ampm_checker[i] = tolower(ampm_checker[i]);
    }

    if(strcmp(ampm_checker, "pm") == 0){
      userInput.tm_hour += 12;
    } else if (strcmp(ampm_checker, "am") == 0){
      if(userInput.tm_hour == 12) 
        userInput.tm_hour = 0;
    } else {
      fputs("Use valid time format!\n", stdout);
    }
  } else {

    if(sscanf(timeString, "%d:%d", &userInput.tm_hour, &userInput.tm_min) != 2){
      fputs("Lacking Arguments!\n", stdout);
      return 0;
    } else if ((userInput.tm_hour > 24 || userInput.tm_hour < 0) || (userInput.tm_min > 59 || userInput.tm_min < 0)){
      fputs("Invalid Time Arguments!\n", stdout);
      return 0;
    }
  }


  time_t triggerTill = mktime(&userInput);
  time_t trigger = triggerTill - now;

  if(triggerTill < now ){
    fputs("That time is in the past!\n", stdout);
    return 0;
  }
  printf("\nThe current time: %d:%d %d\nReminder explodes at %d:%d %d\n", current->tm_hour, current->tm_min, current->tm_year+1900, userInput.tm_hour, userInput.tm_min, userInput.tm_year+1900);


  daemonize();
  sleep(trigger);
  triggerReminder(reminderMessage);

  return 1;

}


int main(int argc, char *argv[]){


  int check;

  while((check = getopt(argc, argv, "th")) != -1){
    switch(check){
      case 't':
        isMeridiem = true; 
        break;
      case 'h':
        fputs("\nreme [option] \"REMINDER\" TIME DATE\n\n", stdout);
        fputs("REMINDER\treminder message\n", stdout);
        fputs("TIME\t\tHH:MM format\n", stdout);
        fputs("DATE\t\tMM/DD/YYYY format\n", stdout);
        fputs("-t\t\tchange format to AM / PM\n", stdout);
        return 0;
    }
  }

  if(argc - optind < 2) {
    fputs("USAGE: reme [OPTION]... \"REMINDER\" TIME DATE\n", stdout);
    fputs("-h for help\n", stdout);
    return 1;
  }

  if(argc - optind == 2){

    setTime(argv[optind], argv[optind + 1], NULL);
  } else if(argc - optind == 3){
    setTime(argv[optind], argv[optind + 1], argv[optind + 2]);
  }



  return 0;
}


