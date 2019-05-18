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

//Print metadata info header to output
void print_header(const char* text, FILE* output)
{
  fprintf(output,"\n\n");
  
  for(int i = 0; i < 140; i++)
    fprintf(output,"/");

  fprintf(output,"\n");
  
  for(int i = 0; i < 60; i++)
    fprintf(output," ");

  fprintf(output,"%s\n",text);


  for(int i = 0; i < 140; i++)
    fprintf(output,"/");

  fprintf(output,"\n\n");
}

//TODO is it neccessary?
//Function that returns true if detected marker has any lenght that follows
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

//Function that transforms a uint8_t array of size 8 into 64 bit value
uint64_t to64bit(uint8_t* input)
{
    uint64_t output = 0;

    for(int i = 0; i < 8; i++) {
        output |= ((uint64_t) input[i]) << (56 - (i * 8));
    }

    return output;
}

//Function that transforms 32 bit value stored inside uint8_t array into
//uint64_t value
uint64_t to64bit32bit(uint8_t* input)
{
    uint64_t output = 0;

    for(int i = 0; i < 4; i++) {
        output |= ((uint64_t) input[i]) << (24 - (i * 8));
    }

    return output;
}

//Function that transforms uint64_t value into uint8_t array
uint8_t* to8bit(uint64_t input)
{
   uint8_t* output = (uint8_t*) malloc(sizeof(uint8_t)*4);

   for(int i = 0; i < 4; i++) {
       output[i] = (uint8_t) ((input >> (24 - (i*8))) & 255);
    }

   return output;
}

//Function that transforms uint16_t value into uint8_t array
uint8_t* to8bitfrom16(uint16_t input)
{
    uint8_t* output = (uint8_t*) malloc(sizeof(uint8_t)*2);

    output[0] = (uint8_t) ((input >> 8) & 255);
    output[1] = (uint8_t) (input & 255);

    return output;
}

//Function that initializes JPEG struct
void init_jpeg(struct JPEG* jpeg)
{
    jpeg->unrecognized_count = 0;
    jpeg->app_count = 0;
    jpeg->dht_count = 0;
    jpeg->dqt_count = 0;

    jpeg->app = NULL;
    jpeg->dht = NULL;
    jpeg->dqt = NULL;
    jpeg->unrecognized = NULL;

    jpeg->eoi = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->sos.image_data = 0;
    jpeg->sos.data_size = 0;

    jpeg->order = (char*) malloc(sizeof(char)*256);
    jpeg->order_size = 0;

    jpeg->sof.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->sos.marker = (uint8_t*) malloc(2*sizeof(uint8_t));
}

//Function that destroys JPEG struct
void destroy_jpeg(struct JPEG* jpeg)
{
    free(jpeg->eoi);
    free(jpeg->leftovers);
    free(jpeg->order);

    for(int i = 0; i < jpeg->unrecognized_count; i++) {
        free(jpeg->unrecognized[i].marker);
        free(jpeg->unrecognized[i].data);
    }
    free(jpeg->unrecognized);

    for(int i = 0; i < jpeg->app_count; i++) {
        free(jpeg->app[i].marker);
        free(jpeg->app[i].header);
        free(jpeg->app[i].metadata);
    }
    free(jpeg->app);

    for(int i = 0; i < jpeg->dht_count; i++) {
        free(jpeg->dht[i].marker);
        free(jpeg->dht[i].data);
    }
    free(jpeg->dht);

    for(int i = 0; i < jpeg->dqt_count; i++) {
        free(jpeg->dqt[i].marker);
        free(jpeg->dqt[i].data);
    }
    free(jpeg->dqt);

    free(jpeg->sof.marker);
    free(jpeg->sof.components);

    free(jpeg->sos.marker);
    free(jpeg->sos.header);
    free(jpeg->sos.image_data);
}

//Print APP segment of JPEG to file
void print_app(FILE* file, struct APP* app)
{
    if(app) {
        printf("App segment size reconstructed: %d\n",app->size);
        fwrite(app->marker, 2, 1, file);
        fwrite(to8bitfrom16(app->size), 2, 1, file);
        fwrite(app->metadata, app->size -2, 1, file);
    }
}

