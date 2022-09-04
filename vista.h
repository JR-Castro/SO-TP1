#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>           
#include <semaphore.h>

#define STDIN 0
#define STDOUT 1
#define MAX_MEM_CHARS 21
#define ERROR -1
#define MAX_BUFF 4096
#define ERROR_MSG 100

#endif

