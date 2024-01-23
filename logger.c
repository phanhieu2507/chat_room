// logger.c
#include "logger.h"
#include <stdio.h>
#include <time.h>

void writeToLog(const char *message) {
    FILE *logFile;
    time_t currentTime;
    struct tm *localTime;

    logFile = fopen("activity_log.log", "a");

    if (logFile == NULL) {
        printf("Không thể mở tệp tin log.\n");
        return;
    }

    currentTime = time(NULL);
    localTime = localtime(&currentTime);

    fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
            localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
            message);

    fclose(logFile);
}
