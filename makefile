CC = gcc

all: main.o jpeg_read.o map.o exif.o
	$(CC) main.o jpeg_read.o map.o exif.o

main.o: main.c
	$(CC) -c main.c

jpeg_read.o: jpeg_read.c jpeg_read.h
	$(CC) -c jpeg_read.c

map.o: map.c map.h
	$(CC) -c map.c

exif.o: exif.c exif.h
	$(CC) -c exif.c

clean:
	rm -f *.o
