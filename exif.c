#include "exif.h"
#include <string.h>
#include <stdio.h>
#include "file_read_func.h"
#include "jpeg_read.h"

#define EXIF_BASE_OFFSET 6                                        //Skips the "Exif\0\0" string to correctly calculate offset

void fill_exif_map(struct Map* map)
{
  map_push(map,0x0001,"InteropIndex");
  map_push(map,0x0100,"Image width");
  map_push(map,0x0101,"Image height");
  map_push(map,0x0102,"Bits per sample");
  map_push(map,0x0103,"Compression");
  map_push(map,0x0106,"Photometric Interpretation");
  map_push(map,0x010e,"Image description");
  map_push(map,0x010f,"Make");
  map_push(map,0x0110,"Model");
  map_push(map,0x0111,"Strip offsets");
  map_push(map,0x0112,"Orientation");
  map_push(map,0x0115,"Samples per pixel");
  map_push(map,0x0116,"Rows per strip");
  map_push(map,0x0117,"Strip byte counts");
  map_push(map,0x011a,"X resolution");
  map_push(map,0x011b,"Y resolution");
  map_push(map,0x011c,"Planar configuration");
  map_push(map,0x0128,"Resolution unit");
  map_push(map,0x012d,"Transfer function");
  map_push(map,0x0131,"Software");
  map_push(map,0x0132,"Modify date");
  map_push(map,0x013b,"Artist");
  map_push(map,0x013f,"Primary chromaticities");
  map_push(map,0x0201,"Thumbnail offset");
  map_push(map,0x0202,"Thumbnail length");
  map_push(map,0x0211,"YCbCrCoefficients");
  map_push(map,0x0212,"YCbCrSubSampling");
  map_push(map,0x0213,"YCbCrPositioning");
  map_push(map,0x0214,"ReferenceBlackWhite");
  map_push(map,0x8298,"Copyright");
  map_push(map,0x829a,"Exposure time");
  map_push(map,0x829d,"FNumber");
  map_push(map,0x8769,"Exif offset");
  map_push(map,0x8822,"Exposure program");
  map_push(map,0x8824,"Spectral sensitivity");
  map_push(map,0x8825,"GPS info");
  map_push(map,0x8827,"ISO");
  map_push(map,0x8828,"Opto-electrict Convolution Factor");
  map_push(map,0x8830,"Sensitivity type");
  map_push(map,0x8831,"Standard output sensitivity");
  map_push(map,0x8832,"Recommended exposure index");
  map_push(map,0x8833,"ISO speed");
  map_push(map,0x8834,"ISO speed latitude yyy");
  map_push(map,0x8835,"ISO speed latitude zzz");
  map_push(map,0x9000,"Exif version");
  map_push(map,0x9003,"Date time original");
  map_push(map,0x9004,"Create date");
  map_push(map,0x9010,"Offset time");
  map_push(map,0x9011,"Offset time original");
  map_push(map,0x9012,"Offset time digitized");
  map_push(map,0x9101,"Components configuration");
  map_push(map,0x9102,"Compressed bits per pixel");
  map_push(map,0x9201,"Shutter speed value");
  map_push(map,0x9202,"Aperture value");
  map_push(map,0x9203,"Brightness value");
  map_push(map,0x9204,"Exposure compensation");
  map_push(map,0x9205,"Map aperture value");
  map_push(map,0x9206,"Subject distance");
  map_push(map,0x9207,"Metering mode");
  map_push(map,0x9208,"Light source");
  map_push(map,0x9209,"Flash");
  map_push(map,0x920a,"Focal length");
  map_push(map,0x9214,"Subject area");
  map_push(map,0x9286,"User comment");
  map_push(map,0x9290,"SubSecTime");
  map_push(map,0x9291,"SubSecTimeOriginal");
  map_push(map,0x9292,"SubSecTimeDigitized");
  map_push(map,0x9400,"Ambient temperature");
  map_push(map,0x9401,"Humidity");
  map_push(map,0x9402,"Pressure");
  map_push(map,0x9403,"Water depth");
  map_push(map,0x9404,"Acceleration");
  map_push(map,0x9405,"Camera elevation angle");
  map_push(map,0xa000,"Flashpix version");
  map_push(map,0xa001,"Color space");
  map_push(map,0xa002,"Exif image width");
  map_push(map,0xa003,"Exif image height");
  map_push(map,0xa004,"Related sound file");
  map_push(map,0xa005,"InteropOffset");
  map_push(map,0xa20b,"Flash energy");
  map_push(map,0xa20c,"Spatial frequency response");
  map_push(map,0xa20e,"Focal plane X resolution");
  map_push(map,0xa20f,"Focal plane Y resolution");
  map_push(map,0xa210,"Focal plane resolution unit");
  map_push(map,0xa214,"Subject location");
  map_push(map,0xa215,"Exposure index");
  map_push(map,0xa217,"Sensing method");
  map_push(map,0xa300,"File source");
  map_push(map,0xa301,"Scene type");
  map_push(map,0xa302,"CFA pattern");
  map_push(map,0xa401,"Custom rendered");
  map_push(map,0xa402,"Exposure mode");
  map_push(map,0xa403,"White balance");
  map_push(map,0xa404,"Digital zoom ratio");
  map_push(map,0xa405,"Focal length in 35mm format");
  map_push(map,0xa406,"Scene capture type");
  map_push(map,0xa407,"Gain control");
  map_push(map,0xa408,"Contrast");
  map_push(map,0xa409,"Saturation");
  map_push(map,0xa40a,"Sharpness");
  map_push(map,0xa40b,"Device setting description");
  map_push(map,0xa40c,"Subject distance range");
  map_push(map,0xa420,"Image unique ID");
  map_push(map,0xa430,"Owner name");
  map_push(map,0xa431,"Serial number");
  map_push(map,0xa432,"Lens info");
  map_push(map,0xa433,"LensMake");
  map_push(map,0xa434,"Lens model");
  map_push(map,0xa435,"Lens serial number");
}

