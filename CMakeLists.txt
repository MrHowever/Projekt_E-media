cmake_minimum_required(VERSION 3.13)
project(Projekt_E_media)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Projekt_E_media
        encryption.c
        encryption.h
        exif.c
        exif.h
        file_read_func.c
        file_read_func.h
        jfif.c
        jfif.h
        jpeg_read.c
        jpeg_read.h
        main.c
        map.c
        map.h
        obrazek.jpeg
        obrazek1.jpeg dct.h dct.c)