//Print SOF segment of JPEG to file
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

//Print SOS segment of JPEG to file
void print_sos(FILE* file, struct SOS* sos)
{
    if(sos) {
        fwrite(sos->marker, 2, 1, file);
        fwrite(to8bitfrom16(sos->header_size),2,1,file);
        fwrite(sos->header, sos->header_size-2, 1, file);
        fwrite(sos->image_data, sos->data_size, 1, file);
    }
}

//Print unrecognized segments of JPEG to file
void print_unrecognized(FILE* file, struct Unrecognized* unrecognized)
{
    if(unrecognized) {
        fwrite(unrecognized->marker, 2, 1, file);
        fwrite(to8bitfrom16(unrecognized->size), 2, 1, file);
        fwrite(unrecognized->data, unrecognized->size-2, 1, file);
    }
}

//Print DHT segment of JPEG to file
void print_dht(FILE* file, struct DHT* dht)
{
    if(dht) {
        fwrite(dht->marker, 2, 1, file);
        fwrite(to8bitfrom16(dht->size), 2, 1, file);
        fwrite(dht->data, dht->size-2, 1, file);
    }
}

//Print DQT segment of JPEG to file
void print_dqt(FILE* file, struct DQT* dqt)
{
    if(dqt) {
        fwrite(dqt->marker, 2, 1, file);
        fwrite(to8bitfrom16(dqt->size), 2, 1, file);
        fwrite(dqt->data, dqt->size-2, 1, file);
    }
}

//Print SOI marker of JPEG to file
void print_soi(FILE* file, uint16_t soi)
{
    if(soi) {
        fwrite(to8bitfrom16(soi), 2, 1, file);
    }
}

//Print EOI marker of JPEG to file
void print_eoi(FILE* file, uint8_t* eoi)
{
    if(eoi) {
        fwrite(eoi, 2, 1, file);
    }
}

//Function that reconstructs an JPEG file from JPEG struct
void reconstruct_jpeg(struct JPEG* jpeg, const char* filename)
{
    FILE* output_file = fopen(filename,"wb");
    long app_idx = 0, dht_idx = 0,dqt_idx = 0, unrecognized_idx = 0;

    //Iterate through order array which dictates order of segments
    //inside the JPEG file
    for(int i = 0; i < jpeg->order_size; i++) {
        switch(jpeg->order[i]) {
            case 'a':
                print_app(output_file,&jpeg->app[app_idx]);
                app_idx++;
                break;

            case 'h':
                print_dht(output_file,&jpeg->dht[dht_idx]);
                dht_idx++;
                break;

            case 'q':
                printf("\n\nPrinting DQT\n\n");
                print_dqt(output_file,&jpeg->dqt[dqt_idx]);
                dqt_idx++;
                break;

            case 'f':
                print_sof(output_file,&jpeg->sof);
                break;

            case 's':
                print_sos(output_file,&jpeg->sos);
                break;

            case 'e':
                print_eoi(output_file,jpeg->eoi);
                break;

            case 'u':
                print_unrecognized(output_file,&jpeg->unrecognized[unrecognized_idx]);
                unrecognized_idx++;
                break;

            case 'i':
                print_soi(output_file,jpeg->soi);
                break;

            default:
                break;
        }
    }
}

