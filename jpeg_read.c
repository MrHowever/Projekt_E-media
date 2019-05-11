#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jpeg_read.h"
#include "map.h"
#include "exif.h"
#include "jfif.h"
#include "file_read_func.h"
#include "encryption.h"

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

void read_sof(FILE* jpeg_file, struct JPEG* jpeg)
{
  uint16_t rows,columns;
  uint8_t components,comp_id,comp_sampling;
  
  jpeg->sof.header_length = read_word(jpeg_file);    //Read header length
  jpeg->sof.sample_precision = read_byte(jpeg_file);    //Read sample precision

  jpeg->sof.rows = read_word(jpeg_file);//rows = read_word(jpeg_file);    //Read rows
  jpeg->sof.cols = read_word(jpeg_file);//columns = read_word(jpeg_file); //Read columns
  jpeg->sof.comp_count = read_byte(jpeg_file);//components = read_byte(jpeg_file); //Read components

  print_header("SOF Marker Metadata");
  printf("Image size: %d x %d\nColor depth (Bit depth): %d\n",
          jpeg->sof.rows,
          jpeg->sof.cols,
          jpeg->sof.comp_count);

  jpeg->sof.components = (struct Components*) malloc(jpeg->sof.comp_count*sizeof(struct Components));

  for(int i = 0; i < jpeg->sof.comp_count; i++)
  {
    jpeg->sof.components[i].id = read_byte(jpeg_file);
    jpeg->sof.components[i].sampling = read_byte(jpeg_file);//comp_sampling = read_byte(jpeg_file);
    jpeg->sof.components[i].dest_selector = read_byte(jpeg_file);
    
    printf("Sampling factor of component %d:\n\tHorizontal: %d\n\tVertical: %d\n",
            jpeg->sof.components[i].id,
            jpeg->sof.components[i].sampling>>4,
            jpeg->sof.components[i].sampling&0x0F);
  }
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
    case 0xD9:
    case 0xFF:
        return 0;

    default:
        return 1;
  }
}

uint64_t* to64bit(uint8_t* input, long size)
{
    uint64_t* output = (uint64_t*) malloc(sizeof(uint8_t)*size);
    uint8_t* values = (uint8_t*) malloc(sizeof(uint8_t)*8);
    uint64_t single_output_value = 0;

    printf("\n\nsize = %ld", size);
    fflush(NULL);

    int j = 0, n = 0, m = 0, i = 0, x = 0;
    for(; (j+8) < size; j+=8,n++) {
        for (m = 0, i = j; m < 8 && i < size; i++, m++) {
            values[m] = input[i];
        }

        single_output_value = 0;
        for (x = 0; x < m; x++) {
            single_output_value |= ((uint64_t) values[x]) << (56 - (x * 8));
        }

        output[n] = single_output_value;
    }

    return output;
}

uint8_t* to8bit(uint64_t* input, long size)
{
   uint8_t* output = (uint8_t*) malloc(sizeof(uint8_t)*size);

   for(int i = 0, j = 0; i < size; i++) {
       output[i] = (uint8_t) ((input[i/8] >> (56 - ((i%8)*8))) & 255);
    }

   return output;
}

uint8_t* to8bitfrom16(uint16_t input)
{
    uint8_t* output = (uint8_t*) malloc(sizeof(uint8_t)*2);

    output[0] = (uint8_t) ((input >> 8) & 255);
    output[1] = (uint8_t) (input & 255);

    return output;
}


