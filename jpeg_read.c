#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jpeg_read.h"
#include "map.h"
#include "exif.h"
#include "jfif.h"


uint16_t read_word(FILE* file)
{
  uint8_t* first = (uint8_t*) malloc(sizeof(uint8_t));
  uint8_t* second = (uint8_t*) malloc(sizeof(uint8_t));

  fread(first,1,1,file);
  fread(second,1,1,file);

  uint16_t first_val = (*first) << 8;
  uint16_t second_val = *second;

  free(first);
  free(second);
  
  return first_val | second_val;
}

uint8_t read_byte(FILE* file)
{
  uint8_t* ptr = (uint8_t*) malloc(sizeof(uint8_t));
  uint8_t val = 0;
  fread(ptr,1,1,file);
  val = *ptr;
  free(ptr);
  return val;
}

uint16_t* read_n_words(int n, FILE* file)
{
  uint16_t* arr = (uint16_t*) malloc(n*sizeof(uint16_t));

  for(int i = 0; i < n; i++)
    arr[i] = read_word(file);

  return arr;
}

uint8_t* read_n_bytes(int n, FILE* file)
{
  uint8_t* arr = (uint8_t*) malloc(n*sizeof(uint8_t));

  for(int i = 0; i < n; i++)
    arr[i] = read_byte(file);

  return arr;
}

uint16_t intel_to_mot16(uint8_t* first)
{
  uint16_t first16 = (uint16_t) *first;
  uint16_t second16 = (uint16_t) *(first+1);
  
  return (second16 << 8) | first16;
}

uint32_t intel_to_mot32(uint8_t* first)
{
  uint32_t first32 = (uint32_t) *first;
  uint32_t second32 = (uint32_t) *(first+1);
  uint32_t third32 = (uint32_t) *(first+2);
  uint32_t fourth32 = (uint32_t) *(first+3);

  return (fourth32 << 24) | (third32 << 16) | (second32 << 8) | first32;
}

uint16_t read_mot16(uint8_t* first)
{
  uint16_t first16 = (uint16_t) *first;
  uint16_t second16 = (uint16_t) *(first+1);
  
  return (first16<<8) | second16;
}

uint32_t read_mot32(uint8_t* first)
{
  uint32_t first32 = (uint32_t) *first;
  uint32_t second32 = (uint32_t) *(first+1);
  uint32_t third32 = (uint32_t) *(first+2);
  uint32_t fourth32 = (uint32_t) *(first+3);
  
  return (first32<<24) | (second32<<16) | (third32<<8) | fourth32;
}

uint16_t read16(uint8_t* first, int* offset, int allign)
{
  int off = *offset;
  *offset += 2;

  if(allign)
    return read_mot16(first+off);
  else
    return intel_to_mot16(first+off);
}

uint32_t read32(uint8_t* first, int* offset, int allign)
{
  int off = *offset;
  *offset += 4;
  
  if(allign)
    return read_mot32(first+off);
  else
    return intel_to_mot32(first+off);
}

int is_sof(uint8_t marker)
{
  return 0x0C == (marker >> 4) && marker != 0xC4 && marker != 0xC8 && marker != 0xCC;
}

void read_sof(FILE* jpeg_file)
{
  uint8_t* byte = (uint8_t*) malloc(sizeof(uint8_t));
  uint8_t* word = (uint8_t*) malloc(2*sizeof(uint8_t));
  uint16_t rows,columns;
  uint8_t components;
  
  read_word(jpeg_file);    //Read header length
  read_byte(jpeg_file);    //Read sample precision

  rows = read_word(jpeg_file);    //Read rows
  columns = read_word(jpeg_file); //Read columns
  components = read_byte(jpeg_file); //Read components

  fseek(jpeg_file,9,1); //Skip remaininf information in this segment
  
  printf("Image size: %d x %d\nColor depth (Bit depth): %d\n",rows,columns,components);
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
  
  c = (uint8_t*) malloc(sizeof(uint8_t));
  
  if(!jpeg_file)
  {
    printf("Error. Unable to open file\n");
    return 0;
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
	}
      else if( *c == 0xE1)           //APP1 marker equals 0xE1, Exif metadata
	{	  
	  seg_size = read_word(jpeg_file);
	  
    	  metadata = (uint8_t*) malloc(seg_size-2);	  
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
