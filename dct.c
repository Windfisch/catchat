#define MIN_BITS 4
/*
 * dct.c
 *
 * Copyright (C) 2012, Owen Campbell-Moore.
 * Changed by Florian Jung (2020)
 *
 * This program allows you to take JPEG, modify it's DCT coefficients and
 * then output the new coefficients to a new JPEG.
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"        /* Common decls for compressing and decompressing jpegs */
#include "jpegint.h"        /* Common decls for compressing and decompressing jpegs */


JCOEF* carrier_coef(JCOEF* block)
{
    int i;
    for (i = DCTSIZE2; i-->0;)
    {
        if (block[i] & ~((1<<MIN_BITS)-1))
            break;
    }
    if (i>=0 && i < DCTSIZE2)
    {
        return &block[i];
    }
    else
    {
        return NULL;
    }
}

void manipulate_jpeg(const unsigned char* inbuffer, unsigned long insize, unsigned char** outbuffer, unsigned long* outsize, const char* text, long textsize)
{
  struct jpeg_decompress_struct inputinfo;
  struct jpeg_error_mgr jerr;
  jvirt_barray_ptr *coef_arrays;
  JBLOCKARRAY coef_buffers[MAX_COMPONENTS];
  JBLOCKARRAY row_ptrs[MAX_COMPONENTS];

  /* Initialize the JPEG compression and decompression objects with default error handling. */
  inputinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&inputinfo);

  /* Specify data source for decompression and recompression */
  jpeg_mem_src(&inputinfo, inbuffer, insize);

  /* Read file header */
  (void) jpeg_read_header(&inputinfo, TRUE);

  /* Allocate memory for reading out DCT coeffs */
  for (JDIMENSION compnum=0; compnum<inputinfo.num_components; compnum++)
    coef_buffers[compnum] = ((&inputinfo)->mem->alloc_barray) 
                            ((j_common_ptr) &inputinfo, JPOOL_IMAGE,
                             inputinfo.comp_info[compnum].width_in_blocks,
                             inputinfo.comp_info[compnum].height_in_blocks);

  /* Read input file as DCT coeffs */
  coef_arrays = jpeg_read_coefficients(&inputinfo);

  JDIMENSION i, compnum, rownum, blocknum;

  /* Copy DCT coeffs to a new array */
  int num_components = inputinfo.num_components;
  size_t block_row_size[num_components];
  int width_in_blocks[num_components];
  int height_in_blocks[num_components];
  for (compnum=0; compnum<num_components; compnum++)
  {
    height_in_blocks[compnum] = inputinfo.comp_info[compnum].height_in_blocks;
    width_in_blocks[compnum] = inputinfo.comp_info[compnum].width_in_blocks;
    block_row_size[compnum] = (size_t) sizeof(JCOEF)*DCTSIZE2*width_in_blocks[compnum];
    for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
    {
      row_ptrs[compnum] = ((&inputinfo)->mem->access_virt_barray) 
                          ((j_common_ptr) &inputinfo, coef_arrays[compnum], 
                            rownum, (JDIMENSION) 1, FALSE);
      for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
      {
        for (i=0; i<DCTSIZE2; i++)
        {
          coef_buffers[compnum][rownum][blocknum][i] = row_ptrs[compnum][0][blocknum][i];
        }
      }
    }
  }

  if (!text) // decode
  {
      int bits = 0;
      for (compnum=0; compnum<num_components; compnum++) {
        for (rownum=0; rownum<height_in_blocks[compnum]; rownum++) {
          for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
          {
            JCOEF* coef = carrier_coef(coef_buffers[compnum][rownum][blocknum]);
            if (coef)
                bits++;
          }
        }
      }
      int bytes = bits / 8;
	  printf("%i possible bit locations", bits);
      unsigned char* result = malloc(bytes);
      int result_pos = 0;
      bits = 0;
      unsigned char byte = 0;
      for (compnum=0; compnum<num_components; compnum++) {
        for (rownum=0; rownum<height_in_blocks[compnum]; rownum++) {
          for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
          {
            JCOEF* coef = carrier_coef(coef_buffers[compnum][rownum][blocknum]);
            if (coef)
            {
                int lsb = (*coef) & 1;
                byte = (byte << 1) + lsb;
                if (++bits == 8) {
                    result[result_pos++] = byte;
                    byte = 0;
                    bits = 0;
                }
            }
          }
        }
      }

      *outbuffer = result;
      *outsize = bytes;
  }
  else // encode
  {
  int counter = 0;
      struct jpeg_compress_struct outputinfo;
      outputinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&outputinfo);
      jpeg_mem_dest(&outputinfo, outbuffer, outsize);
      /* Copy compression parameters from the input file to the output file */
      jpeg_copy_critical_parameters(&inputinfo, &outputinfo);

      int input_bytepos = 0;
      int input_bitmask = 0x80;
      /* modify DCT coefficients */
      for (compnum=0; compnum<num_components; compnum++) {
        printf("Component: %i\n", compnum);
        for (rownum=0; rownum<height_in_blocks[compnum]; rownum++) {
          for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
          {
                JCOEF* coef = carrier_coef(coef_buffers[compnum][rownum][blocknum]);
                if (coef)
                {
                    counter++;
                    if (input_bytepos < textsize)
                    {
                        int bit = (text[input_bytepos] & input_bitmask) != 0;
                        input_bitmask >>= 1;
                        if (input_bitmask == 0)
                        {
                            input_bitmask = 0x80;
                            input_bytepos++;
                        }

                        *coef &= ~1;
                        if (bit)
                            *coef |= 1;
                    }
                }
          }
        }
      }

        printf("wrote to %i possible bit locations", counter);

      /* Output the new DCT coeffs to a JPEG file */
      for (compnum=0; compnum<num_components; compnum++)
      {
        for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
        {
          row_ptrs[compnum] = ((&outputinfo)->mem->access_virt_barray) 
                              ((j_common_ptr) &outputinfo, coef_arrays[compnum], 
                               rownum, (JDIMENSION) 1, TRUE);
          memcpy(row_ptrs[compnum][0][0], 
                 coef_buffers[compnum][rownum][0],
                 block_row_size[compnum]);
        }
      }

      /* Write to the output file */
      jpeg_write_coefficients(&outputinfo, coef_arrays);

      /* Finish compression and release memory */
      jpeg_finish_compress(&outputinfo);
      jpeg_destroy_compress(&outputinfo);
  }
  jpeg_finish_decompress(&inputinfo);
  jpeg_destroy_decompress(&inputinfo);

  /* All done. */
}