int copy_image(const char* filename)
{
    FILE* jpeg_image = fopen(filename,"rb");
    FILE* jpeg_copy = 0;
    uint8_t* file_data = 0;
    uint8_t* image_data = 0;
    uint64_t* image_data_64bit = 0;
    long file_length = 0;

    //Create copy file
    char* cpy_filename = malloc(strlen(filename)+strlen(".cpy")+1);
    cpy_filename[0] = '\0';    //Ensures the string is empty
    strcat(cpy_filename,filename);
    strcat(cpy_filename,".cpy");
    jpeg_copy = fopen(cpy_filename,"wb");

    //Find size of original image
    fseek(jpeg_image,0,SEEK_END);
    file_length = ftell(jpeg_image);
    rewind(jpeg_image);

    //Copy image
    file_data = (uint8_t*) malloc((file_length+1)*sizeof(uint8_t));
    fread(file_data,file_length,1,jpeg_image);
    fclose(jpeg_image);

    long i = 0;
    while( i < file_length)
    {
        if(file_data[i] == 0xFF) {
            if(file_data[i+1] == 0xD8)
                i += 2;
            if(file_data[i+1] == 0xD8)
                i += 2;

        }
       // !(file_data[i] == 0xFF && file_data[i+1] == 0xDA)
        i++;
    };

    i += 2;
    fwrite(file_data,i,1,jpeg_copy);

    file_data += i;

    //TODO check if image is not the last segment of jpeg

    long j=i;
    while(j < 5011879)
    {
        if(file_data[j] == 0xFF && file_data[j+1] == 0xD9)
            printf("\n\ni = %ld, j = %ld\n\n",i,j);

        j++;
    }

    struct public_key pubk;
    struct private_key privk;
    generate_keys(&pubk,&privk);

    int actual_image_size = file_length - (i+4);

    image_data = (uint8_t*) malloc(sizeof(uint8_t)*actual_image_size);
    memcpy(image_data,file_data,file_length-(i+4));

    FILE* tempfile = fopen("encrypteddata","wb");
    FILE* tempfile2 = fopen("unencrypteddata","wb");

    image_data_64bit = (uint64_t*) malloc(sizeof(uint8_t)*actual_image_size);
    image_data_64bit = to64bit(image_data,actual_image_size);
    fwrite(image_data_64bit,actual_image_size,1,tempfile2);
    encrypt(image_data_64bit,actual_image_size/8,&pubk);
    image_data = to8bit(image_data_64bit,actual_image_size);        //TODO last byte is different
    fwrite(image_data,file_length-(i+4),1,tempfile);

    file_data += file_length-(i+4);

    fwrite(file_data,2,1,jpeg_copy);

    fclose(jpeg_image);
    fclose(jpeg_copy);
    return 0;
}

void init_jpeg(struct JPEG* jpeg)
{
    jpeg->unrecognized = NULL;
    jpeg->unrecognized_count = 0;
    jpeg->eoi = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->sos.image_data = 0;
    jpeg->sos.data_size = 0;
    jpeg->app0.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->app1.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->sof.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->sos.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->dht.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->dqt.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->order = (char*) malloc(sizeof(char)*256);
    jpeg->order_size = 0;
}

void destroy_jpeg(struct JPEG* jpeg)
{
    free(jpeg->eoi);
    free(jpeg->leftovers);

    for(int i = 0; i < jpeg->unrecognized_count; i++) {
        free(jpeg->unrecognized[i].marker);
        free(jpeg->unrecognized[i].data);
    }
    free(jpeg->unrecognized);

    free(jpeg->app0.marker);
    free(jpeg->app0.header);
    free(jpeg->app0.metadata);

    free(jpeg->app1.marker);
    free(jpeg->app1.header);
    free(jpeg->app1.metadata);

    free(jpeg->sof.marker);
    free(jpeg->sof.components);

    free(jpeg->sos.marker);
    free(jpeg->sos.header);
    free(jpeg->sos.image_data);

    free(jpeg->dht.marker);
    free(jpeg->dht.data);

    free(jpeg->dqt.marker);
    free(jpeg->dqt.data);
}

void print_app(FILE* file, struct APP* app)
{
    if(app) {
        fwrite(app->marker, 2, 1, file);
        fwrite(to8bitfrom16(app->size), 2, 1, file);
        fwrite(app->header, app->header_size, 1, file);
        fwrite(app->metadata, app->size - app->header_size-2, 1, file);
    }
}

