#ifndef EXIF_H
#define EXIF_H

#include "map.h"
#include <stdio.h>

struct ifd_offsets
{
  uint32_t ifd1_offset;
  uint32_t subifd_offset;
  uint32_t gps_offset;
};

void offsets_init(struct ifd_offsets*);
void fill_exif_map(struct Map*);
void fill_gps_map(struct Map*);
void fill_format_map(struct Map*);

void read_ifd(uint8_t*,int,int,struct Map*,struct ifd_offsets*,FILE*);
void parse_exif_md(uint8_t*,FILE*);
void print_exif_data(uint16_t,uint32_t,uint8_t*,int,int,FILE*);

uint32_t data_size(uint16_t,FILE*);
char* val_t(uint16_t,struct Map*);
int check_alignment(const char*, FILE*);

#endif
