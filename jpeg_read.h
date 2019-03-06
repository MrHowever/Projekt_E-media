#ifndef JPEG_READ_H
#define JPED_READ_H

#include <stdio.h>
#include <stdint.h>

int read_jpeg(const char*);

uint16_t intel_to_mot16(uint8_t*);
uint32_t intel_to_mot32(uint8_t*);
uint16_t read_mot16(uint8_t*);
uint32_t read_mot32(uint8_t*);
uint16_t read16(uint8_t*,int*,int);
uint32_t read32(uint8_t*,int*,int);

#endif