void print_sof(FILE* file, struct SOF* sof)
{
    if(sof) {
        fwrite(sof->marker, 2, 1, file);
        fwrite(to8bitfrom16(sof->header_length), 2, 1, file);
        fwrite(&sof->sample_precision, 1, 1, file);
        fwrite(to8bitfrom16(sof->rows), 2, 1, file);
        fwrite(to8bitfrom16(sof->cols), 2, 1, file);
        fwrite(&sof->comp_count, 1, 1, file);

        for (int i = 0; i < sof->comp_count; i++) {
            fwrite(&sof->components[i].id, 1, 1, file);
            fwrite(&sof->components[i].sampling, 1, 1, file);
            fwrite(&sof->components[i].dest_selector, 1, 1, file);
        }
    }
}

void print_sos(FILE* file, struct SOS* sos)
{
    if(sos) {
        fwrite(sos->marker, 2, 1, file);
        fwrite(to8bitfrom16(sos->header_size),2,1,file);
        fwrite(sos->header, sos->header_size-2, 1, file);
        fwrite(sos->image_data, sos->data_size, 1, file);
    }
}

void print_unrecognized(FILE* file, struct Unrecognized* unrecognized)
{
    if(unrecognized) {
        fwrite(unrecognized->marker, 2, 1, file);
        fwrite(to8bitfrom16(unrecognized->size), 2, 1, file);
        fwrite(unrecognized->data, unrecognized->size-2, 1, file);
    }
}

void print_dht(FILE* file, struct DHT* dht)
{
    if(dht) {
        fwrite(dht->marker, 2, 1, file);
        fwrite(to8bitfrom16(dht->size), 2, 1, file);
        fwrite(dht->data, dht->size-2, 1, file);
    }
}

void print_dqt(FILE* file, struct DQT* dqt)
{
    if(dqt) {
        fwrite(dqt->marker, 2, 1, file);
        fwrite(to8bitfrom16(dqt->size), 2, 1, file);
        fwrite(dqt->data, dqt->size-2, 1, file);
    }
}

void print_soi(FILE* file, uint16_t soi)
{
    if(soi) {
        fwrite(to8bitfrom16(soi), 2, 1, file);
    }
}

void print_eoi(FILE* file, uint8_t* eoi)
{
    if(eoi) {
        fwrite(eoi, 2, 1, file);
    }
}

void reconstruct_jpeg(struct JPEG* jpeg, const char* filename)
{
    FILE* output_file = fopen(filename,"wb");

    for(int i = 0; i < jpeg->order_size; i++) {
        switch(jpeg->order[i]) {
            case

        }
    }
    print_soi(output_file,jpeg->soi);
   // print_app(output_file,&jpeg->app0);
    print_app(output_file,&jpeg->app1);
    print_dqt(output_file,&jpeg->dqt);
    print_sof(output_file,&jpeg->sof);
    print_dht(output_file,&jpeg->dht);

/*
    for(int i = 0; i < jpeg->unrecognized_count; i++) {
        print_unrecognized(output_file,&jpeg->unrecognized[i]);
    }
*/
    print_sos(output_file,&jpeg->sos);
    print_eoi(output_file,jpeg->eoi);
}

void read_sos(FILE* jpeg_file, struct JPEG* jpeg)
{
    uint8_t* c = (uint8_t*) malloc(sizeof(uint8_t));
    uint8_t prev = 0;
    long eoi_pos = 0;
    long data_begin_pos = 0;

    jpeg->sos.header_size = read_word(jpeg_file);
    jpeg->sos.header = read_n_bytes(jpeg->sos.header_size-2,jpeg_file);

    data_begin_pos = ftell(jpeg_file);
    printf("\nBegin image data = %ld",ftell(jpeg_file));

    while(1)
    {
        fread(c, 1, 1, jpeg_file);

        if (prev == 0xFF && *c == 0xD9) {
            jpeg->eoi[0] = prev;
            jpeg->eoi[1] = *c;
            eoi_pos = ftell(jpeg_file) - 1;
            printf("\nEOI pos = %ld\n",ftell(jpeg_file));
            break;
        }

        prev = *c;
    }

    jpeg->sos.data_size = eoi_pos - data_begin_pos-1;

    printf("\nPosition after reading sos = %ld\n",ftell(jpeg_file));

    fseek(jpeg_file,data_begin_pos,SEEK_SET);
    printf("\nPosition after rewinding = %ld\n",ftell(jpeg_file));
    printf("\nSOS size = %ld\n",jpeg->sos.data_size);

    jpeg->sos.image_data = (uint8_t*) malloc(jpeg->sos.data_size*sizeof(uint8_t));
    fread(jpeg->sos.image_data,jpeg->sos.data_size,1,jpeg_file);
    printf("\nPosition after reading sos = %ld",ftell(jpeg_file));
}

