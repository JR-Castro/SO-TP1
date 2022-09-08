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
#define STRINGAMOUNT 1000

// Link with -lrt for shmem
// Link with -pthread for semaphore

//  Select a name for the shm, and amount is the amount of strings it should support.
int createShm(const char name[STRINGSIZE]);

int connectShm(const char name[STRINGSIZE]);

int shmwrite(const char * s);

int shmread(char * s);

int readerDisconnect();

int writerDelete();

#endif //SO_TP1_SHMADT_H
