CC = gcc
FLAGS = -g -Wall -std=c99

all: md5 vista esclavo

md5: aplicacion.c shmADT.c
	${CC} ${FLAGS} aplicacion.c -o md5

vista: vista.c shmADT.c
	${CC} ${FLAGS} vista.c -o vista

esclavo: esclavo.c
	${CC} ${FLAGS} esclavo.c -o esclavo

clean:
	rm -f md5 vista esclavo
