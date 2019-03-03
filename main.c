#include "jpeg_read.h"

int main()
{
  parse_exif_md(read_jpeg("obrazek.jpeg"));

  return 0;
}
