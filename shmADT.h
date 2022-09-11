//
// Created by monarch on 08/09/22.
//

#ifndef SO_TP1_SHMADT_H
#define SO_TP1_SHMADT_H

#ifndef _BSD_SOURCE // ftruncate
#define _BSD_SOURCE
#endif

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <string.h>

#define SHMADT_ERROR (-1)
#define STRINGSIZE 256
#define STRINGAMOUNT 2048
#define ENDSTRING "ZZZZZZ"

// Link with -lrt for shmem
// Link with -pthread for semaphore

/*
 * The writer process creates the shared memory
 * Returns:
 *      0 on success
 *      -1 on failure, with errno set
 */
int createShm(const char name[STRINGSIZE]);

/*
 * The reader process connects to the shared memory
 * Returns 0 on success, -1 on error with errno set
 */
int connectShm(const char name[STRINGSIZE]);

/*
 * Writes to the shared memory
 * Returns the amount of space left for strings of STRINGSIZE
 *  or -1 if an error happened with the semaphore, and errno is set
*/
int shmwrite(const char s[STRINGSIZE]);

/*
 * Reads from the shared memory
 * Returns a pointer to the string in the shared memory (DO NOT MODIFY), NULL if there was nothing to read
 * or (char*)-1 if there was an error.
*/
char *shmread();

/*
 * Reader deletes the shared memory
 * Returns 0 on success, -1 on error and errno is set
 */
int readerClose();

/*
 * The writer disconnects from the shared memory
 * Returns 0 on success, -1 on error and errno is set
 */
int writerClose();

#endif //SO_TP1_SHMADT_H
