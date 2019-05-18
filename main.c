#include "encryption.h"
#include "jfif.h"
#include "exif.h"
#include "dct.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define FILENAME_BUFFER_SIZE 256
#define USER_BUFFER_SIZE 256


//Main function contains the GUI control
int main(int argc, char** argv)
{
    int image_count = argc-1;
    struct public_key pubk;
    struct private_key privk;
    int selected_jpeg = 0;
    int chosen_option = 0;
    struct JPEG** jpeg_arr = (struct JPEG**) malloc(sizeof(struct JPEG*)*image_count);
    char** file_names = (char**) malloc(sizeof(char*)*image_count);

    //Allocate memory for JPEG filenames
    for(int i = 0; i < image_count; i++) {
        file_names[i] = (char*) malloc(sizeof(char)*FILENAME_BUFFER_SIZE);
    }

    //Read the JPEG files from provided filenames
    for(int i = 1; i < argc; i++) {
        strcpy(file_names[i-1],argv[i]);
        jpeg_arr[i-1] = read_jpeg(file_names[i-1]);
    }

    //Generate RSA encryption keys
    generate_keys(&pubk,&privk);

    //Main program loop
    while(1) {
        char input_line[USER_BUFFER_SIZE];
        jpeg_selection:
        fflush(NULL);
        system("clear");
        printf("Please select JPEG image (-1 to exit, 0 to add new JPEG):\n");

        for (int i = 0; i < image_count; i++) {
            printf("%d) %s\n", i + 1, file_names[i]);
        }

        fgets(input_line,USER_BUFFER_SIZE,stdin);
        sscanf(input_line," %d", &selected_jpeg);
        selected_jpeg -= 1;     //Images in the GUI are indexed from 1 and array is indexed from 0

        if(selected_jpeg == -2)
            goto quit_program;

        //Add new JPEG file
        if(selected_jpeg == -1) {
            //TODO fix jpeg_arr realloc
            char new_filename[FILENAME_BUFFER_SIZE];
            image_count++;
            jpeg_arr = (struct JPEG**) realloc(jpeg_arr,sizeof(struct JPEG*)*image_count);
            file_names = (char**) realloc(file_names,sizeof(char*)*image_count);
            file_names[image_count-1] = (char*) malloc(sizeof(char)*FILENAME_BUFFER_SIZE);

            system("clear");
            printf("Please provide new JPEG file name:\n");

            fgets(input_line,USER_BUFFER_SIZE,stdin);
            sscanf(input_line,"%s",new_filename);
            printf("Added image: %s\n",new_filename);
            strcpy(file_names[image_count-1],new_filename);
            jpeg_arr[image_count-1] = read_jpeg(file_names[image_count-1]);

            goto jpeg_selection;
        }


        while (1) {
            system("clear");
            printf("1) Print EXIF metadata\n");
            printf("2) Print JFIF metadata\n");
            printf("3) Print SOF metadata\n");
            printf("4) Print all metadata\n");
            printf("5) Encrypt image\n");
            printf("6) Decrypt image\n");
            printf("7) Save copy as\n");
            printf("8) View image\n");
            printf("9) Select different image\n");
            printf("10) Display a DCT of file fragment\n");
            printf("11) Display a 2D DCT of file fragment\n");
            printf("12) Quit\n");


            fgets(input_line,USER_BUFFER_SIZE,stdin);
            sscanf(input_line," %d", &chosen_option);

            system("clear");
            fflush(NULL);

            switch (chosen_option) {
                case 1:
                    //Find an APP1 marker
                    for(int i = 0; i < jpeg_arr[selected_jpeg]->app_count; i++) {
                        if (!strcmp(jpeg_arr[selected_jpeg]->app[i].header, "Exif\0\0")) {
                            parse_exif_md(jpeg_arr[selected_jpeg]->app[i].metadata, stdout);
                        } else
                            fprintf(stdout, "No EXIF metadata found\n");
                    }
                    getchar();
                    break;
                case 2:
                    //Find an APP0 marker
                    for(int i = 0; i < jpeg_arr[selected_jpeg]->app_count; i++) {
                        if (!strcmp(jpeg_arr[selected_jpeg]->app[i].header, "JFIF\0")) {
                            print_jfif(read_jfif(jpeg_arr[selected_jpeg]->app[i].metadata), stdout);
                        } else
                            fprintf(stdout, "No JFIF metadata found\n");
                    }
                    getchar();
                    break;
                case 3:
                    read_sof_metadata(stdout,&jpeg_arr[selected_jpeg]->sof);
                    getchar();
                    break;
                case 4:
                    system("clear");
                    read_metadata(jpeg_arr[selected_jpeg],stdout);
                    getchar();
                    break;
                case 5:
                    encrypt_jpeg(jpeg_arr[selected_jpeg],&pubk);
                    break;
                case 6:
                    decrypt_jpeg(jpeg_arr[selected_jpeg],&privk);
                    break;
                case 7: {
                    char new_filename[256];
                    printf("Please provide a new name for the file copy:\n");
                    fgets(input_line,FILENAME_BUFFER_SIZE,stdin);
                    sscanf(input_line,"%s",new_filename);
                    reconstruct_jpeg(jpeg_arr[selected_jpeg],new_filename);
                }
                break;
                case 8: {
                    char sys_call[256] = "display ";
                    strcat(sys_call,file_names[selected_jpeg]);
                    strcat(sys_call," &");
                    system(sys_call);
                }
                break;
                case 9:
                    goto jpeg_selection;
                case 10: {
                    display_dct(dct(jpeg_arr[selected_jpeg]->sos.image_data,1000),jpeg_arr[selected_jpeg]->sos.image_data,1000);
                }
                    break;
                case 11:
                    display_dct2D_comp(jpeg_arr[selected_jpeg],8);
                break;
                case 12:
                    goto quit_program;
            }
        }

    }

    //Clean-up
    quit_program:

    for(int i = 0; i < image_count; i++) {
        destroy_jpeg(jpeg_arr[i]);
        free(jpeg_arr[i]);
        free(file_names[i]);
    }

    free(jpeg_arr);
    free(file_names);
    system("rm fragment_dct.temp-resized.jpg");
    system("rm image_fragment.temp-resized.jpg");
    system("clear");
    return 0;
}