//Read SOS segment of JPEG file
void read_sos(FILE* jpeg_file, struct JPEG* jpeg)
{
    uint8_t* c = (uint8_t*) malloc(sizeof(uint8_t));
    uint8_t prev = 0;
    long eoi_pos = 0, data_begin_pos = 0;

    //Read header of the SOS segment
    jpeg->sos.header_size = read_word(jpeg_file);
    jpeg->sos.header = read_n_bytes(jpeg->sos.header_size-2,jpeg_file);

    //Save the offset to the image data
    data_begin_pos = ftell(jpeg_file);

    //Move through the image data until the EOI (0xFFD9) marker is found
    while(1)
    {
        fread(c, 1, 1, jpeg_file);

        if (prev == 0xFF && *c == 0xD9) {
            jpeg->eoi[0] = prev;
            jpeg->eoi[1] = *c;

            //Save the offset to the EOI marker
            eoi_pos = ftell(jpeg_file) - 1;

            break;
        }

        prev = *c;
    }

    //Data size is the difference between offsets to the data and EOI marker
    jpeg->sos.data_size = eoi_pos - data_begin_pos-1;

    //Move back to the beginning of the image data
    fseek(jpeg_file,data_begin_pos,SEEK_SET);

    //Read the image data into the struct
    jpeg->sos.image_data = (uint8_t*) malloc(jpeg->sos.data_size*sizeof(uint8_t));
    fread(jpeg->sos.image_data,jpeg->sos.data_size,1,jpeg_file);

    free(c);
}

//Function that adds new segment to the order
void add_to_order(char id,struct JPEG* jpeg)
{
    jpeg->order[jpeg->order_size] = id;
    jpeg->order_size++;
}

//Function that parses the APP segment of JPEG file and creates a new APP struct
void add_new_app(FILE* jpeg_file, struct JPEG* jpeg, uint8_t marker)
{
    //Resize the APP array holding all the APP segments inside the JPEG struct
    jpeg->app = (struct APP*) realloc(jpeg->app, (jpeg->app_count+1)*sizeof(struct APP));
    
    //Read APP segment data
    jpeg->app[jpeg->app_count].marker = (uint8_t*) malloc(sizeof(uint8_t)*2);
    jpeg->app[jpeg->app_count].marker[0] = 0xFF;
    jpeg->app[jpeg->app_count].marker[1] = marker;
    jpeg->app[jpeg->app_count].size = read_word(jpeg_file);//seg_size = read_word(jpeg_file);
    jpeg->app[jpeg->app_count].metadata = read_n_bytes(jpeg->app[jpeg->app_count].size-2,jpeg_file);//metadata = read_n_bytes(seg_size-7,jpeg_file);

    //Add to file order
    add_to_order('a',jpeg);

    //Case for APP0 segmennt
    if(marker == 0xE0) {
        jpeg->app[jpeg->app_count].header_size = 5;
        jpeg->app[jpeg->app_count].header = (uint8_t*) malloc(sizeof(uint8_t)*jpeg->app[jpeg->app_count].header_size);
        memcpy(jpeg->app[jpeg->app_count].header,jpeg->app[jpeg->app_count].metadata,jpeg->app[jpeg->app_count].header_size);
    }   //Case for APP1 segment
    else if(marker == 0xE1) {
        jpeg->app[jpeg->app_count].header_size = 6;
        jpeg->app[jpeg->app_count].header = (uint8_t*) malloc(sizeof(uint8_t)*jpeg->app[jpeg->app_count].header_size);
        memcpy(jpeg->app[jpeg->app_count].header,jpeg->app[jpeg->app_count].metadata,jpeg->app[jpeg->app_count].header_size);
    }   //Case for the rest of the APPn segments
    else {
        jpeg->app[jpeg->app_count].header_size = 0;
        jpeg->app[jpeg->app_count].header = NULL;
    }

    jpeg->app_count++;
}

//Master function that calls all the metadata reading functions
void read_metadata(struct JPEG* jpeg, FILE* output)
{
    for(int i = 0; i < jpeg->app_count; i++) {
        read_app_metadata(output,&jpeg->app[i]);
    }

    read_sof_metadata(output,&jpeg->sof);
}

//Function that reads metadata from an APPn segment
void read_app_metadata(FILE* output, struct APP* app)
{
    if(app->marker[1] == 0xE0) {
        if (app->header != NULL && !strcmp(app->header, "JFIF\0"))
            print_jfif(read_jfif(app->metadata),output);
        else if (!strcmp(app->header, "JFXX\0"))
            fprintf(output, "\nUnrecognized APP0 format:\n");
    } else if(app->marker[1] = 0xE1) {
        if (app->header != NULL && !strcmp(app->header, "Exif\0\0"))      //Check for valid Exif header
            parse_exif_md(app->metadata,output);
        else
            fprintf(output, "\nUnrecognized APP1 format:\n");
    }
}