void add_to_order(char id,struct JPEG* jpeg)
{
    jpeg->order[jpeg->order_size] = id;
    jpeg->order_size++;
}

struct JPEG* read_jpeg(const char* filename)
{
  FILE* jpeg_file = fopen(filename,"rb");                 //File stream
  uint8_t* c = (uint8_t*) malloc(sizeof(uint8_t));        //Variable to store a single byte of data
  uint8_t prev = 0;                                       //Value of previous byte
  struct JPEG* jpeg = (struct JPEG*) malloc(sizeof(struct JPEG));
  long file_length = 0;
  init_jpeg(jpeg);

    //Find size of original image
    fseek(jpeg_file,0,SEEK_END);
    file_length = ftell(jpeg_file);
    rewind(jpeg_file);

    printf("File length = %ld",file_length);

    if(c == NULL) {
      fprintf(stderr,"Memory error. Unable to allocate neccessary memory.\n");
      return 0;
  }

  if(!jpeg_file)
  {
    fprintf(stderr,"Error. Unable to open file\n");
    return 0;
  }

  if((jpeg->soi = read_word(jpeg_file)) != 0xFFD8)
  {
    fprintf(stderr,"This is not an JPEG image!\n");
    return 0;
  }

  add_to_order('i',jpeg);

  printf("Beginning = %ld\n",ftell(jpeg_file));

