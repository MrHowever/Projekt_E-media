//
// Created by mrhowever on 28.03.19.
//

#ifndef PROJEKT_E_MEDIA_DCT_H
#define PROJEKT_E_MEDIA_DCT_H
#include<stdint.h>
#include <jpeg_read.h>

double* dct(uint8_t*,long);
int** dct2D(uint8_t**,int);

void display_dct(double*,uint8_t*,long);
void display_dct2D_comp(struct JPEG*,int);

uint8_t** prepare_image_fragment(struct JPEG*,int);
void int_data_to_image(int**,int,const char*);
void uint_data_to_image(uint8_t**,int,const char*);
void create_script();


#endif //PROJEKT_E_MEDIA_DCT_H
