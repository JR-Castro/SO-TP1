// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//MUST to recieve the paths of the files to process y it MUST initiate the program corresponding for processing them (md5sum)
//MUST send the information relevant to processing to the process aplication
//MUSTN'T dump the result in a file in disc, to then read it from the slave, it MUST recieve the output of md5sum utilizing
//some mechanism of IPC more sophisticated

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


int main(void)
{
    char* path;
    char* msg;
    char* alphanumber;
    char* filename;
    char* final;
    ssize_t rvalue;
    int filesize;

  do {
    rvalue = read(stdin,path,BUFSIZ);
    FILE* file = popen(strcat("md5sum ", path), "r");
    if(file == NULL)
        perror("Error in popen.");
    
    fgets(msg, 1024, file);
    alphanumber = strtok(msg, "  ");
    filename = strtok(NULL, "  ");

    final = strcat(alphanumber, " - ");
    final = strcat(final, filename);

    pclose(file);
    if(write(stdout, final, strlen(final) + 1) < 0)
        perror("Error on write.");

  } while (rvalue > 0);
  
  return rvalue;
}