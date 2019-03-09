#include "file_read_func.h"

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

uint8_t read8(int* offset, uint8_t* data)
{
  uint8_t byte = data[*offset];
  (*offset)++;
  return byte;
}
