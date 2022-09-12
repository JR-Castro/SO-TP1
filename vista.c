// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shmADT.h"
#include <stdio.h>
#include <stdlib.h>

void errorHandler(const char *errorMsg);

void disconnectShmOnExit() {
    if (readerClose())
        perror("disconnectShmOnExit");
}

int main(int argc, char const *argv[]) {

    if (setvbuf(stdout, NULL, _IONBF, 0))
        errorHandler("setvbuf");

    char shmName[STRINGSIZE] = {0};

    if (argc == 1) {
        ssize_t length = read(STDIN_FILENO, shmName, STRINGSIZE);
        if (length < 1)
            errorHandler("fgets");
        char *newline = strchr(shmName, '\n');
        if (newline != NULL)
            *newline = '\0';
        else
            shmName[length - 1] = '\0';
        // We check above that length is >= 1
    } else {
        strncpy(shmName, argv[1], STRINGSIZE);
        shmName[STRINGSIZE - 1] = '\0';
    }

    if (connectShm(shmName)) {
        errorHandler("connectShm");
    }
    if (atexit(disconnectShmOnExit))
        errorHandler("at exit");

    char *result;
    do {
        result = shmread();
        if (result == (char *) -1)
            errorHandler("shmread");
        if (result != NULL) {
            puts(result);
        }
    } while (result != NULL);

    exit(EXIT_SUCCESS);
}

void errorHandler(const char *errorMsg) {
    perror(errorMsg);
    exit(EXIT_FAILURE);
}
