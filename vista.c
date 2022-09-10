// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "vista.h"

int main(int argc, char const *argv[]){

    char buffer[MAX_MEM_CHARS] = {0};
    
    if( argc == 1 ){
        if(read(STDIN, buffer, MAX_MEM_CHARS) == ERROR ) {
            //puedo usar scanf con barra
            errorHandler("Read failed");
        }
    } else {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        return -1;
    }
 
    int connect = connecshr(buffer)

    if(connect != SHMADT_ERROR){
        while(shmread() != null ){
            printf("%s \n",shmread())
        }
    }
    else
        return -1;

    readerDisconnect(); 

    return 0;
}

void errorHandler(const char *errorMsg) {
    char finalMsg[ERROR_MSG] = {0};
    strcat(finalMsg,errorMsg);
    strcat(finalMsg,"\n");
    perror(finalMsg);
    exit(EXIT_FAILURE);
}
