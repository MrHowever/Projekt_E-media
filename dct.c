#include "dct.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

double* dct(uint8_t* data, long length)
{
    double* output = (double*) malloc(length*sizeof(double));

    //Set all bytes to 0
    memset(output,0,length*sizeof(double));

    //Calculate 1D DCT
    for(int k = 0; k < length; k++) {
        for(int n = 0; n < length; n++) {
            output[k] += (data[n]*cos((M_PI/length)*(n+0.5)*k));
        }

        output[k] *= sqrt(2./length)*sqrt(.5);
    }

    return output;
}

int** dct2D(uint8_t** data, int size)
{
    FILE* file = fopen("file","w");
    double** output_double = (double**) malloc(sizeof(double*)*size);
    int** output_int = (int**) malloc(sizeof(int*)*size);
    for(int i = 0; i < size; i++) {
        output_double[i] = (double*) malloc(sizeof(double)*size);
        memset(output_double[i],0,sizeof(double)*size);     //Set all bytes to 0

        output_int[i] = (int*) malloc(sizeof(int)*size);
        memset(output_int[i],0,sizeof(int)*size);           //Set all bytes to 0
    }

    //Quantization matrix commonly used in JPG compression
    int quant_matrix[8][8] = {{16,11,10,16,24,40,51,61},
                              {12,12,14,19,26,58,60,55},
                              {14,13,16,24,40,57,69,56},
                              {14,17,22,29,51,87,80,62},
                              {18,22,37,56,68,109,103,77},
                              {24,35,55,64,81,104,113,92},
                              {49,64,78,87,103,121,120,101},
                              {72,92,95,98,112,100,103,99}};

    //Rescale the data for 0 mean value
    for(int u = 0; u < size; u++) {
        for(int v = 0; v < size; v++) {
            output_int[u][v] = data[u][v] - 128;
        }
    }

    //Calculate the 2D DCT on values of type double
    for(int u = 0; u < size; u++) {
        for(int v = 0; v < size; v++) {
            for(int i = 0; i < size; i++) {
                for(int j = 0; j < size; j++) {
                    output_double[u][v] += output_int[i][j]*cos((M_PI/size)*(i+.5)*u)*cos((M_PI/size)*(j+.5)*v);
                }
            }
            output_double[u][v] *= sqrt(2./size)*sqrt(2./size)*sqrt(.5)*sqrt(.5);
        }
    }

    //Apply quantization matrix and round to the nearest integer value
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            output_int[i][j] = round(output_double[i][j]/quant_matrix[i][j]);
        }
    }

    for(int i = 0; i < size; i++) {
        free(output_double[i]);
    }
    free(output_double);

    return output_int;
}

void uint_data_to_image(uint8_t** data, int size, const char* filename)
{
    char sys_call[512];
    FILE* file = fopen(filename,"wb");

    //Write data to file
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            fwrite(&data[i][j], 1, 1, file);
        }
    }

    fflush(file);

    //Create an .jpg file from previously written data
    sprintf(sys_call,"convert -depth 8 -size %dx%d+0 gray:%s %s.jpg",
            size,size,filename,filename);
    system(sys_call);

    //Rescale the data for easier comparison
    sprintf(sys_call,"convert %s.jpg -scale %dx%d %s-resized.jpg",
            filename,size*100,size*100, filename);
    system(sys_call);

    fclose(file);
}

void int_data_to_image(int** data, int size, const char* filename)
{
    char sys_call[512];
    FILE* file = fopen(filename,"wb");

    //Write data to file
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            fwrite(&data[i][j], sizeof(int), 1, file);
        }
    }

    fflush(file);

    //Create an .jpg file from previously written data
    sprintf(sys_call,"convert -depth 32 -size %dx%d+0 gray:%s %s.jpg",
            size,size,filename,filename);
    system(sys_call);

    //Rescale the data for easier comparison
    sprintf(sys_call,"convert %s.jpg -scale %dx%d %s-resized.jpg",
            filename, size*100,size*100,filename);
    system(sys_call);

    fclose(file);
}

