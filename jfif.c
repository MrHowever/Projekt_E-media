#include "jfif.h"
#include "jpeg_read.h"
#include "file_read_func.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct jfif_data read_jfif(uint8_t* data)
{
  struct jfif_data jdata;
  int offset = 0;
  int thumb_size = 0;
  
  jdata.major_ver = read8(&offset,data);
  jdata.minor_ver = read8(&offset,data);
  jdata.density_units = read8(&offset,data);
  jdata.x_density = read16(data,&offset,1);
  jdata.y_density = read16(data,&offset,1);
  jdata.thumb_width = read8(&offset,data);
  jdata.thumb_height = read8(&offset,data);

  thumb_size = 3*jdata.thumb_width*jdata.thumb_height;
  
  jdata.thumb_data = (uint8_t*) malloc(thumb_size*sizeof(uint8_t));

  memcpy(jdata.thumb_data,data,thumb_size);

  return jdata;
}

void print_jfif(struct jfif_data data)
{
  print_header("JFIF Metadata");
  printf("JFIF version: %d.%d\n", data.major_ver,data.minor_ver);
  printf("Density units: ");

  switch(data.density_units)
    {
    case 0:
      printf("No units\n");
      break;
    case 1:
      printf("Pixels per inch\n");
      break;
    case 2:
      printf("Pixels per centimeter\n");
      break;
    }

  printf("Horizontal pixel density: %d\n",data.x_density);
  printf("Vertical pixel density: %d\n",data.y_density);
  printf("Horizontal pixel count of the thumbnail: %d\n",data.thumb_width);
  printf("Vertical pixel count of the thumbnail: %d\n",data.thumb_height);
}
