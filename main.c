#include "jpeg_read.h"
#include "encryption.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main()
{
  // read_jpeg("obrazek.jpeg");

  srand(time(NULL));

  int arr_size = 5;
  uint64_t* val_arr = (uint64_t*) malloc(arr_size*sizeof(uint64_t));
  struct public_key pubk;
  struct private_key privk;

  generate_keys(&pubk,&privk);
  
  for(int i = 0; i < arr_size; i++)
    val_arr[i] = i;

  encrypt(val_arr,arr_size,&pubk);

  printf("\nEncrypted :");
  for(int i = 0; i < arr_size; i++)
    printf("%lu ",val_arr[i]);
  
  decrypt(val_arr,arr_size,&privk);

  printf("\nDecrypted :");
  for(int i = 0; i < arr_size; i++)
    printf("%lu ",val_arr[i]);
  
  return 0;
}