  while(1)        //Read consecutive bytes to find markers
  {
      fread(c,1,1,jpeg_file);

      if( prev == 0xFF ) {                      //0xFF byte signals that next byte might be a marker
          printf("\n\nFound marker 0x%X",*c);

          switch(*c)
          {
              // case 0xD9:                      //EOF marker
                //  goto loopend;

               case 0xE0:                      //JFIF metadata marker
                   printf("APP0 = %ld\n",ftell(jpeg_file));
                    jpeg->app0.header_size = 5;
                    jpeg->app0.marker[0] = prev;
                    jpeg->app0.marker[1] = *c;
                    jpeg->app0.size = read_word(jpeg_file);//seg_size = read_word(jpeg_file);
                    jpeg->app0.header = read_n_bytes(jpeg->app0.header_size,jpeg_file);//header = read_n_bytes(5,jpeg_file);
                    jpeg->app0.metadata = read_n_bytes(jpeg->app0.size-7,jpeg_file);//metadata = read_n_bytes(seg_size-7,jpeg_file);

                    add_to_order('a',jpeg);

                    printf(", size = %d\n",jpeg->app0.size);
                    if(!strcmp(jpeg->app0.header,"JFIF\0"))
                          print_jfif(read_jfif(jpeg->app0.metadata));
                    else if(!strcmp(jpeg->app0.header,"JFXX\0"))
                          fprintf(stderr,"\nUnrecognized APP0 format\n");
                      break;

               case 0xE1:                      //EXIF metadata marker
                   printf("APP1 = %ld\n",ftell(jpeg_file));
                    jpeg->app1.header_size = 6;
                    jpeg->app1.marker[0] = prev;
                    jpeg->app1.marker[1] = *c;
                    jpeg->app1.size = read_word(jpeg_file);//seg_size = read_word(jpeg_file);
                    jpeg->app1.header = read_n_bytes(jpeg->app1.header_size,jpeg_file);
                    jpeg->app1.metadata = read_n_bytes(jpeg->app1.size-8,jpeg_file);//metadata = read_n_bytes(seg_size-2,jpeg_file);   //Read metadata, -2 because the length contains already read 2 bytes of length

                    //TODO -8? bylo -7

                    add_to_order('a',jpeg);

                  printf(", size = %d\n",jpeg->app1.size);

                  printf("Header = %s\n", jpeg->app1.header);

                    if(!strcmp(jpeg->app1.header,"Exif\0\0"))      //Check for valid Exif header
                        parse_exif_md(jpeg->app1.metadata);
                    else
                        fprintf(stderr,"\nUnrecognized APP1 format\n");
                    break;

              case 0xDA:
                  printf("SOS = %ld\n",ftell(jpeg_file));

                  jpeg->sos.marker[0] = prev;
                  jpeg->sos.marker[1] = *c;
                  read_sos(jpeg_file,jpeg);
                  printf(", size = %ld\n",jpeg->sos.data_size+jpeg->sos.header_size);

                  add_to_order('s',jpeg);
                  add_to_order('e',jpeg);
                  printf("\n\nFound SOS marker\n\n");
                  goto loopend;


              case 0xD0:
              case 0xD1:
              case 0xD2:
              case 0xD3:
              case 0xD4:
              case 0xD5:
              case 0xD6:
              case 0xD7:
                printf("\n\nFound RST marker\n");
                break;

              case 0xC4:
                  printf("DHT = %ld\n",ftell(jpeg_file));

                  jpeg->dht.marker[0] = prev;
                  jpeg->dht.marker[1] = *c;
                  jpeg->dht.size = read_word(jpeg_file);
                  printf(", size = %d\n",jpeg->dht.size);

                  add_to_order('h',jpeg);

                  jpeg->dht.data = read_n_bytes(jpeg->dht.size-2,jpeg_file);
                  break;

              case 0xDB:
                  printf("DQT = %ld\n",ftell(jpeg_file));

                  jpeg->dqt.marker[0] = prev;
                  jpeg->dqt.marker[1] = *c;
                  jpeg->dqt.size = read_word(jpeg_file);
                  printf(", size = %d\n",jpeg->dqt.size);

                  add_to_order('q',jpeg);

                  jpeg->dqt.data = read_n_bytes(jpeg->dqt.size-2,jpeg_file);
                  break;

              default:
                  if(is_sof(*c))                      //Detect SOF marker
                  {
                      printf("SOF = %ld\n",ftell(jpeg_file));

                      jpeg->sof.marker[0] = prev;
                      jpeg->sof.marker[1] = *c;
                      read_sof(jpeg_file,jpeg);
                      printf(", size = %d\n",jpeg->sof.header_length);
                      add_to_order('f',jpeg);

                  }
                  else if(has_length(*c)) {                //If a segment that doesnt contain any useful metadata is encountered, jump through it
                      printf("ELSE = %ld\n",ftell(jpeg_file));

                      jpeg->unrecognized = (struct Unrecognized*) realloc(jpeg->unrecognized,
                                                                          (jpeg->unrecognized_count+1)*sizeof(struct Unrecognized));
                      jpeg->unrecognized_count += 1;

                      jpeg->unrecognized[jpeg->unrecognized_count-1].size = read_word(jpeg_file);
                      jpeg->unrecognized[jpeg->unrecognized_count-1].data = read_n_bytes(jpeg->unrecognized[jpeg->unrecognized_count-1].size-2,jpeg_file);
                      jpeg->unrecognized[jpeg->unrecognized_count-1].marker = (uint8_t*) malloc(2*sizeof(uint8_t));
                      jpeg->unrecognized[jpeg->unrecognized_count-1].marker[0] = prev;
                      jpeg->unrecognized[jpeg->unrecognized_count-1].marker[1] = *c;
                      printf(", size = %d\n",jpeg->unrecognized[jpeg->unrecognized_count-1].size);

                      add_to_order('u',jpeg);

                      //fseek(jpeg_file,seg_size-2,1);
                  }
                  break;
            };
        }

      prev = *c;
  }

  loopend:

    if(ftell(jpeg_file) < file_length) {
        printf("\nleftovers = %ld",ftell(jpeg_file));
        jpeg->leftovers_size = file_length - ftell(jpeg_file);
        jpeg->leftovers = (uint8_t*) malloc(sizeof(uint8_t)* jpeg->leftovers_size);
        fread(jpeg->leftovers,jpeg->leftovers_size,1,jpeg_file);
        printf("Leftovers size = %ld",jpeg->leftovers_size);
    }

  fclose(jpeg_file);
  free(c);

  return jpeg;
}
