//
// Created by mrhowever on 28.03.19.
//

#include "dct.h"
#include <math.h>
#include <stdlib.h>

uint8_t* dct(uint8_t* data, int height, int width)
{
    int size = height*width;
    int pixels = size/3;

    uint8_t* transformed = (uint8_t*) malloc(size*sizeof(uint8_t));

    for(int i = 0; i < 3; i++) {        //Each component has its own DCT transform
        for(int k = i; k < size; k += 3) {  //For every output component val
            for(int n = i; n < size; n += 3) {  //For every input component val
              //  transformed[k] += data[n]*cos((M_PI/pixels)*k*(n+0.5));
            }
        }
    }

    return transformed;
}

