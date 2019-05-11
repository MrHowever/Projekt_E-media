#include "jpeg_read.h"
#include "encryption.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char** argv)
{
    char* filename = 0;

    struct JPEG* jpeg;

    for(int i = 1; i < argc; i++) {
        filename = argv[i];
       jpeg = read_jpeg(filename);
       reconstruct_jpeg(jpeg,"reconstructed");
    }

  /*
  // read_jpeg("obrazek.jpeg");

  srand(time(NULL));

  struct public_key pubk;
  struct private_key privk;
  int arr_size = 5;
  uint64_t* val_arr = (uint64_t*) malloc(arr_size*sizeof(uint64_t));

  
  generate_keys(&pubk,&privk);

  
  for(int i = 0; i < arr_size; i++)
    val_arr[i] = 200000000000000000-i;;
               //1922431004861730761
              //34711169844624409
  printf("\nValues :");
  for(int i = 0; i < arr_size; i++)
    printf("%lu ",val_arr[i]);
  
  encrypt(val_arr,arr_size,&pubk);

  printf("\nEncrypted :");
  for(int i = 0; i < arr_size; i++)
    printf("%lu ",val_arr[i]);
  
  decrypt(val_arr,arr_size,&privk);

  printf("\nDecrypted :");
  for(int i = 0; i < arr_size; i++)
    printf("%lu ",val_arr[i]);

  */
  
  return 0;
}