void fill_gps_map(struct Map* map)
{
  map_push(map,0x0000,"GPSVersion ID");
  map_push(map,0x0001,"GPS latitude reference");
  map_push(map,0x0002,"GPS latitude");
  map_push(map,0x0003,"GPS longitude reference");
  map_push(map,0x0004,"GPS longitude");
  map_push(map,0x0005,"GPS altitude reference");
  map_push(map,0x0006,"GPS altitude");
  map_push(map,0x0007,"GPS time stamp");
  map_push(map,0x0008,"GPS satellites");
  map_push(map,0x0009,"GPS status");
  map_push(map,0x000A,"GPS measure mode");
  map_push(map,0x000B,"GPSDOP");
  map_push(map,0x000C,"GPS Speed reference");
  map_push(map,0x000D,"GPS speed");
  map_push(map,0x000E,"GPS track reference");
  map_push(map,0x000F,"GPS track");
  map_push(map,0x0010,"GPS image direction reference");
  map_push(map,0x0011,"GPS image direction");
  map_push(map,0x0012,"GPSMapDatum");
  map_push(map,0x0013,"GPS destination latitude ref.");
  map_push(map,0x0014,"GPS destination latitude");
  map_push(map,0x0015,"GPS destination longitude ref.");
  map_push(map,0x0016,"GPS destination longitude");
  map_push(map,0x0017,"GPS destination bearing ref.");
  map_push(map,0x0018,"GPS destination bearing");
  map_push(map,0x0019,"GPS destination distance ref.");
  map_push(map,0x001A,"GPS destination distance");
  map_push(map,0x001B,"GPS processing method");
  map_push(map,0x001C,"GPS area information");
  map_push(map,0x001D,"GPS date stamp");
  map_push(map,0x001E,"GPS differential");
  map_push(map,0x001F,"GPS positioning error");
}

void fill_format_map(struct Map* map)
{
  map_push(map,1,"Unsigned byte");
  map_push(map,2,"ASCII string");
  map_push(map,3,"Unsigned short");
  map_push(map,4,"Unsigned long");
  map_push(map,5,"Unsigned rational");
  map_push(map,6,"Signed byte");
  map_push(map,7,"Undefined");
  map_push(map,8,"Signed short");
  map_push(map,9,"Signed long");
  map_push(map,10,"Signed rational");
  map_push(map,11,"Single float");
  map_push(map,12,"Double float");
}

//Returns the string describing the data type of dictionary entry
char* val_t(uint16_t data_format, struct Map* map)
{
    return map_find(map,data_format);
}

//Returns the size of data entry in bytes
uint32_t data_size(uint16_t data_format,FILE* output)
{
  switch(data_format)
  {
      case 1:
      case 2:
      case 6:
      case 7:
        return 1;

      case 3:
      case 8:
        return 2;

      case 4:
      case 9:
      case 11:
        return 4;

      case 5:
      case 10:
      case 12:
        return 8;

      default:
        fprintf(output,"Unknown data format\n");
        return 0;
  }
}

//Returns 0 if the data is written in Intel alignment and 1 if it's written
//in Motorola alignment
int check_alignment(const char* alignment, FILE* output)
{
  if(!strcmp(alignment,"II"))
   return 0;
  else if(!strcmp(alignment,"MM"))
    return 1;
  else {
    fprintf(output,"\nCorrupted byte allign information\n");
    return -1;
  }  
}


