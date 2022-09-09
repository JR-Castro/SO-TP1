//
// Created by monarch on 08/09/22.
//

#ifndef SO_TP1_SHMADT_H
#define SO_TP1_SHMADT_H

#define _BSD_SOURCE // ftruncate

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <string.h>

#define SHMADT_ERROR -1
#define STRINGSIZE 256
#define STRINGAMOUNT 2048

// Link with -lrt for shmem
// Link with -pthread for semaphore

int createShm(const char name[STRINGSIZE]);

int connectShm(const char name[STRINGSIZE]);

// Writes to the shared memory
// Returns the amount of space left for strings of STRINGSIZE or -1 if an error happened with the semaphore.
int shmwrite(const char s[STRINGSIZE]);

// Reads from the shared memory
// Returns the amount of space left for strings of STRINGSIZE or -1 if something happened with se semaphore.
int shmread(char s[STRINGSIZE]);

int readerDisconnect();

int writerDelete();

#endif //SO_TP1_SHMADT_H
