// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*
DEBE recibir por entrada estándar y como parámetro la información necesaria para
conectarse al buffer compartido. Esto DEBERÁ permitir utilizar un pipe para iniciar el
proceso aplicación y el vista: ./md5 files/* | ./vista, y también iniciar la aplicación y
más tarde la vista: ./solve files/* en una terminal o en background y ./vista <info> en
otra terminal o en foreground.
DEBE mostrar en pantalla el contenido del buffer de llegada a medida que se va
cargando el mismo. El buffer DEBE tener la siguiente información:
○ Nombre de archivo.
○ Md5 del archivo.
○ ID del esclavo que lo procesó.
*/
#include "vista.h"

int main(int argc, char const *argv[]){

    int shmSize;
    char buffer[MAX_MEM_CHARS] = {0};
    
    if( argc == 1 ){
        if(read(STDIN, buffer, MAX_MEM_CHARS) == ERROR ) {
            //puedo usar scanf con barra
            errorHandler("Read failed");
        }
        shmSize = atoi(argv[1])
    } else if (argc == 2){
        shmSize =  atoi(argv[1]);
    } else {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        return -1;
    }
 
    int connect = connecshr(buffer)
    int count = 0;

    if(connect != SHMADT_ERROR){
        while(count < shmSize){
            printf("%s \n",shmread())
        }
    }
    else
        return SHMADT_ERROR;

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