//Function that adds new DHT segment to JPEG struct
void add_new_dht(FILE* jpeg_file, struct JPEG* jpeg, uint8_t marker)
{
    //Resize the array holding the DHT segments
    jpeg->dht = (struct DHT*) realloc(jpeg->dht, (jpeg->dht_count+1)*sizeof(struct DHT));

    //Read the header data
    jpeg->dht[jpeg->dht_count].marker = (uint8_t*) malloc(sizeof(uint8_t)*2);
    jpeg->dht[jpeg->dht_count].marker[0] = 0xFF;
    jpeg->dht[jpeg->dht_count].marker[1] = marker;
    jpeg->dht[jpeg->dht_count].size = read_word(jpeg_file);

    //Add to order
    add_to_order('h',jpeg);

    //Read the segment data
    jpeg->dht[jpeg->dht_count].data = read_n_bytes(jpeg->dht[jpeg->dht_count].size-2,jpeg_file);

    jpeg->dht_count++;
}

//Function that adds new DQT segment to JPEG struct
void add_new_dqt(FILE* jpeg_file, struct JPEG* jpeg, uint8_t marker)
{
    //Resize the array holding the DQT segments
    jpeg->dqt = (struct DQT*) realloc(jpeg->dqt, (jpeg->dqt_count+1)*sizeof(struct DQT));

    //Read the header data
    jpeg->dqt[jpeg->dqt_count].marker = (uint8_t*) malloc(sizeof(uint8_t)*2);
    jpeg->dqt[jpeg->dqt_count].marker[0] = 0xFF;
    jpeg->dqt[jpeg->dqt_count].marker[1] = marker;
    jpeg->dqt[jpeg->dqt_count].size = read_word(jpeg_file);

    //Add to order
    add_to_order('q',jpeg);

    //Read the segment data
    jpeg->dqt[jpeg->dqt_count].data = read_n_bytes(jpeg->dqt[jpeg->dqt_count].size-2,jpeg_file);

    jpeg->dqt_count++;
}

//Function that creates the SOF segment from file data
void read_sof(FILE* jpeg_file, struct JPEG* jpeg, uint8_t marker)
{
    jpeg->sof.marker[0] = 0xFF;
    jpeg->sof.marker[1] = marker;
    jpeg->sof.header_length = read_word(jpeg_file);
    jpeg->sof.sample_precision = read_byte(jpeg_file);

    //Row and columns count
    jpeg->sof.rows = read_word(jpeg_file);
    jpeg->sof.cols = read_word(jpeg_file);

    //Component count
    jpeg->sof.comp_count = read_byte(jpeg_file);
    jpeg->sof.components = (struct Components*) malloc(jpeg->sof.comp_count*sizeof(struct Components));

    //Read the data for every component
    for(int i = 0; i < jpeg->sof.comp_count; i++)
    {
        jpeg->sof.components[i].id = read_byte(jpeg_file);
        jpeg->sof.components[i].sampling = read_byte(jpeg_file);//comp_sampling = read_byte(jpeg_file);
        jpeg->sof.components[i].dest_selector = read_byte(jpeg_file);
    }

    //Add to order
    add_to_order('f',jpeg);
}

//Function that prints the metadata from SOF marker
void read_sof_metadata(FILE* output, struct SOF* sof)
{
    print_header("SOF Marker Metadata",output);
    fprintf(output, "Image size: %d x %d\nColor depth (Bit depth): %d\n",
           sof->rows,
           sof->cols,
           sof->comp_count);

    for(int i = 0; i < sof->comp_count; i++) {
        fprintf(output,"Sampling factor of component %d:\n\tHorizontal: %d\n\tVertical: %d\n",
               sof->components[i].id,
               sof->components[i].sampling >> 4,
               sof->components[i].sampling & 0x0F);
    }
}