void parse_exif_md(uint8_t* metadata, FILE* output)
{
  //First 3 bytes contain information about byte alignment of the data
  const char byte_align[3] = {metadata[6],metadata[7],'\0'};
  int align;
  int offset = 8;
  struct Map dict, gps_dict;
  struct ifd_offsets offsets;

  //Initialize the maps
  map_init(&dict);
  map_init(&gps_dict);
  offsets_init(&offsets);
  fill_exif_map(&dict);
  fill_gps_map(&gps_dict);

  //Check data byte alignment
  align = check_alignment(byte_align,output);

  //Check for correct TAG marker
  if(read16(metadata,&offset,align) != 0x002A) {
    fprintf(output,"\nCorrupted TAG mark\n");
    return;
  }

  //Read offset to IFD0
  offset = read32(metadata,&offset,align) + EXIF_BASE_OFFSET;                        //6 byte offset comes from "Exif\0\0"

  //Print all the found IFD's
  print_header("Exif Metadata",output);
  read_ifd(metadata,offset,align,&dict,&offsets,output);

  if(offsets.subifd_offset >= 8) {
    print_header("SubIFD Metadata",output);
    read_ifd(metadata, offsets.subifd_offset + EXIF_BASE_OFFSET,align,&dict, &offsets,output);
  }

  if(offsets.ifd1_offset >= 8) {
    print_header("IFD1 Metadata",output);
    read_ifd(metadata, offsets.ifd1_offset + EXIF_BASE_OFFSET,align,&dict, &offsets,output);
  }

  if(offsets.gps_offset >= 8) {
    print_header("GPS Metadata",output);
    read_ifd(metadata, offsets.gps_offset + EXIF_BASE_OFFSET,align,&gps_dict,&offsets,output);
  }

  map_destroy(&dict);
  map_destroy(&gps_dict);
}


//Read IFD segment
void read_ifd(uint8_t* metadata, int offset, int align, struct Map* dict, struct ifd_offsets* offsets,FILE* output)
{
  uint16_t directory_entries = 0;
  uint16_t tag = 0, data_format = 0;
  uint32_t components_num = 0;
  struct Map type_map;

  map_init(&type_map);
  fill_format_map(&type_map);

  //Read the number of directory entries
  directory_entries = read16(metadata,&offset,align);

  //Print header
  fprintf(output,"\n%-4s\t\t%-20s\t\t%s\t\t%s\t\t%s\n\n","Tag","Description","Value type","Components","Value");
  
  //Read IFD
  for(uint16_t i = 0; i < directory_entries; i++) {            //-1 Because last entry is an offset to next IFD
    tag = read16(metadata,&offset,align);
    data_format = read16(metadata,&offset,align);
    components_num = read32(metadata,&offset,align);
    
    fprintf(output,"0x%04X\t\t%-25s\t%-20s\t%d\t\t\t",tag,map_find(dict,tag), val_t(data_format,&type_map),components_num);
    print_exif_data(data_format, components_num, metadata,offset,align,output);

    //Tags 0x8769 and 0x8825 contain information about offsets to SubIFD and GPS IFD
    if(tag == 0x8769)
      offsets->subifd_offset = read32(metadata,&offset,align);
    else if(tag == 0x8825)
      offsets->gps_offset = read32(metadata,&offset,align);
    else
      offset += 4;    //Exif data or its' offset is contained in 4 bytes    
  }

  //Read offset to IFD1 segment
  offsets->ifd1_offset = read32(metadata,&offset,align);
}

void print_exif_data(uint16_t format, uint32_t comp_num, uint8_t* metadata, int offset, int align,FILE* output)
{
  uint32_t data_length  = data_size(format,output) * comp_num;
  double nom,denom,result;

  //If data is longer than 4 bytes it is an offset to actual data position
  if(data_length > 4)
    offset = read32(metadata, &offset,align) + EXIF_BASE_OFFSET;

  for(int i = 0; i < comp_num; i++)
  {
    switch(format)
    {
        case 1:       //Unsigned byte
          fprintf(output,"%u ", metadata[offset+i]);
          break;
        case 2:       //ASCII string
        case 6:       //Signed byte
          fprintf(output,"%c ",  metadata[offset+i]);
          break;
        case 3:       //Unsigned short
          fprintf(output,"%hu ",  read16(metadata, &offset, align));
          break;
        case 4:       //Unsigned long
          fprintf(output,"%lu ", (long unsigned int) read32(metadata,&offset,align));
          break;
        case 5:       //Unsigned rational (unsigned long divided by unsigned long)
          nom = read32(metadata,&offset,align);
          denom = read32(metadata,&offset,align);
          result = denom == 0 ? 0 : nom/denom;
          fprintf(output,"%f ",result);
          break;
        case 7:       //Undefined
          break;
        case 8:       //Signed short
          fprintf(output,"%hd ", read16(metadata,&offset,align));
          break;
        case 9:       //Signed long
          fprintf(output,"%ld ", (long unsigned int) read32(metadata,&offset,align));
          break;
        case 10:      //Signed rational
          fprintf(output,"%f ", ((double) read32(metadata,&offset,align))/((double) read32(metadata,&offset,align)));
          break;
        case 11:      //Single float
          fprintf(output,"%f ", (float) read32(metadata,&offset,align));
          break;
        case 12:      //Double float
          fprintf(output,"%f %f ", (float) read32(metadata,&offset,align), (float) read32(metadata,&offset,align));
          break;
        default:
          fprintf(output,"\nUndefined data format\n");
          break;
    }
  }

  fprintf(output,"\n");
}

void offsets_init(struct ifd_offsets* offsets)
{
  offsets->gps_offset = 0;
  offsets->ifd1_offset = 0;
  offsets->subifd_offset = 0;
}