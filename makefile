CC = gcc
FLAGS = -Wall -std=c99

all: md5 vista slave

md5: aplicacion.c
	${CC} ${FLAGS} aplicacion.c shmADT.c -o md5 -lrt -pthread

vista: vista.c
	${CC} ${FLAGS} vista.c shmADT.c -o vista -lrt -pthread

slave: esclavo.c
	${CC} ${FLAGS} esclavo.c -o slave

clean:
	rm md5 vista slave