//Function that adds new unrecognized segment
//Unrecognized segment usually means it doesn't contain any metadata that this program
//is designed to handle or the marker isn't usually present in an JPEG file
void add_new_unrecognized(FILE* jpeg_file, struct JPEG* jpeg, uint8_t marker)
{
    //Resize the array holding all the unrecognized segments
    jpeg->unrecognized = (struct Unrecognized*) realloc(jpeg->unrecognized,
                                                        (jpeg->unrecognized_count+1)*sizeof(struct Unrecognized));
    jpeg->unrecognized_count += 1;

    //Read the segment data
    jpeg->unrecognized[jpeg->unrecognized_count-1].size = read_word(jpeg_file);
    jpeg->unrecognized[jpeg->unrecognized_count-1].data = read_n_bytes(jpeg->unrecognized[jpeg->unrecognized_count-1].size-2,jpeg_file);
    jpeg->unrecognized[jpeg->unrecognized_count-1].marker = (uint8_t*) malloc(2*sizeof(uint8_t));
    jpeg->unrecognized[jpeg->unrecognized_count-1].marker[0] = 0xFF;
    jpeg->unrecognized[jpeg->unrecognized_count-1].marker[1] = marker;

    //Add to order
    add_to_order('u',jpeg);
}

//Function that parses the JPEG file into JPEG struct
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

    //Check if memory allocation succeeded
    if(c == NULL) {
        fprintf(stderr,"Memory error. Unable to allocate neccessary memory.\n");
        return 0;
    }

    //Check if file could be opened
    if(!jpeg_file) {
        fprintf(stderr,"Error. Unable to open file\n");
        return 0;
    }

    //Check JPG magic number
    if((jpeg->soi = read_word(jpeg_file)) != 0xFFD8)
    {
        fprintf(stderr,"This is not an JPEG image!\n");
        return 0;
    }

    //Add SOI marker to order
    add_to_order('i',jpeg);

    while(1)
    {
        //Read consecutive bytes to find markers
        fread(c,1,1,jpeg_file);

        if( prev == 0xFF ) {                      //0xFF byte signals that next byte might be a marker
            switch(*c)
            {
                //APPn markers
                    case 0xE0:                      //JFIF metadata marker
                    case 0xE1:                      //EXIF metadata marker
                    case 0xE2:
                    case 0xE3:
                    case 0xE4:
                    case 0xE5:
                    case 0xE6:
                    case 0xE7:
                    case 0xE8:
                    case 0xE9:
                    case 0xEA:
                    case 0xEB:
                    case 0xEC:
                    case 0xED:
                    case 0xEE:
                    case 0xEF:
                      add_new_app(jpeg_file,jpeg,*c);
                        break;

                        //SOS marker
                  case 0xDA:
                      jpeg->sos.marker[0] = prev;
                      jpeg->sos.marker[1] = *c;
                      read_sos(jpeg_file,jpeg);

                      //Add SOS and EOI markers to order
                      add_to_order('s',jpeg);
                      add_to_order('e',jpeg);
                      goto loopend;

                      //RST markers
                  case 0xD0:
                  case 0xD1:
                  case 0xD2:
                  case 0xD3:
                  case 0xD4:
                  case 0xD5:
                  case 0xD6:
                  case 0xD7:
                    break;

                        //DHT markers
                  case 0xC4:
                      add_new_dht(jpeg_file,jpeg,*c);
                      break;

                        //DQT markers
                  case 0xDB:
                      add_new_dqt(jpeg_file,jpeg,*c);
                      break;

                      //SOF markers
                  case 0xC0:
                  case 0xC1:
                  case 0xC2:
                  case 0xC3:
                  case 0xC5:
                  case 0xC6:
                  case 0xC7:
                  case 0xC9:
                  case 0xCA:
                  case 0xCB:
                  case 0xCD:
                  case 0xCE:
                  case 0xCF:
                        read_sof(jpeg_file,jpeg,*c);
                        break;

                  default:
                      if(has_length(*c)) {
                          add_new_unrecognized(jpeg_file,jpeg,*c);
                      } else{
                          fprintf(stderr,"ERROR: Unrecognized marker, parsing failed.");
                      }
                      break;
                };
            }

          prev = *c;
      }

  loopend:

    //If there's any data after the EOI marker it's saved
    if(ftell(jpeg_file) < file_length) {
        jpeg->leftovers_size = file_length - ftell(jpeg_file);
        jpeg->leftovers = (uint8_t*) malloc(sizeof(uint8_t)* jpeg->leftovers_size);
        fread(jpeg->leftovers,jpeg->leftovers_size,1,jpeg_file);
    }


    //Clean-up
    fclose(jpeg_file);
    free(c);

    return jpeg;
}

