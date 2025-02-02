#ifndef FILE_READ_FUNC_H
#define FILE_READ_FUNC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

uint16_t read_word(FILE*);
uint16_t* read_n_words(int,FILE*);
uint8_t read_byte(FILE*);
uint8_t* read_n_bytes(int,FILE*);

uint32_t read32(uint8_t*,int*,int);
uint16_t read16(uint8_t*,int*,int);
uint32_t read_mot32(uint8_t*);
uint16_t read_mot16(uint8_t*);
uint32_t intel_to_mot32(uint8_t*);
uint16_t intel_to_mot16(uint8_t*);

uint8_t read8(int*,uint8_t*);

#endif
