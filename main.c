#include "jpeg_read.h"
#include "exif.h"

int main()
{
  parse_exif_md(read_jpeg("obrazek.jpeg"));

  return 0;
}
