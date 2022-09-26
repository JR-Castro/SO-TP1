// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGSIZE 1024
#define SLAVELIMIT 5
//  Path to the slaves binary
#define SLAVEPATH "./esclavo"
//  Path to the results file
#define RESULTPATH "./resultado"
//  Amount of files sent initially to each slave
//  If we send more than 1 file at the start, we should change the size of the buffer used on the read in readAndSendFileToSlave()
#define INITIALFILESCOUNT 1

int SLAVE_IN[SLAVELIMIT];   //  fd for slave inputs -> where the paths are sent to be processed by the slaves
int SLAVE_OUT[SLAVELIMIT];  //  fd for slave outputs -> where the slaves response is sent.
char **paths;
int pathc;
int fifofd;

void getFilePaths(int amount, char *paths[]);

void createSlavePipes(int fd[2]);

void executeSlave(int master_to_slave[2], int slave_to_master[2]);

void createSlaves();

void errorHandler(const char *msg);

/*
 *  Returns 0 if write was completed correctly.
 *  Retruns 1 if path was not a file.
 */
int sendFileToSlave(const char *path, unsigned int slavenum) {
    struct stat statbuffer;
    if (stat(path, &statbuffer) == -1)
        errorHandler("stat");

    if (S_ISREG(statbuffer.st_mode)) {
        //  Only check for errors, since when writing to a pipe, write blocks until there's enough space.
        if (write(SLAVE_IN[slavenum], path, strlen(path)) == -1)
            errorHandler("write to pipe");
        return 0;   // Write completed correctly.
    }
    return 1;   // Path was not a file.
}

/*  Returns the biggest file descriptor numbre in the array */
int getMaxFd(int fds[SLAVELIMIT]) {
    int max = fds[0];
    for (int i = 1; i < SLAVELIMIT; ++i) {
        if (fds[i] > max)
            max = fds[i];
    }
    return max;
}

void setFd(fd_set *fdSet, int fds[SLAVELIMIT]) {
    for (int i = 0; i < SLAVELIMIT; ++i) {
        FD_SET(fds[i], fdSet);
    }
}

//  Reads a slave's output and prints it to the results file and sends to the shared memory
void readAndSendFileToSlave(int *pathIterator, int *filesReceived, int *filesSent, int slaveNum, int resultFd) {
    char result[STRINGSIZE];
    long sizeRead;
    int ret = 1;
    /*  Read the result from the slave.
     *  If we send more than one file in the initial distribution, then we should use a bigger buffer to
     *  make sure all their results can be stored at once.
     */
    sizeRead = read(SLAVE_OUT[slaveNum], result, STRINGSIZE - 1);
    if (sizeRead == -1)
        errorHandler("read from slave pipe");
    (*filesReceived)++;

    // End the string.
    if (sizeRead >= 0 && sizeRead <= STRINGSIZE - 2) {
        result[sizeRead++] = '\n';
        result[sizeRead] = '\0';
    }

    if (write(resultFd, result, strlen(result)) == -1)
        errorHandler("write to results file");

    write(fifofd, result, sizeRead);

    while (ret && *pathIterator < pathc) {
        ret = sendFileToSlave(paths[*pathIterator], slaveNum);
        (*pathIterator)++;
    }
    if (ret == 0)
        (*filesSent)++;
}

void manageSlaves(int resultFd) {
    unsigned int slaveIterator = 0;
    int pathIterator = 0, filesSent = 0, filesReceived = 0;

    // Send initial files for slaves
    while (filesSent < SLAVELIMIT * INITIALFILESCOUNT && pathIterator < pathc){
        int ret = 1;
        while (ret && pathIterator < pathc) {
            ret = sendFileToSlave(paths[pathIterator], slaveIterator);
            pathIterator++;
        }
        if (ret == 0)
            filesSent++;
        slaveIterator = (slaveIterator + 1) % SLAVELIMIT;
    }

    fd_set fdSet;
    int nfds = getMaxFd(SLAVE_OUT) + 1;
    while (filesReceived < filesSent) {
        int numToRead, numRead;
        FD_ZERO(&fdSet);
        setFd(&fdSet, SLAVE_OUT);

        numRead = 0;
        numToRead = select(nfds, &fdSet, NULL, NULL, NULL);
        if (numToRead == -1)
            errorHandler("select");

        for (int i = 0; i < SLAVELIMIT; ++i) {
            if (FD_ISSET(SLAVE_OUT[i], &fdSet)) {
                readAndSendFileToSlave(&pathIterator, &filesReceived, &filesSent, i, resultFd);
                numRead++;
            }
        }
        if (numRead != numToRead)
            errorHandler("numRead != numToRead");
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {         //  Check if path is an argument.
        fprintf(stderr, "Usage: %s [FILE]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    paths = &(argv[1]);
    pathc = argc - 1;

    //  Create the results file
    int result_fd = open(RESULTPATH, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
    if (result_fd == -1)
        errorHandler("Open results");

    char * fifoName = "md5-fifoadadfdsa";
    if (mkfifo(fifoName, 0777)){
        errorHandler("mkfifo");
    }
    puts(fifoName);
    if (fflush(stdout))
        errorHandler("fflush");
    fifofd = open(fifoName, O_WRONLY);
    if (fifofd == -1){
        errorHandler("fifo open");
    }

    sleep(2);

    //  Create slaves and set fd.
    createSlaves();

    manageSlaves(result_fd);

    exit(EXIT_SUCCESS);
}

void createSlavePipes(int fd[2]) {
    if (pipe(fd))
        errorHandler("pipe");
}

void executeSlave(int master_to_slave[2], int slave_to_master[2]) {
    close(master_to_slave[1]);
    close(slave_to_master[0]);

    dup2(master_to_slave[0], 0);    //  Change stdin for pipe from master.
    dup2(slave_to_master[1], 1);    //  Change stdout for pipe to master.
    // dup2 also closes the original 0 and 1 file descriptors.

    if (execl(SLAVEPATH, SLAVEPATH, NULL)) { //  Execute the slave program.
        errorHandler("execl");
    }
}

void createSlaves() {
    int master_to_slave[2];
    int slave_to_master[2];

    for (int i = 0; i < SLAVELIMIT; ++i) {
        createSlavePipes(master_to_slave);

        createSlavePipes(slave_to_master);

        SLAVE_IN[i] = master_to_slave[1];   //  Write end of the pipe is saved by the master.
        SLAVE_OUT[i] = slave_to_master[0];  //  Read end of the pipe is saved by the master.

        int pid = fork();
        if (pid == 0) //  Child process modifies it's file descriptors and executes ./slave.
            executeSlave(master_to_slave, slave_to_master);
        if (pid == -1)
            errorHandler("fork");

        //  Close fd meant for the slave.
        close(master_to_slave[0]);
        close(slave_to_master[1]);
    }

}

void errorHandler(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
