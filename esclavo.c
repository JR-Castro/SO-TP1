// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

//MUST to recieve the paths of the files to process y it MUST initiate the program corresponding for processing them (md5sum)
//MUST send the information relevant to processing to the process aplication
//MUSTN'T dump the result in a file in disc, to then read it from the slave, it MUST recieve the output of md5sum utilizing
//some mechanism of IPC more sophisticated

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#define BUFFERSIZE 1024
#define HASHSIZE 33
#define SCANFFORMAT "%32s %1023s"

void errorHandler(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    char command[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    char path[BUFFERSIZE];
    char hash[HASHSIZE];

    ssize_t rvalue = read(STDIN_FILENO, buffer, BUFFERSIZE);
    while (rvalue > 0){
        strcpy(command, "md5sum ");
        strncat(command, buffer, rvalue);

        FILE *file = popen(command, "r");
        if (file == NULL)
            errorHandler("popen");

        fscanf(file, SCANFFORMAT, hash, path);
        //  Basename returns the component following the final '/'
        //  In this case, that means it's the filename
        snprintf(buffer, BUFFERSIZE, "%d - %s - %s", getpid(), basename(path), hash);

        if (pclose(file) == -1)
            errorHandler("pclose");

        if (write(STDOUT_FILENO, buffer, strlen(buffer)) == -1)
            errorHandler("write");

        rvalue = read(STDIN_FILENO, buffer, BUFFERSIZE);
    }
    if (rvalue == -1)
        errorHandler("read");
    exit(EXIT_SUCCESS);
}