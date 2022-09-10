// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shmADT.h"
#include <stdio.h>
#include <stdlib.h>

void errorHandler(const char *errorMsg);

void disconnectShmOnExit() {
    if (readerDisconnect())
        perror("disconnectShmOnExit");
}

int main(int argc, char const *argv[]) {

    char shmName[STRINGSIZE] = {0};

    if (argc == 1) {
        if (fgets(shmName, STRINGSIZE, stdin) == NULL)
            errorHandler("fgets");
    } else {
        strncpy(shmName, argv[1], STRINGSIZE);
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
