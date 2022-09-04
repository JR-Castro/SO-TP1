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
#include "view.h"

int main(int argc, char const *argv[]){

    size_t mem;
    
    if( argc == 1 ){
        char buffer[MAX_MEM_CHARS] = {0};
        if(read(STDIN, buffer, MAX_MEM_CHARS) == ERROR ) {
            errorHandler("Read failed");
        }
        mem = atoi(buffer);
    } else if( argc == 2 ) {
        mem = atoi(argv[1]);
    } else {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
    }

    //crear una estructura para las vistas?
    //crar metodo para abrir y cerrar las vistas
    //

    return 0;
}

void errorHandler(const char *errorMsg) {
    char finalMsg[ERROR_MSG] = {0};
    strcat(finalMsg,errorMsg);
    strcat(finalMsg,"\n");
    perror(finalMsg);
    exit(EXIT_FAILURE);
}