//Function that transforms uint64_t value into uint8_t array
uint8_t* touint8arrfrom64(uint64_t value)
{
    uint8_t* arr = (uint8_t*) malloc(sizeof(uint8_t)*8);

    for(int i = 0; i < 8; i++) {
        arr[i] = (uint8_t) ( (value >> (56 - (i*8))) & 255);
    }

    return arr;
}

//Function that encrypts the image data of JPEG
void encrypt_jpeg(struct JPEG* jpeg, struct public_key* pubk)
{
    uint8_t value32bit[4];
    uint64_t value64bit = 0;
    uint8_t* temp32bit = 0;
    uint8_t* encrypted_data = (uint8_t*) malloc(sizeof(uint8_t)*jpeg->sos.data_size*2);
    uint64_t idx = 0, i = 0;
    uint64_t output_size = 0;
    int progress = 0;

    //Remove byte stuffing from image data
    jpeg->sos.data_size = remove_byte_stuffing(&jpeg->sos,jpeg->sos.data_size);

    //Iterate through the image data encrypting 32-bit data fragments
    for(i = 0; (i + 4) < jpeg->sos.data_size; i += 4) {

       //Get 32-bit value as array of 8-bit values
        for(int j = 0; j < 4; j++) {
            value32bit[j] = jpeg->sos.image_data[i+j];
        }

        if( i > progress*(jpeg->sos.data_size/100)) {
            fprintf(stdout,"Encryption progress: %d%%\n",progress);
            progress += 5;
        }

        //Transform 32-bit number from 8-bit array to 64-bit value
        value64bit = to64bit32bit(value32bit);
        //Encrypt the 64-bit value
        value64bit = encrypt(value64bit,pubk);
        //Transform the 64-bit value back to 8-bit array
        temp32bit = touint8arrfrom64(value64bit);

        //Write the encrypted value to temporary array
        for(int j = 0; j < 8; j++, idx++) {
            encrypted_data[idx] = temp32bit[j];
        }

        output_size += 8;

        free(temp32bit);
    }

    //Write the remaining data to temporary array
    for(; i < jpeg->sos.data_size; i++, idx++) {
        encrypted_data[idx] = jpeg->sos.image_data[i];
        output_size += 1;
    }

    //Update data size
    jpeg->sos.data_size = output_size;

    //Move the encrypted data from temporary array to the struct
    free(jpeg->sos.image_data);
    jpeg->sos.image_data = (uint8_t*) malloc(jpeg->sos.data_size*sizeof(uint8_t));
    memcpy(jpeg->sos.image_data,encrypted_data,jpeg->sos.data_size);
    free(encrypted_data);

    //Add byte stuffing to the data
    jpeg->sos.data_size = add_byte_stuffing(&jpeg->sos,jpeg->sos.data_size);
}

