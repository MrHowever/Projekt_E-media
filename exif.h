#ifndef EXIF_H
#define EXIF_H

#include "map.h"

void fill_exif_map(struct Map*);
void parse_exif_md(uint8_t*);
uint32_t print_exif_data(uint16_t,uint32_t,uint8_t*,int,int,int);
uint32_t data_size(uint16_t);
uint32_t read_ifd(uint8_t*,int,int,struct Map*,uint32_t*);
char* val_t(uint16_t);

#endif
