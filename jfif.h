#ifndef JFIF_H
#define JFIF_H

#include <stdint.h>

struct jfif_data
{
  uint8_t major_ver, minor_ver;
  uint8_t density_units;
  uint16_t x_density, y_density;
  uint8_t thumb_width, thumb_height;
  uint8_t* thumb_data;
};

struct jfif_data read_jfif(uint8_t*);
uint8_t read8(int*,uint8_t*);
void print_jfif(struct jfif_data);


#endif