void decrypt_jpeg(struct JPEG* jpeg,struct private_key* privk)
{
    uint8_t value32bit[8];
    uint64_t value64bit = 0;
    uint8_t* temp32bit = 0;
    uint8_t* decrypted_data = (uint8_t*) malloc(sizeof(uint8_t)*jpeg->sos.data_size);
    uint64_t idx = 0, i = 0;
    uint64_t output_size = 0;
    int progress = 0;

    //Remove byte stuffing from image data
    jpeg->sos.data_size = remove_byte_stuffing(&jpeg->sos,jpeg->sos.data_size);

    //Iterate through the image data decrypting 64-bit data fragments
    for(i = 0; (i + 8) < jpeg->sos.data_size; i += 8) {

        //Get 64-bit value as array of 8-bit values
        for(int j = 0; j < 8; j++) {
            value32bit[j] = jpeg->sos.image_data[i+j];
        }

        if( i > progress*(jpeg->sos.data_size/100)) {
            fprintf(stdout,"Decryption progress: %d%%\n",progress);
            progress += 5;
        }

        //Transform 64-bit number from 8-bit array to 64-bit value
        value64bit = to64bit(value32bit);
        //Decrypt the 64-bit value
        value64bit = decrypt(value64bit,privk);
        //Transform the 64-bit value back to 8-bit array as an 32-bit number
        temp32bit = to8bit(value64bit);

        //Write the decrypted value to temporary array
        for(int j = 0; j < 4; j++,idx++) {
            decrypted_data[idx] = temp32bit[j];
        }

        output_size += 4;
        free(temp32bit);
    }

    //Write the remaining data to temporary array
    for(; i < jpeg->sos.data_size; i++, idx++) {
        decrypted_data[idx] = jpeg->sos.image_data[i];
        output_size += 1;
    }

    //Move the decrypted data from temporary array to the struct
    free(jpeg->sos.image_data);
    jpeg->sos.data_size = output_size;
    jpeg->sos.image_data = (uint8_t*) malloc(sizeof(uint8_t)*jpeg->sos.data_size);
    memcpy(jpeg->sos.image_data,decrypted_data,jpeg->sos.data_size);
    free(decrypted_data);

    //Add byte stuffing to the image data
    jpeg->sos.data_size = add_byte_stuffing(&jpeg->sos,jpeg->sos.data_size);
}

//Function that removes byte stuffing from provided image data
long remove_byte_stuffing(struct SOS* sos, long length)
{
    uint8_t* temp_storage = 0;
    long stuff_count = 0;

    //Count the instances of 0xFF marker
    for(int i = 0; i < length; i++) {
        if(sos->image_data[i] == 0xFF)
            stuff_count++;
    }

    //Allocate the needed memory for smaller array
    temp_storage = (uint8_t*) malloc(sizeof(uint8_t)*(length-stuff_count));

    //Iterate through the data, copy it to the temporary array and if an
    //0xFF marker is encountered, skip the following 0x00 byte
    for(int i = 0, j = 0; i < length; i++, j++) {
        temp_storage[j] = sos->image_data[i];
        if(sos->image_data[i] == 0xFF)
            i++;
    }

    //Move the altered image data to struct
    free(sos->image_data);
    sos->image_data = (uint8_t*) malloc(sizeof(uint8_t)*(length-stuff_count));
    memcpy(sos->image_data,temp_storage,length-stuff_count);
    free(temp_storage);

    return length-stuff_count;
}

long add_byte_stuffing(struct SOS* sos, long length)
{
    uint8_t* temp_storage = 0;
    long stuff_count = 0;

    //Count the instances of 0xFF marker
    for(int i = 0; i < sos->data_size; i++) {
        if(sos->image_data[i] == 0xFF)
            stuff_count++;
    }

    //Allocate the needed memory for bigger array
    temp_storage = (uint8_t*) malloc(sizeof(uint8_t)*(length+stuff_count));

    //Iterate through the data, copy it to the temporary array and if an
    //0xFF marker is encountered, add a following 0x00 byte
    for(int i = 0, j = 0; i < length; i++, j++) {
        temp_storage[j] = sos->image_data[i];
        if(sos->image_data[i] == 0xFF) {
            temp_storage[j+1] = 0x00;
            j++;
        }
    }

    //Move the altered image data to struct
    free(sos->image_data);
    sos->image_data = (uint8_t*) malloc(sizeof(uint8_t)*(length+stuff_count));
    memcpy(sos->image_data,temp_storage,length+stuff_count);
    free(temp_storage);

    return length+stuff_count;
}