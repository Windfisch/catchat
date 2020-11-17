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
#include "jpeglib.h"		/* Common decls for compressing and decompressing jpegs */
#include "jpegint.h"		/* Common decls for compressing and decompressing jpegs */

void manipulate_jpeg(const unsigned char* inbuffer, unsigned long insize, unsigned char** outbuffer, unsigned long* outsize)
{
  struct jpeg_decompress_struct inputinfo;
  struct jpeg_compress_struct outputinfo;
  struct jpeg_error_mgr jerr;
  jvirt_barray_ptr *coef_arrays;
  JDIMENSION i, compnum, rownum, blocknum;
  JBLOCKARRAY coef_buffers[MAX_COMPONENTS];
  JBLOCKARRAY row_ptrs[MAX_COMPONENTS];

  int changed = 0;

  /* Initialize the JPEG compression and decompression objects with default error handling. */
  inputinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&inputinfo);
  outputinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&outputinfo);

  /* Specify data source for decompression and recompression */
  jpeg_mem_src(&inputinfo, inbuffer, insize);
  jpeg_mem_dest(&outputinfo, outbuffer, outsize);

  /* Read file header */
  (void) jpeg_read_header(&inputinfo, TRUE);

  /* Allocate memory for reading out DCT coeffs */
  for (compnum=0; compnum<inputinfo.num_components; compnum++)
    coef_buffers[compnum] = ((&inputinfo)->mem->alloc_barray) 
                            ((j_common_ptr) &inputinfo, JPOOL_IMAGE,
                             inputinfo.comp_info[compnum].width_in_blocks,
                             inputinfo.comp_info[compnum].height_in_blocks);

  /* Read input file as DCT coeffs */
  coef_arrays = jpeg_read_coefficients(&inputinfo);

  /* Copy compression parameters from the input file to the output file */
  jpeg_copy_critical_parameters(&inputinfo, &outputinfo);

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

  /* Print out or modify DCT coefficients */
  for (compnum=0; compnum<num_components; compnum++)
  {
    printf("Component: %i\n", compnum);
    int counter[DCTSIZE2] = {0};
    for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
    {
      for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
      {
        //printf("\n\nComponent: %i, Row:%i, Column: %i\n", compnum, rownum, blocknum);
#define MIN_BITS 4
        for (i = DCTSIZE2; i-->0;)
        {
            if (coef_buffers[compnum][rownum][blocknum][i] & ~((1<<MIN_BITS)-1))
                break;
        }
        if (i>=0 && i < DCTSIZE2)
            counter[i]++;



        int ii = -1;
        for (i=0; i<DCTSIZE2; i++)
        {
            if (coef_buffers[compnum][rownum][blocknum][i] & ~3)
            {
                ii = i;
            }
        }
        //coef_buffers[compnum][rownum][blocknum][ii] = coef_buffers[compnum][rownum][blocknum][ii] & ~(1<<2);
        if (ii > 2)
		{
			coef_buffers[compnum][rownum][blocknum][ii] = coef_buffers[compnum][rownum][blocknum][ii] *2;
			changed++;
		}

        for (i=0; i<DCTSIZE2; i++)
        {
          //coef_buffers[compnum][rownum][blocknum][i] = coef_buffers[compnum][rownum][blocknum][i] /2;
          //coef_buffers[compnum][rownum][blocknum][i] = - coef_buffers[compnum][rownum][blocknum][i];
          //printf("%i,", coef_buffers[compnum][rownum][blocknum][i]);
        }
      }
    }
    int max = 0;
    for (i=0; i<DCTSIZE2; i++) {
        if (counter[i]>max)
            max=counter[i];
    }
    int j = 0;
    int next = 1;
    for (i=0; i<DCTSIZE2; i++) {
        printf("%3i: ", i);
        for (int j=0; j < counter[ inputinfo.natural_order[i]  ] * 100 / max; j++)
            printf("*");

        printf(" (%i)\n", counter[ inputinfo.natural_order[i] ]);
        
        if (++j >= next)
        {
            j = 0;
            next++;
            printf("\n");
        }
    }
  }
  printf("\n\n");

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
  jpeg_finish_decompress(&inputinfo);
  jpeg_destroy_decompress(&inputinfo);

  /* All done. */
  printf("changed %i\n", changed);
  return 0;			/* suppress no-return-value warnings */
}