uint8_t** prepare_image_fragment(struct JPEG* jpeg, int size)
{
    uint8_t** output = (uint8_t**) malloc(sizeof(uint8_t*)*size);
    for(int i = 0; i < size; i++) {
        output[i] = (uint8_t*) malloc(sizeof(uint8_t)*size);
    }

    for(int i = 0, j = 0, k = 0; i < size*size; i++, k++) {
        output[k][j] = jpeg->sos.image_data[i];

        if(k == (size -1)) {
            j++;
            k=0;
        }
    }

    return output;
}

void create_script()
{
    FILE* script = fopen("script.gp","w");
    fprintf(script,"set style line 1 \\\n"
                   "    linecolor rgb '#0060ad' \\\n"
                   "    linetype 1 linewidth 2 \\\n"
                   "    pointtype 7 pointsize 1.5\n"
                   "\n"
                   "set multiplot layout 2, 1 title \"Transformacja DCT\"\n"
                   "\n"
                   "set xlabel 'N'\n"
                   "set ylabel 'Value'\n"
                   "\n"
                   "set xrange [0:1000]\n"
                   "set yrange[0:256]\n"
                   "set pointsize 0.5\n"
                   "set title 'Original data'\n"
                   "\n"
                   "\n"
                   "plot 'orig_data.dat' title 'Original data' with impulses, 'orig_data.dat' with points pt 7 notitle\n"
                   "\n"
                   "set title 'DCT'\n"
                   "set yrange [-200:200]\n"
                   "\n"
                   "plot 'dct_data.dat' using 1:2 title 'DCT' with impulses, 'dct_data.dat' with points pt 7 notitle");
    fclose(script);
}

void display_dct(double* dct_data, uint8_t* orig_data, long length)
{
    FILE* dct_output = fopen("dct_data.dat","w");
    FILE* orig_output = fopen("orig_data.dat","w");
    FILE* script;

    //Check if GNUPlot script exists, create it if necessary
    if(!(script = fopen("script.gp","r")))
        create_script();
    else
        fclose(script);

    //Write data to files
    for(long i = 0; i < length; i++) {
        fprintf(dct_output,"%ld %f\n",i,dct_data[i]);
        fprintf(orig_output,"%ld %d\n",i,orig_data[i]);
    }

    fflush(dct_output);
    fflush(orig_output);

    //Call GNUPlot
    system("gnuplot -persistent script.gp");

    //Cleanup
    fclose(dct_output);
    fclose(orig_output);
    system("rm dct_data.dat");
    system("rm orig_data.dat");
    free(dct_data);
}

void display_dct2D_comp(struct JPEG* jpeg, int fragment_size)
{
    const char* original_file = "image_fragment.temp";
    const char* dct_file = "fragment_dct.temp";
    char sys_call[256] = "\0";

    //Prepare data
    uint8_t** image_fragment = prepare_image_fragment(jpeg,fragment_size);
    int** dct = dct2D(image_fragment,fragment_size);

    //Write data to files
    int_data_to_image(dct,fragment_size,dct_file);
    uint_data_to_image(image_fragment,fragment_size,original_file);

    //System calls to display the images for comparison
    sprintf(sys_call,"display -title \"Original image\" %s-resized.jpg &",
            original_file);
    system(sys_call);

    sprintf(sys_call,"display -title \"DCT image\" %s-resized.jpg &",
            dct_file);
    system(sys_call);

    //Cleanup
    for(int i = 0; i < fragment_size; i++) {
        free(image_fragment[i]);
        free(dct[i]);
    }

    system("rm image_fragment.temp");
    system("rm image_fragment.temp.jpg");
    system("rm fragment_dct.temp");
    system("rm fragment_dct.temp.jpg");
    free(image_fragment);
    free(dct);
}
