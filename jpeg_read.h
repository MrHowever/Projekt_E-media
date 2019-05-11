#ifndef JPEG_READ_H
#define JPED_READ_H

#include <stdio.h>
#include <stdint.h>

//TODO czytanei i rekonstrukcja APP0 moze nie dzialac, sprawdzic
//unrecognized tez moze nie dzialac

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
    uint16_t soi;
    struct APP app0;
    struct APP app1;
    struct SOF sof;
    struct DHT dht;
    struct DQT dqt;
    struct SOS sos;
    uint8_t* eoi;
    struct Unrecognized* unrecognized;
    size_t unrecognized_count;
    uint8_t* leftovers;
    long leftovers_size;
    char* order;
    long order_size;
};

void init_jpeg(struct JPEG*);

struct JPEG* read_jpeg(const char*);
void read_sos(FILE*, struct JPEG*);
int copy_image(const char*);
void reconstruct_jpeg(struct JPEG*, const char*);

uint16_t intel_to_mot16(uint8_t*);
uint32_t intel_to_mot32(uint8_t*);
uint16_t read_mot16(uint8_t*);
uint32_t read_mot32(uint8_t*);
uint16_t read16(uint8_t*,int*,int);
uint32_t read32(uint8_t*,int*,int);

void print_header(const char*);
void read_sof(FILE*, struct JPEG*);
#endif
