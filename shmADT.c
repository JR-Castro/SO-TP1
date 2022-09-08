// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shmADT.h"

char *shmem = NULL;
sem_t *semaphore = NULL;
unsigned int iterator = 0;
char shmemName[STRINGSIZE];

static int openSemaphore();

static int closeSemaphore();

int createShm(const char name[STRINGSIZE]){
    if (shmem != NULL)
        return SHMADT_ERROR;
    int fd = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if (fd == -1)
        return SHMADT_ERROR;

    if (ftruncate(fd, STRINGAMOUNT * STRINGSIZE))
        return SHMADT_ERROR;

    void *mapped = mmap(NULL, STRINGAMOUNT * STRINGSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED)
        return SHMADT_ERROR;

    shmem = (char *) mapped;
    strncpy(shmemName, name, STRINGSIZE);

    return openSemaphore();
}

int connectShm(const char name[STRINGSIZE]){
    if (shmem != NULL)
        return SHMADT_ERROR;
    int fd = shm_open(name, O_RDONLY, 0);
    if (fd == -1)
        return SHMADT_ERROR;

    void *mapped = mmap(NULL, STRINGAMOUNT * STRINGSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (close(fd))  //  fd is no longer needed after mmap
        return SHMADT_ERROR;

    if (mapped == MAP_FAILED)
        return SHMADT_ERROR;

    shmem = (char *) mapped;
    strncpy(shmemName, name, STRINGSIZE);

    return 0;
}

int readerDisconnect(){
    return munmap((void*)shmem, STRINGAMOUNT * STRINGSIZE) ? SHMADT_ERROR : 0;
}

int writerDelete(){
    if (munmap((void*)shmem, STRINGAMOUNT * STRINGSIZE))
        return SHMADT_ERROR;
    return shm_unlink(shmemName) ? SHMADT_ERROR : 0;
}

int openSemaphore(){
    char semname[STRINGSIZE];
    strncpy(semname, shmemName, STRINGSIZE);
    strncat(semname, "-sem", STRINGSIZE);
    //  Both reader and writer use the same function, since if the semaphore already exists
    //  then mode and value are ignored
    sem_t * sem = sem_open(semname, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (sem == SEM_FAILED)
        return SHMADT_ERROR;
    semaphore = sem;
    return 0;
}