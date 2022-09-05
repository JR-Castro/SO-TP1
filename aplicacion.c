// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <errno.h>
#include <string.h>

#define SLAVELIMIT 5
#define SLAVEPATH "./slave"
#define RESULTPATH "./resultado"

int SLAVE_IN[SLAVELIMIT];   //  fd for slave inputs -> where the paths are sent to be processed by the slaves

int SLAVE_OUT[SLAVELIMIT];  //  fd for slave outputs -> where the slaves response is sent.

void getFilePaths(int amount, char * paths[], glob_t * globbuf);

void createSlavePipes(int fd[2]);

void executeSlave(int master_to_slave[2], int slave_to_master[2]);

void createSlaves();

void errorHandler(const char * msg);

int main(int argc, char * argv[]){

    if (argc < 2){         //  Check if path is an argument.
        fprintf(stderr, "Usage: %s <path_to_files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    glob_t globbuf;
    getFilePaths(argc-1, &(argv[1]), &globbuf);

    //  Create the results file
    errno = 0;
    int result_fd = open(RESULTPATH,O_CREAT|O_RDONLY|O_TRUNC, S_IWUSR|S_IRUSR);
    if (result_fd == -1)
        errorHandler("Open results");

    // TODO: Shared memory with view process

    sleep(2);

    //  Create slaves and set fd.
    createSlaves();

    // Check if a matched path is a file, if it's then send it to the slave int line.
    int slave_iterator = 0;
    for (int i = 0; i < globbuf.gl_pathc; ++i) {
        struct stat statbuffer;
        errno = 0;
        if (stat(globbuf.gl_pathv[i], &statbuffer) == -1)
            errorHandler("stat");

        // If it's a regular file, then send it to a slave to process.
        if (S_ISREG(statbuffer.st_mode)){
            //  Only check for errors, since when writing to a pipe, write blocks until there's enough space.
            errno = 0;
            if (write(SLAVE_IN[slave_iterator], globbuf.gl_pathv[i], strlen(globbuf.gl_pathv[i])) == -1)
                errorHandler("write to pipe");
            slave_iterator = (slave_iterator + 1) % SLAVELIMIT;
        }
    }
    // TODO: Change to send an initial ammount of files to each slave, then send them more as they finish, read results and save them to ./resultados and shared memory.

    //  Once all files are distributed between slaves, close the pipes to signal EOF.
    errno = 0;
    for (int i = 0; i < SLAVELIMIT; ++i) {
        if (close(SLAVE_IN[i]))
            errorHandler("Close pipes");
    }

    globfree(&globbuf);
}

void getFilePaths(int amount, char * paths[], glob_t * globbuf){
    for (int i = 1; i < amount; ++i) {
        errno = 0;
        int ans = glob(paths[i], GLOB_NOSORT | ( i > 0 ? GLOB_APPEND : 0), NULL, globbuf);
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

void errorHandler(const char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}
