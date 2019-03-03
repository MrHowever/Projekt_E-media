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


uint8_t* read_jpeg(const char* filename)
{
  FILE* jpeg_file = fopen(filename,"rb");                 //File stream
  uint8_t* c = 0;                                         //Variable to store a single byte of data
  int end = 0;                                            //Boolean variable to end the loop
  uint16_t* mdata_size = 0;                               //Size of metadata block
  uint8_t* metadata = 0;                                  //Variable holding the metadata
  
  c = (uint8_t*) malloc(sizeof(uint8_t));
  mdata_size = (uint16_t*) malloc(sizeof(uint16_t));
  
  if(!jpeg_file)
  {
    printf("Error. Unable to open file\n");
    return 0;
  }

  while(!end)        //Read consecutive bytes to find the APP1 marker
  {
    fread(c,1,1,jpeg_file);
 
    if( *c == 0xFF )           //0xFF byte signals that next byte is a marker
    {
      fread(c,1,1,jpeg_file);

      if( *c == 0xD9)                            //D9 is an EOF/EOI marker
      {
	printf("\nExif metadata not found\n");
	return 0;
      }
      
      if( *c == 0xE1)           //APP1 marker equals 0xE1
      {
	end = 1;
	printf("Znaleziono metadane Exif\n");
      }	
    }
  }
  
  fread(mdata_size,2,1,jpeg_file);  //Read Exif data size

  printf("Exif metadata size = %d\n", *mdata_size);
  
  metadata = (uint8_t*) malloc(*mdata_size);
  
  fread(metadata,1,*mdata_size,jpeg_file);  //Read metadata
  
  return metadata;
}

void parse_exif_md(uint8_t* metadata)
{
  const char exif_header[7] = {metadata[0],metadata[1],metadata[2],metadata[3],metadata[4],metadata[5], '\0'};
  const char byte_align[3] = {metadata[6],metadata[7],'\0'};
  int align;
  int offset = 0;
  uint32_t IFDoffset = 0;
  uint16_t directory_entries = 0;
  uint16_t tag = 0, data_format = 0;
  uint32_t components_num = 0, entry_data = 0;
  struct Map dict;

  map_init(&dict);
  fill_exif_map(&dict);
  
  if(strcmp(exif_header,"Exif\0\0"))      //Check for valid Exif header
  {
    printf("\nCorrupted Exif format. Exiting.\n");
    return;
  }

  if(!strcmp(byte_align,"II"))
  {
    align = 0;
  }
  else if(!strcmp(byte_align,"MM"))
  {
    align = 1;
  }
  else
  {
    printf("\nCorrupted byte allign information\n");
    return;
  }  

  offset = 8;

  if(read16(metadata,&offset,align) != 0x002A)    //Check for correct TAG mark
  {
    printf("\nCorrupted TAG mark\n");
    return;
  }

  IFDoffset = read32(metadata,&offset,align);    //Offset to first IFD (IFD0)

  //Move to IFD0
  offset = IFDoffset + 6;                        //6 byte offset comes from "Exif\0\0"

  directory_entries = read16(metadata,&offset,align);  //Read the number of directory entries

  printf("entries = %d\n", directory_entries);
  
  //Read IFD0
  for(uint16_t i = 0; i < directory_entries; i++)
  {
    tag = read16(metadata,&offset,align);
    data_format = read16(metadata,&offset,align);
    components_num = read32(metadata,&offset,align);
    entry_data = read32(metadata,&offset,align);

    printf("%d %s\n",tag, map_find(&dict, tag));
  }
}
