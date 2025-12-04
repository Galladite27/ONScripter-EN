/* -*- C++ -*-
 * 
 *  conv_shared.cpp - Shared code of sarconv, nsaconv and ns2conv
 *
 *  Copyright (c) 2001-2006 Ogapee. All rights reserved.
 *  (of conv_shared.cpp from onscripter source) 
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2009-2010 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Modified by Uncle Mion (UncleMion@gmail.com) Jan 2010,
//   to add PNG image support

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conv_shared.h"

#include <bzlib.h>
#include "resize_image.h"

int scale_ratio_upper;
int scale_ratio_lower;

unsigned char *rescaled_tmp2_buffer = NULL;
size_t rescaled_tmp2_length = 0;
unsigned char *rescaled_tmp_buffer = NULL;
size_t rescaled_tmp_length = 0;

unsigned char *restored_buffer = NULL;
size_t restored_length = 0;

#define INPUT_BUFFER_SIZE       4096

void rescaleImage( unsigned char *original_buffer, int width, int height, int byte_per_pixel,
                   bool src_pad_flag, bool dst_pad_flag, bool palette_flag, int num_of_cells )
{
    size_t width_pad = 0;
    if ( src_pad_flag ) width_pad = (4 - width * byte_per_pixel % 4) % 4;
    
    size_t w = (int)((width / num_of_cells) * scale_ratio_upper / scale_ratio_lower) * num_of_cells;
    size_t h = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( w==0 ) w=num_of_cells;
    if ( h==0 ) h=1;
    size_t w_pad = 0;
    if ( dst_pad_flag ) w_pad = (4 - w * byte_per_pixel % 4) % 4;

    //printf("rescaling: (%d,%d) -> (%d,%d), w_pad=%d\n", width, height, w, h, w_pad); fflush(stdout);
    if ( ((width / num_of_cells) * num_of_cells) != width)
        printf("warning: image width %d is not a multiple of %d, some pixel data may be lost\n", width, num_of_cells);
    if  ( ((w * byte_per_pixel + w_pad) * h) > rescaled_tmp_length ){
        int len = (w * byte_per_pixel + w_pad) * h;
        if ( rescaled_tmp_buffer ) delete[] rescaled_tmp_buffer;
        rescaled_tmp_buffer = new unsigned char[ len ];
        rescaled_tmp_length = len;
    }

    size_t tmp_width = width * byte_per_pixel + width_pad;
    size_t len = tmp_width * (height+1) + byte_per_pixel;
    if (scale_ratio_upper > scale_ratio_lower){
        tmp_width = w * byte_per_pixel + w_pad;
        len = tmp_width * (h+1) + byte_per_pixel;
    }
    if ( len<16 ) len = 16;
    if ( len > rescaled_tmp2_length ){
        if ( rescaled_tmp2_buffer ) delete[] rescaled_tmp2_buffer;
        rescaled_tmp2_buffer = new unsigned char[ len ];
        rescaled_tmp2_length = len;
    }

    resizeImage( rescaled_tmp_buffer, w, h, w*byte_per_pixel+w_pad,
                 original_buffer, width, height, width*byte_per_pixel+width_pad,
                 byte_per_pixel, rescaled_tmp2_buffer, tmp_width,
                 num_of_cells, palette_flag );
}


void rescaleBMPWrite( unsigned char *original_buffer, size_t total_size, int width, int height, unsigned char **rescaled_buffer )
{
    int buffer_offset = original_buffer[10] + (original_buffer[11] << 8);
    memcpy( *rescaled_buffer, original_buffer, buffer_offset );
    memcpy( *rescaled_buffer + buffer_offset, rescaled_tmp_buffer, total_size - buffer_offset );

    *(*rescaled_buffer + 2) = total_size & 0xff;
    *(*rescaled_buffer + 3) = (total_size >>  8) & 0xff;
    *(*rescaled_buffer + 4) = (total_size >> 16) & 0xff;
    *(*rescaled_buffer + 5) = (total_size >> 24) & 0xff;
    *(*rescaled_buffer + 18) = width & 0xff;
    *(*rescaled_buffer + 19) = (width >>  8) & 0xff;
    *(*rescaled_buffer + 20) = (width >> 16) & 0xff;
    *(*rescaled_buffer + 21) = (width >> 24) & 0xff;
    *(*rescaled_buffer + 22) = height & 0xff;
    *(*rescaled_buffer + 23) = (height >>  8) & 0xff;
    *(*rescaled_buffer + 24) = (height >> 16) & 0xff;
    *(*rescaled_buffer + 25) = (height >> 24) & 0xff;
    *(*rescaled_buffer + 34) = 0;
    *(*rescaled_buffer + 35) = 0;
    *(*rescaled_buffer + 36) = 0;
    *(*rescaled_buffer + 37) = 0;

#if 0
    FILE *fp = fopen( "test.bmp", "wb" );
    fwrite( *rescaled_buffer, 1, width2 * height2 * byte_per_pixel + 54 + color_num*4, fp );
    fclose(fp);
    getchar();
#endif
}

size_t rescaleBMP( unsigned char *original_buffer, unsigned char **rescaled_buffer,
                   bool output_png_flag, bool output_jpeg_flag, int quality, int num_of_cells=1 )
{
    if (original_buffer[14] != 40){
        if (original_buffer[14] == 12)
            fprintf( stderr, " OS/2 format is not supported.\n");
        else
            fprintf( stderr, " this bitmap can't be handled.\n");
        exit(-1);
    }

    int buffer_offset = original_buffer[10] + (original_buffer[11] << 8);
    int width  = original_buffer[18] + (original_buffer[19] << 8);
    int height = original_buffer[22] + (original_buffer[23] << 8);

    int bit_per_pixel = original_buffer[28];
    if (bit_per_pixel == 1 || bit_per_pixel == 4){
        fprintf( stderr, " bit_per_pixel %d is not supported.\n", bit_per_pixel);
        exit(-1);
    }
    int byte_per_pixel = bit_per_pixel / 8;
    int color_num = original_buffer[46] + ((int)original_buffer[47] << 8) + (original_buffer[48] << 16) + (original_buffer[49] << 24);
    if (bit_per_pixel == 8 && color_num == 0) color_num = 256;

    bool palette_flag = false;
    if (bit_per_pixel == 8) palette_flag = true;
    if (palette_flag) {
        output_jpeg_flag = false;
        output_png_flag = false;
    }

    size_t width2  = (int)((width / num_of_cells) * scale_ratio_upper / scale_ratio_lower) * num_of_cells;
    if ( width2 == 0 ) width2 = num_of_cells;
    size_t width2_pad = (4 - width2 * byte_per_pixel % 4) % 4;
    
    size_t height2 = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( height2 == 0 ) height2 = 1;

    size_t total_size = (width2 * byte_per_pixel + width2_pad) * height2 + buffer_offset;
    if ( total_size+0x400 > restored_length ){
        restored_length = total_size+0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }

    if (output_jpeg_flag){
        rescaleImage( original_buffer+buffer_offset, width, height,
                      byte_per_pixel, true, false, palette_flag, num_of_cells );
        total_size = rescaleJPEGWrite(width, height, byte_per_pixel, rescaled_buffer, quality, true, num_of_cells);
        printf(" BMP->JPG");
    }
    else if (output_png_flag){
        rescaleImage( original_buffer+buffer_offset, width, height,
                      byte_per_pixel, true, false, palette_flag, num_of_cells );
        total_size = rescalePNGWrite( width, height, byte_per_pixel, rescaled_buffer, NULL, NULL, palette_flag, num_of_cells );
        printf(" BMP->PNG");
    }
    else {
        rescaleImage( original_buffer+buffer_offset, width, height,
                      byte_per_pixel, true, true, palette_flag, num_of_cells );
        rescaleBMPWrite(original_buffer, total_size, width2, height2, rescaled_buffer);
        printf(" BMP");
    }

    return total_size;
}
