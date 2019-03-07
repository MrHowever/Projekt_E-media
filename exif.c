#include "exif.h"
#include <string.h>
#include <stdio.h>
#include "file_read_func.h"

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

char* val_t(uint16_t data_format)
{
  switch(data_format)
  {
    case 1:
      return "Unsigned byte";
    case 2:
      return "ASCII string";
    case 3:
      return "Unsigned short";
    case 4:
      return "Unsigned long";
    case 5:
      return "Unsigned rational";
    case 6:
      return "Signed byte";
    case 7:
      return "Undefined";
    case 8:
      return "Signed short";
    case 9:
      return "Signed long";
    case 10:
      return "Signed rational";
    case 11:
      return "Single float";
    case 12:
      return "Double float";
    default:
      return "Unknown";
  }
}

uint32_t data_size(uint16_t data_format)
{
  switch(data_format)
  {
  case 1:
  case 2:
  case 6:
  case 7:
    return 1;
    break;

  case 3:
  case 8:
    return 2;
    break;

  case 4:
  case 9:
  case 11:
    return 4;
    break;
    
  case 5:
  case 10:
  case 12:
    return 8;
    break;

  default:
    printf("Unknown data format\n");
    return 0;
    break;
  }
}

int check_alignment(const char* alignment)
{
  if(!strcmp(alignment,"II"))
   return 0;
  else if(!strcmp(alignment,"MM"))
    return 1;
  else
  {
    printf("\nCorrupted byte allign information\n");
    return -1;
  }  
}

void parse_exif_md(uint8_t* metadata)
{
  const char byte_align[3] = {metadata[6],metadata[7],'\0'};
  int align;
  int offset = 0;
  uint32_t IFDoffset = 0;
  uint32_t subifd_offset = 0;
  uint32_t ifd1_offset = 0;
  struct Map dict;
  
  map_init(&dict);
  fill_exif_map(&dict);

  align = check_alignment(byte_align);
  
  offset = 8;

  if(read16(metadata,&offset,align) != 0x002A)    //Check for correct TAG mark
  {
    printf("\nCorrupted TAG mark\n");
    return;
  }

  IFDoffset = read32(metadata,&offset,align);    //Offset to first IFD (IFD0)

  //Move to IFD0
  offset = IFDoffset + EXIF_BASE_OFFSET;                        //6 byte offset comes from "Exif\0\0"
  
  ifd1_offset = read_ifd(metadata,offset,align,&dict, &subifd_offset);
  
  if(subifd_offset >= 8)
    read_ifd(metadata, subifd_offset + EXIF_BASE_OFFSET,align,&dict, &subifd_offset);

  if(ifd1_offset >= 8)
    read_ifd(metadata, ifd1_offset,align,&dict, &subifd_offset);
}

uint32_t read_ifd(uint8_t* metadata, int offset, int align, struct Map* dict, uint32_t* sub_ifd_offset)
{
  uint16_t directory_entries = 0;
  uint16_t tag = 0, data_format = 0;
  uint32_t components_num = 0;
  uint32_t next_ifd_offset = 0;
  int get_val = 0;
  uint32_t temp_offset = 0;

  directory_entries = read16(metadata,&offset,align);  //Read the number of directory entries

  printf("\n%-4s\t\t%-20s\t\t%s\t\t%s\t\t%s\n\n","Tag","Description","Value type","Components","Value");
  
  //Read IFD
  for(uint16_t i = 0; i < directory_entries; i++)             //-1 Because last entry is an offset to next IFD
  {
    tag = read16(metadata,&offset,align);
    data_format = read16(metadata,&offset,align);
    components_num = read32(metadata,&offset,align);
    
    printf("0x%04X\t\t%-25s\t%-20s\t%d\t\t\t",tag,map_find(dict,tag), val_t(data_format),components_num);
    
    if(tag == 0x8769)
      get_val = 1;
    
    temp_offset = print_exif_data(data_format, components_num, metadata, offset,align,get_val);
    
    if(get_val)
      *sub_ifd_offset = temp_offset;
    
    offset += 4;    //Exif data or its' offset is contained in 4 bytes
    
    get_val = 0;
  }

  next_ifd_offset = read32(metadata,&offset,align) + EXIF_BASE_OFFSET;
  
  return next_ifd_offset;
}

uint32_t print_exif_data(uint16_t format, uint32_t comp_num, uint8_t* metadata, int offset, int align, int get_val)
{
  uint32_t data_length = 0;
  uint32_t subifd_offset = 0;
  uint32_t tmp_val = 0;
  
  if(format == 7)           //Its undefined what does undefined data contain so just print whatever
  {
    tmp_val = read32(metadata,&offset,align);
    printf("%d\n",tmp_val);
    
    if(get_val)
      return tmp_val;
      
    return 0;
  }
  
  data_length = data_size(format) * comp_num;

  if(data_length > 4)
    offset = read32(metadata, &offset,align) + EXIF_BASE_OFFSET;

  if(get_val)        //Case for returning SubIFD offset
  {
    subifd_offset = read32(metadata, &offset,align);
    printf("%lu\n", (long unsigned int) subifd_offset);
    return subifd_offset;  
  }
  
  for(int i = 0; i < comp_num; i++)
  {
    switch(format)
    {
    case 1:       //Unsigned byte
      printf("%c ", metadata[offset+i]);
      break;
    case 2:       //ASCII string
      printf("%c",  metadata[offset+i]);
      break;
    case 3:       //Unsigned short
      printf("%hu ",  read16(metadata, &offset, align));
      break;
    case 4:       //Unsigned long
      printf("%lu ", (long unsigned int) read32(metadata,&offset,align));
      break;
    case 5:       //Unsigned rational (unsigned long divided by unsigned long)
      printf("%f ", ((double) read32(metadata,&offset,align))/((double) read32(metadata,&offset,align)));
      break;
    case 6:       //Signed byte
      printf("%c ",  metadata[offset+i]);
      break;
    case 7:       //Undefined
      break;
    case 8:       //Signed short
      printf("%hd ", read16(metadata,&offset,align));
      break;
    case 9:       //Signed long
      printf("%ld ", (long unsigned int) read32(metadata,&offset,align));
      break;
    case 10:      //Signed rational
      printf("%f ", ((double) read32(metadata,&offset,align))/((double) read32(metadata,&offset,align)));
      break;
    case 11:      //Single float
      printf("%f ", (float) read32(metadata,&offset,align));
      break;
    case 12:      //Double float
      printf("%f %f ", (float) read32(metadata,&offset,align), (float) read32(metadata,&offset,align));
      break;
    default:
      printf("\nUndefined data format\n");
      break;
    }
  }

  printf("\n");
  return 0;
}
