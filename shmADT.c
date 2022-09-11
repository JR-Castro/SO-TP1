// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shmADT.h"

char *shmem = NULL;
sem_t *semaphore = NULL;
unsigned int iterator = 0;
char shmemName[STRINGSIZE];

static int openSemaphore();

static void getSemaphoreName(char semname[STRINGSIZE]);

int createShm(const char name[STRINGSIZE]) {
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

int connectShm(const char name[STRINGSIZE]) {
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

    return openSemaphore();
}

int readerDelete() {
    if (munmap((void *) shmem, STRINGAMOUNT * STRINGSIZE)) {
        return SHMADT_ERROR;
    }
    if (shm_unlink(shmemName))
        return SHMADT_ERROR;
    char semname[STRINGSIZE];
    getSemaphoreName(semname);
    return sem_unlink(semname);
}

int writerDisconnect() {
    if (shmwrite(ENDSTRING) == -1)
        return SHMADT_ERROR;
    if (munmap((void *) shmem, STRINGAMOUNT * STRINGSIZE))
        return SHMADT_ERROR;
    return sem_close(semaphore);
}

int shmwrite(const char s[STRINGSIZE]) {
    if (iterator < STRINGAMOUNT) {
        strncpy(&(shmem[iterator * STRINGSIZE]), s, STRINGSIZE);
        iterator++;
        return (sem_post(semaphore)) ? -1 : STRINGAMOUNT - (int) iterator;
    }
    return 0;
}

char *shmread() {
    if (iterator < STRINGAMOUNT) {
        if (sem_wait(semaphore))
            return (char *) -1;
        char *ans = &(shmem[iterator * STRINGSIZE]);
        if (strcmp(ans, ENDSTRING) == 0) {
            iterator = STRINGAMOUNT;
            return NULL;
        }
        iterator++;
        return ans;
    }
    return NULL;
}

static int openSemaphore() {
    char semname[STRINGSIZE];
    getSemaphoreName(semname);
    //  Both reader and writer use the same function, since if the semaphore already exists
    //  then mode and value are ignored
    sem_t *sem = sem_open(semname, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (sem == SEM_FAILED)
        return SHMADT_ERROR;
    semaphore = sem;
    return 0;
}

static void getSemaphoreName(char semname[STRINGSIZE]) {
    strncpy(semname, shmemName, STRINGSIZE - 1);
    size_t size = strlen(semname);
    strncat(semname, "-sem", STRINGSIZE - 1 - size);
}

