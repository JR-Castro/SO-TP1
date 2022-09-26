// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define STRINGSIZE 1024

void errorHandler(const char *errorMsg);

int main(int argc, char const *argv[]) {

    if (setvbuf(stdout, NULL, _IONBF, 0))
        errorHandler("setvbuf");

    char fifoName[STRINGSIZE] = {0};

    if (argc == 1) {
        ssize_t length = read(STDIN_FILENO, fifoName, STRINGSIZE);
        if (length < 1)
            errorHandler("fgets");
        char *newline = strchr(fifoName, '\n');
        if (newline != NULL)
            *newline = '\0';
        else
            fifoName[length - 1] = '\0';
        // We check above that length is >= 1
    } else {
        strncpy(fifoName, argv[1], STRINGSIZE);
        fifoName[STRINGSIZE - 1] = '\0';
    }

    int fifofd = open(fifoName, O_RDONLY);
    if (fifofd == -1)
        errorHandler("open fifo");

    char buffer[STRINGSIZE];

    ssize_t result;
    do {
        result = read(fifofd, buffer, STRINGSIZE);
        if (result == 0)
            break;
        write(STDOUT_FILENO, buffer, result);
    } while (1);

    exit(EXIT_SUCCESS);
}

void errorHandler(const char *errorMsg) {
    perror(errorMsg);
    exit(EXIT_FAILURE);
}
