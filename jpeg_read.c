#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jpeg_read.h"
#include "map.h"
#include "exif.h"
#include "jfif.h"
#include "file_read_func.h"

int is_sof(uint8_t marker)
{
  return 0x0C == (marker >> 4) && marker != 0xC4 && marker != 0xC8 && marker != 0xCC;
}

void print_header(const char* text)
{
  printf("\n\n");
  
  for(int i = 0; i < 140; i++)
    printf("/");

  printf("\n");
  
  for(int i = 0; i < 60; i++)
    printf(" ");

  printf("%s\n",text);

  for(int i = 0; i < 140; i++)
    printf("/");

  printf("\n\n");
}

void read_sof(FILE* jpeg_file)
{
  uint16_t rows,columns;
  uint8_t components,comp_id,comp_sampling;
  
  read_word(jpeg_file);    //Read header length
  read_byte(jpeg_file);    //Read sample precision

  rows = read_word(jpeg_file);    //Read rows
  columns = read_word(jpeg_file); //Read columns
  components = read_byte(jpeg_file); //Read components

  print_header("SOF Marker Metadata");
  printf("Image size: %d x %d\nColor depth (Bit depth): %d\n",rows,columns,components);

  for(int i = 0; i < components; i++)
  {
    comp_id = read_byte(jpeg_file);
    comp_sampling = read_byte(jpeg_file);
    read_byte(jpeg_file);
    
    printf("Sampling factor of component %d:\n\tHorizontal: %d\n\tVertical: %d\n",comp_id,comp_sampling>>4,comp_sampling&0x0F);
  }
  
  //fseek(jpeg_file,9,1); //Skip remaininf information in this segment

}

int has_length(uint8_t marker)
{
  switch(marker)
  {
    case 0x00:
    case 0x01:
  case 0xD0:
  case 0xD1:
  case 0xD2:
  case 0xD3:
  case 0xD4:
  case 0xD5:
  case 0xD6:
  case 0xD7:
  case 0xD8:
  case 0xFF:
    return 0;

  default:
    return 1;
  }
}

int read_jpeg(const char* filename)
{
  FILE* jpeg_file = fopen(filename,"rb");                 //File stream
  uint8_t* c = 0;                                         //Variable to store a single byte of data
  uint8_t* metadata = 0;                                  //Pointer that will contain read metadata if it is encountered
  uint8_t* header = 0;                                    //Variable that will hold a header of the segment to check it svalidity
  uint16_t seg_size = 0;                                  //Size of a segment
  uint8_t prev = 0;                                       //Value of previous byte
  uint16_t jpeg_header;
  
  c = (uint8_t*) malloc(sizeof(uint8_t));
  
  if(!jpeg_file)
  {
    printf("Error. Unable to open file\n");
    return 0;
  }

  if((jpeg_header = read_word(jpeg_file)) != 0xFFD8)
  {
    printf("This is not an JPEG image!\n");
    return -1;
  }
  
  while(1)        //Read consecutive bytes to find markers
  {    
    fread(c,1,1,jpeg_file);

    if( prev == 0xFF )           //0xFF byte signals that next byte might be a marker
    {
      if( *c == 0xD9)
	break;
      else if(is_sof(*c))             //Detect SOF marker
	  read_sof(jpeg_file);
      else if (*c == 0xE0)       //Detect JFIF/JFXX marker
	{
	  seg_size = read_word(jpeg_file);

	  header = read_n_bytes(5,jpeg_file);

	  metadata = read_n_bytes(seg_size-7,jpeg_file);
	  
	  if(!strcmp(header,"JFIF\0"))
	    print_jfif(read_jfif(metadata));
	  else if(!strcmp(header,"JFXX\0"))
	    fseek(jpeg_file,seg_size-2,1);

	  free(header);
	  free(metadata);
	}
      else if( *c == 0xE1)           //APP1 marker equals 0xE1, Exif metadata
	{	  
	  seg_size = read_word(jpeg_file);
	  
	  metadata = read_n_bytes(seg_size-2,jpeg_file);   //Read metadata, -2 because the length contains already read 2 bytes of length

	  header = (uint8_t*) malloc(6*sizeof(uint8_t));
	  
	  strncpy(header,metadata,6);
	  
	  if(!strcmp(header,"Exif\0\0"))      //Check for valid Exif header
	    parse_exif_md(metadata);

	  free(header);
	  free(metadata);
	}
      else if(has_length(*c))            //If a segment that doesnt contain any useful metadata is encountered, jump through it
	{
	  seg_size = read_word(jpeg_file);
	  fseek(jpeg_file,seg_size-2,1);
	}
    }

    prev = *c;
  }

  fclose(jpeg_file);
  free(c);

  return 0;
}
