// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <errno.h>

#define SLAVELIMIT 5
#define SLAVEPATH "./salve"

int SLAVE_IN[SLAVELIMIT];   //  fd for slave inputs -> where the paths are sent to be processed by the slaves

int SLAVE_OUT[SLAVELIMIT];  //  fd for slave outputs -> where the slaves response is sent.

void createslaves();

int main(int argc, char * argv[]){

    if (argc != 2){         //  Check if path to files exist.
        fprintf(stderr, "Usage: %s <path_to_files>\n", argv[0]);
        exit(EXIT_FAILURE);mind that there are other ways to signal an error which, depending on your
    }

    glob_t globbuf;
    errno = 0;
    int ans = glob(argv[1], GLOB_NOSORT, NULL, &globbuf);
    if ( ans ){
        if (ans == GLOB_NOMATCH)    //  No files matched the path supplied.
            exit(EXIT_SUCCESS);
        perror("Glob");
        exit(EXIT_FAILURE);
    }

    // TODO: Shared memory with view process, open ./resultados

    //  Create slaves and set fd.
    createslaves();

    // TODO: Send files to slaves, read results and save them to ./resultados and shared memory

}

void createslavepipes(int fd[2]){
    errno = 0;
    if (pipe(fd)){
        perror("Pipe");
        exit(EXIT_FAILURE);
    }
}

void set_slave_fd(int master_to_slave[2], int slave_to_master[2]){
    close(master_to_slave[1]);
    close(slave_to_master[0]);

    dup2(master_to_slave[0], 0);    //  Change stdin for pipe from master.
    dup2(slave_to_master[1], 1);    //  Change stdout for pipe to master.
    // dup2 also closes the original 0 and 1 file descriptors.

    errno = 0;
    if (execl(SLAVEPATH, SLAVEPATH, NULL)){ //  Execute the slave program.
        perror("Execl");
        exit(EXIT_FAILURE);
    }
}

void createslaves(){
    int master_to_slave[2];
    int slave_to_master[2];

    for (int i = 0; i < SLAVELIMIT; ++i) {
        createslavepipes(master_to_slave);

        createslavepipes(slave_to_master);

        SLAVE_IN[i] = master_to_slave[1];   //  Write end of the pipe is saved by the master.
        SLAVE_OUT[i] = slave_to_master[0];  //  Read end of the pipe is saved by the master.

        if (fork()) //  Child process modifies it's file descriptors and executes ./slave.
            execute_slave(master_to_slave, slave_to_master);

        //  Close fd meant for the slave.
        close(master_to_slave[0]);
        close(slave_to_master[1]);
    }

}
