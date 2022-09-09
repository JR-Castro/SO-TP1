// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <errno.h>
#include <string.h>

#include "shmADT.h"

#define SLAVELIMIT 5
#define SLAVEPATH "./slave"
#define RESULTPATH "./resultado"

int SLAVE_IN[SLAVELIMIT];   //  fd for slave inputs -> where the paths are sent to be processed by the slaves

int SLAVE_OUT[SLAVELIMIT];  //  fd for slave outputs -> where the slaves response is sent.

void getFilePaths(int amount, char * paths[], glob_t * globbuf);

void createSlavePipes(int fd[2]);

void executeSlave(int master_to_slave[2], int slave_to_master[2]);

void createSlaves();

void startShm();

void errorHandler(const char * msg);

/*
 *  Returns 0 if write was completed correctly.
 *  Retruns 1 if path was not a file.
 */
int sendFileToSlave(const char *path, unsigned int slavenum){
    struct stat statbuffer;
    errno = 0;
    if (stat(path, &statbuffer) == -1)
        errorHandler("stat");

    if (S_ISREG(statbuffer.st_mode)) {
        //  Only check for errors, since when writing to a pipe, write blocks until there's enough space.
        errno = 0;
        if (write(SLAVE_IN[slavenum], path, strlen(path)) == -1)
            errorHandler("write to pipe");
        return 0;   // Write completed correctly.
    }
    return 1;   // Path was not a file.
}

/*  Returns the biggest file descriptor numbre in the array */
int getMaxFd(int fd[SLAVELIMIT]){
    int max = fd[0];
    for (int i = 1; i < SLAVELIMIT; ++i) {
        if (fd[i] > max)
            max = fd[i];
    }
    return max;
}

void setFd(fd_set *fdSet, int fds[SLAVELIMIT]){
    for (int i = 0; i < SLAVELIMIT; ++i) {
        FD_SET(fds[i], fdSet);
    }
}

void manageSlaves(glob_t * globbuf, int resultFd){
    unsigned int slaveIterator = 0;
    int pathIterator, filesSent = 0, filesReceived = 0;
    // Send initial files for slaves
    for (pathIterator = 0; pathIterator < globbuf->gl_pathc && pathIterator < SLAVELIMIT * 2; ++pathIterator) {
        int ret = 1;
        while (ret && pathIterator < globbuf->gl_pathc){
            ret = sendFileToSlave(globbuf->gl_pathv[pathIterator], slaveIterator);
            pathIterator++;
        }
        slaveIterator = (slaveIterator + 1) % SLAVELIMIT;
    }

    fd_set fdSet;
    int nfds = getMaxFd(SLAVE_OUT) + 1;
    while (globbuf->gl_pathc > pathIterator || (globbuf->gl_pathc == pathIterator && filesReceived < filesSent)){
        FD_ZERO(&fdSet);
        setFd(&fdSet, SLAVE_OUT);

        int numToRead = select(nfds, &fdSet, NULL, NULL, NULL);
        if (numToRead == -1)
            errorHandler("select");

        for (int i = 0; i < SLAVELIMIT; ++i) {
            char result[STRINGSIZE];
            long sizeRead;
            int ret = 1;

            if (FD_ISSET(SLAVE_OUT[i], &fdSet)){
                // Read the result.
                sizeRead = read(SLAVE_OUT[i], result, STRINGSIZE-1);
                if (sizeRead == -1)
                    errorHandler("read from slave pipe");
                filesReceived++;

                // End the string.
                if (sizeRead >= 0)
                    result[sizeRead] = '\0';

                if (write(resultFd, result, strlen(result)))
                    errorHandler("write to results file");

                shmwrite(result);

                while (ret && pathIterator < globbuf->gl_pathc){
                    ret = sendFileToSlave(globbuf->gl_pathv[pathIterator], i);
                    pathIterator++;
                    if (ret)
                        filesSent++;
                }
            }
        }
    }
}

int main(int argc, char * argv[]){

    if (argc < 2){         //  Check if path is an argument.
        fprintf(stderr, "Usage: %s [FILE]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    glob_t globbuf;
    getFilePaths(argc-1, &(argv[1]), &globbuf);

    //  Create the results file
    errno = 0;
    int result_fd = open(RESULTPATH,O_CREAT|O_RDONLY|O_TRUNC, S_IWUSR|S_IRUSR);
    if (result_fd == -1)
        errorHandler("Open results");

    startShm();

    sleep(2);

    //  Create slaves and set fd.
    createSlaves();

    manageSlaves(&globbuf, result_fd);

    //  Once all files are distributed between slaves, close the pipes to signal EOF.
    errno = 0;
    for (int i = 0; i < SLAVELIMIT; ++i) {
        if (close(SLAVE_IN[i]))
            errorHandler("Close pipes");
    }

    if (writerDelete())
        errorHandler(NULL);

    globfree(&globbuf);
    exit(EXIT_SUCCESS);
}

void getFilePaths(int amount, char * paths[], glob_t * globbuf){
    for (int i = 1; i < amount; ++i) {
        errno = 0;
        int ans = glob(paths[i], GLOB_NOSORT | ( i > 1 ? GLOB_APPEND : 0), NULL, globbuf);
        if (ans && ans != GLOB_NOMATCH){
            errorHandler("glob");
        }
    }
    if (globbuf->gl_pathc == 0)
        exit(EXIT_SUCCESS);
}

void createSlavePipes(int fd[2]){
    errno = 0;
    if (pipe(fd))
        errorHandler("pipe");
}

void executeSlave(int master_to_slave[2], int slave_to_master[2]){
    close(master_to_slave[1]);
    close(slave_to_master[0]);

    dup2(master_to_slave[0], 0);    //  Change stdin for pipe from master.
    dup2(slave_to_master[1], 1);    //  Change stdout for pipe to master.
    // dup2 also closes the original 0 and 1 file descriptors.

    errno = 0;
    if (execl(SLAVEPATH, SLAVEPATH, NULL)){ //  Execute the slave program.
        errorHandler("execl");
    }
}

void createSlaves(){
    int master_to_slave[2];
    int slave_to_master[2];

    for (int i = 0; i < SLAVELIMIT; ++i) {
        createSlavePipes(master_to_slave);

        createSlavePipes(slave_to_master);

        SLAVE_IN[i] = master_to_slave[1];   //  Write end of the pipe is saved by the master.
        SLAVE_OUT[i] = slave_to_master[0];  //  Read end of the pipe is saved by the master.
        errno = 0;
        int pid = fork();
        if ( pid == 0 ) //  Child process modifies it's file descriptors and executes ./slave.
            executeSlave(master_to_slave, slave_to_master);
        if ( pid == -1)
            errorHandler("fork");

        //  Close fd meant for the slave.
        close(master_to_slave[0]);
        close(slave_to_master[1]);
    }

}

void startShm(){
    char shmName[STRINGSIZE];
    snprintf(shmName, STRINGSIZE, "/aplicacion-%d", getpid());
    createShm(shmName);
    puts(shmName);
}

void errorHandler(const char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}
