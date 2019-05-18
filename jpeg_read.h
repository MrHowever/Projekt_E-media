#ifndef JPEG_READ_H
#define JPED_READ_H

#include <stdio.h>
#include <stdint.h>
#include "encryption.h"

//TODO czytanei i rekonstrukcja APP0 moze nie dzialac, sprawdzic
//TODO remove unrecognized?

struct APP   //APPn segments
{
    uint8_t* marker;
    uint16_t size;
    uint8_t* header;
    long header_size;
    uint8_t* metadata;
};

struct Components
{
    uint8_t id;
    uint8_t sampling;
    uint8_t dest_selector;
};

struct SOF
{
    uint8_t* marker;
    uint16_t header_length;
    uint8_t sample_precision;
    uint16_t rows;
    uint16_t cols;
    uint8_t comp_count;
    struct Components* components;
};

struct SOS
{
    uint8_t* marker;
    uint8_t* header;
    uint16_t header_size;
    uint8_t* image_data;
    long data_size;
};

struct Unrecognized
{
    uint8_t* marker;
    uint16_t size;
    uint8_t* data;
};

struct DHT
{
    uint8_t* marker;
    uint16_t size;
    uint8_t* data;
};

struct DQT
{
    uint8_t* marker;
    uint16_t size;
    uint8_t* data;
};

struct JPEG
{
    //JPEG structure
    uint16_t soi;
    struct APP* app;
    struct SOF sof;
    struct DHT* dht;
    struct DQT* dqt;
    struct SOS sos;
    struct Unrecognized* unrecognized;
    uint8_t* eoi;

    //Helper size variables
    size_t unrecognized_count;
    size_t app_count;
    size_t dht_count;
    size_t dqt_count;

    //Everything that follows EOI marker
    uint8_t* leftovers;
    long leftovers_size;

    //Reconstruction order
    char* order;
    long order_size;
};


//Main function parsing JPEG file to JPEG data structure
struct JPEG* read_jpeg(const char*);

//Function used to rebuild JPEG file from JPEG data structure
void reconstruct_jpeg(struct JPEG*, const char*);

//Data and memory management functions for JPEG structure
void init_jpeg(struct JPEG*);
void destroy_jpeg(struct JPEG*);

//Functions used to parse JPEG data to specific substructures
void add_new_unrecognized(FILE*,struct JPEG*,uint8_t);
void add_new_dqt(FILE*,struct JPEG*,uint8_t);
void add_new_dht(FILE*,struct JPEG*,uint8_t);
void add_new_app(FILE*,struct JPEG*,uint8_t);
void read_sos(FILE*, struct JPEG*);
void read_sof(FILE*,struct JPEG*,uint8_t);
void add_to_order(char,struct JPEG*);

//Functions used to print JPEG structure to file when reconstructing
void print_soi(FILE*,uint16_t);
void print_eoi(FILE*,uint8_t*);
void print_dht(FILE*,struct DHT*);
void print_dqt(FILE*,struct DQT*);
void print_unrecognized(FILE*,struct Unrecognized*);
void print_sos(FILE*,struct SOS*);
void print_sof(FILE*,struct SOF*);
void print_app(FILE*,struct APP*);

//Helper functions used to read and transform bytes of data
uint16_t intel_to_mot16(uint8_t*);
uint32_t intel_to_mot32(uint8_t*);
uint16_t read_mot16(uint8_t*);
uint32_t read_mot32(uint8_t*);
uint16_t read16(uint8_t*,int*,int);
uint32_t read32(uint8_t*,int*,int);
uint8_t* to8bitfrom16(uint16_t);
uint8_t* to8bit(uint64_t);
//uint64_t* to64bit(uint8_t*,long);
uint64_t to64bit(uint8_t*);

//Encryption/decryption functions
void encrypt_jpeg(struct JPEG*,struct public_key*);
void decrypt_jpeg(struct JPEG*,struct private_key*);

//Functions used to print metadata stored in JPEG structure
void print_header(const char*,FILE*);
void read_metadata(struct JPEG*,FILE*);
void read_sof_metadata(FILE*, struct SOF*);
void read_app_metadata(FILE*,struct APP*);

//Miscellaneous helper functions
int has_length(uint8_t);

//Add/remove byte stuffing to image segment of jpeg
long remove_byte_stuffing(struct SOS*, long);
long add_byte_stuffing(struct SOS*, long);

uint8_t* touint8arrfrom64(uint64_t);
uint64_t to64bit32bit(uint8_t*);
#endif
