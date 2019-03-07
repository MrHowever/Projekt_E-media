CC = gcc

all: main.o jpeg_read.o map.o exif.o jfif.o file_read_func.o
	$(CC) main.o jpeg_read.o map.o exif.o jfif.o file_read_func.o

main.o: main.c
	$(CC) -c main.c

jpeg_read.o: jpeg_read.c jpeg_read.h
	$(CC) -c jpeg_read.c

map.o: map.c map.h
	$(CC) -c map.c

exif.o: exif.c exif.h
	$(CC) -c exif.c

jfif.o: jfif.c jfif.h
	$(CC) -c jfif.c

file_read_func.o: file_read_func.c file_read_func.h
	$(CC) -c file_read_func.c

clean:
	rm -f *.o
