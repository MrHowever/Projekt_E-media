CC = gcc

all: main.o jpeg_read.o map.o exif.o jfif.o file_read_func.o encryption.o
	$(CC) main.o jpeg_read.o map.o exif.o jfif.o file_read_func.o encryption.o -lm

main.o: main.c
	$(CC) -c main.c -lm

jpeg_read.o: jpeg_read.c jpeg_read.h
	$(CC) -c jpeg_read.c -lm

map.o: map.c map.h
	$(CC) -c map.c -lm

exif.o: exif.c exif.h
	$(CC) -c exif.c -lm

jfif.o: jfif.c jfif.h
	$(CC) -c jfif.c -lm

file_read_func.o: file_read_func.c file_read_func.h
	$(CC) -c file_read_func.c -lm

encryption.o: encryption.c encryption.h
	$(CC) -c encryption.c -lm

clean:
	rm -f *.o
