#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jpeg_read.h"
#include "map.h"
#include "exif.h"

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
  uint8_t* rows,*columns;
  uint8_t* components; //a

  rows = (uint8_t*) malloc(2*sizeof(uint8_t));
  columns = (uint8_t*) malloc(2*sizeof(uint8_t));
  components = (uint8_t*) malloc(sizeof(uint8_t));
  
  fread(word,1,2,jpeg_file);    //Read header length
  fread(byte,1,1,jpeg_file);    //Read sample precision

  printf("found marker Cx and size = %d %d in offset %ld\n",word[0],word[1],ftell(jpeg_file));
  
  fread(rows,2,1,jpeg_file);    //Read rows
  fread(columns,2,1,jpeg_file); //Read columns
  fread(components,1,1,jpeg_file); //Read components

  fseek(jpeg_file,9,1); //Skip remaininf information in this segment
  
  printf("Image size: %d x %d\nColor depth (Bit depth): %d\n",rows[0]*256+rows[1],columns[0]*256+columns[1],*components);
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

uint8_t* read_jpeg(const char* filename)
{
  FILE* jpeg_file = fopen(filename,"rb");                 //File stream
  uint8_t* c = 0;                                         //Variable to store a single byte of data
  int end = 1;                                            //Boolean variable to end the loop
  uint8_t* metadata = 0;
  uint16_t mdata_size = 0;
  uint8_t* header = 0;
  uint16_t seg_size = 0;
  int x = 0;
  uint8_t prev = 0;
  
  c = (uint8_t*) malloc(sizeof(uint8_t));

  
  fseek(jpeg_file,0L,SEEK_END);
  printf("File size = %ld\n",ftell(jpeg_file));
  rewind(jpeg_file);
  
  if(!jpeg_file)
  {
    printf("Error. Unable to open file\n");
    return 0;
  }

  
  
  while(1)        //Read consecutive bytes to find markers
  {    
    end = fread(c,1,1,jpeg_file);

    if(prev == 0xFF && *c == 0xD9)
      {
	printf("found marker %X  in offset %ld\n",*c,ftell(jpeg_file));
	break;
      }
    
    if( prev == 0xFF )           //0xFF byte signals that next byte might be a marker
    {
      if(*c != 0 && *c != 0xFF)
	printf("Marker = %X\n",*c);
      
      if(is_sof(*c))
	{
	  printf("sof marker = %X\n",*c);
	  read_sof(jpeg_file);
	}	
      else if( *c == 0xE1)           //APP1 marker equals 0xE1
	{
	  printf("Znaleziono metadane Exif\n");
	  printf("\nptr position = %ld\n",ftell(jpeg_file));
	  
	  mdata_size = read_word(jpeg_file);

	  printf("e1 block size = %d\n",mdata_size);
	  
    	  metadata = (uint8_t*) malloc(mdata_size-2);	  
	  metadata = read_n_bytes(mdata_size-2,jpeg_file);   //Read metadata, -2 because the length contains already read 2 bytes of length
	   
	  header = read_n_bytes(6,jpeg_file);
	  
	  strncpy(header,metadata,6);
	  
	  if(!strcmp(header,"Exif\0\0"))      //Check for valid Exif header
	    parse_exif_md(metadata);

	  printf("offset after e1 %ld\n",ftell(jpeg_file));
	}
      else if(has_length(*c))
	{
	  seg_size = read_word(jpeg_file);
	  printf("found marker %X and size = %d in offset %ld\n",*c,seg_size,ftell(jpeg_file));

	  fseek(jpeg_file,seg_size-2,1);
	}
    }

    prev = *c;
   
  }
  return metadata;
}
