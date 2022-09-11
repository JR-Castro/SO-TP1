CC = gcc
FLAGS = -Wall -std=c99

all: md5 vista esclavo

md5: aplicacion.c shmADT.c
	${CC} ${FLAGS} aplicacion.c shmADT.c -o md5 -lrt -pthread

vista: vista.c shmADT.c
	${CC} ${FLAGS} vista.c shmADT.c -o vista -lrt -pthread

esclavo: esclavo.c
	${CC} ${FLAGS} esclavo.c -o esclavo

clean:
	rm md5 vista esclavo
