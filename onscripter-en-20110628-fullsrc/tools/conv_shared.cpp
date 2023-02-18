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

namespace jpeglib { //Mion: added namespace to avoid type conflicts
    extern "C" {
#include <jpeglib.h>
    };
}

//Mion: for png image file support
namespace libpng {
#include <png.h>
}

#include <bzlib.h>
#include "resize_image.h"

int scale_ratio_upper;
int scale_ratio_lower;

unsigned char *rescaled_tmp2_buffer = NULL;
size_t rescaled_tmp2_length = 0;
unsigned char *rescaled_tmp_buffer = NULL;
size_t rescaled_tmp_length = 0;

static unsigned char *restored_buffer = NULL;
static size_t restored_length = 0;

#define INPUT_BUFFER_SIZE       4096
typedef struct {
    struct jpeglib::jpeg_source_mgr pub;

    unsigned char *buf;
    size_t left;
} my_source_mgr;

typedef struct {
    struct jpeglib::jpeg_destination_mgr pub;

    unsigned char *buf;
    size_t left;
} my_destination_mgr;

//Mion: for png image file support
typedef struct {
    libpng::png_infop info_ptr;
    int number_of_passes;
    libpng::png_bytep * row_pointers;

    unsigned char *buf;
    size_t left;
} my_png_mgr;


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


void init_source (jpeglib::j_decompress_ptr cinfo)
{
}

jpeglib::boolean fill_input_buffer (jpeglib::j_decompress_ptr cinfo)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte = src->buf;
    src->pub.bytes_in_buffer = src->left;

    return TRUE;
}

void skip_input_data (jpeglib::j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

void term_source (jpeglib::j_decompress_ptr cinfo)
{
}

void init_destination (jpeglib::j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
}

jpeglib::boolean empty_output_buffer (jpeglib::j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
    
    return TRUE;
}

void term_destination (jpeglib::j_compress_ptr cinfo)
{
}

size_t rescaleJPEGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                         int quality, bool bmp2jpeg_flag, int num_of_cells )
{
    using namespace jpeglib;

    jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo2;
    JSAMPROW row_pointer[1];
    
    cinfo2.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo2);

    cinfo2.dest = (struct jpeg_destination_mgr *)
        (*cinfo2.mem->alloc_small) ((j_common_ptr) &cinfo2, JPOOL_PERMANENT,
                                    sizeof(my_destination_mgr));
    my_destination_mgr * dest = (my_destination_mgr *) cinfo2.dest;

    cinfo2.image_width = (int)((width / num_of_cells) * scale_ratio_upper / scale_ratio_lower) * num_of_cells;
    if ( cinfo2.image_width == 0 ) cinfo2.image_width = num_of_cells;
    cinfo2.image_height = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( cinfo2.image_height == 0 ) cinfo2.image_height = 1;

    size_t total_size = cinfo2.image_width * byte_per_pixel * cinfo2.image_height;
    if ((total_size + 0x400) > restored_length){
        restored_length = total_size + 0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }

    dest->buf = *rescaled_buffer;
    dest->left = restored_length;

    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;

    cinfo2.input_components = byte_per_pixel;
    if ( cinfo2.input_components == 1 )
        cinfo2.in_color_space = JCS_GRAYSCALE;
    else
        cinfo2.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo2);
    jpeg_set_quality(&cinfo2, quality, TRUE );
    cinfo2.optimize_coding = true;
    //jpeg_simple_progression (&cinfo2);
    jpeg_start_compress(&cinfo2, TRUE);

    int row_stride = cinfo2.image_width * byte_per_pixel;

    while (cinfo2.next_scanline < cinfo2.image_height) {
        if (bmp2jpeg_flag){
            unsigned char *src = row_pointer[0] = &rescaled_tmp_buffer[(cinfo2.image_height - 1 - cinfo2.next_scanline) * row_stride];
            for(unsigned int i=0 ; i<cinfo2.image_width ; i++, src+=3){
                unsigned char tmp = src[2];
                src[2] = src[0];
                src[0] = tmp;
            }
        }
        else{
            row_pointer[0] = &rescaled_tmp_buffer[cinfo2.next_scanline * row_stride];
        }
        jpeg_write_scanlines(&cinfo2, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo2);
    size_t datacount = dest->left - dest->pub.free_in_buffer;

    jpeg_destroy_compress(&cinfo2);

    return datacount;
}

size_t rescaleJPEG( unsigned char *original_buffer, size_t length,
                    unsigned char **rescaled_buffer, int quality, int num_of_cells=1 )
{
    using namespace jpeglib;

    struct jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    cinfo.src = (struct jpeg_source_mgr *)
        (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                   sizeof(my_source_mgr));
    my_source_mgr * src = (my_source_mgr *) cinfo.src;
    
    src->buf = original_buffer;
    src->left = length;

    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = term_source;

    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;

    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if ( cinfo.output_width * cinfo.output_height * cinfo.output_components + 0x400 > restored_length ){
        restored_length = cinfo.output_width * cinfo.output_height * cinfo.output_components + 0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }
    int row_stride = cinfo.output_width * cinfo.output_components;

    JSAMPARRAY buf = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    unsigned char *buf_p = restored_buffer;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buf, 1);
        memcpy( buf_p, buf[0], row_stride );
        buf_p += cinfo.output_width * cinfo.output_components;
    }

    rescaleImage( restored_buffer, cinfo.output_width, cinfo.output_height,
                  cinfo.output_components, false, false, false, num_of_cells );

    size_t datacount = rescaleJPEGWrite(cinfo.output_width, cinfo.output_height,
                                        cinfo.output_components, rescaled_buffer,
                                        quality, false, num_of_cells);
    printf(" JPG");
    jpeg_destroy_decompress(&cinfo);

    return datacount;
}

//Mion: functions for png image file support
void my_read_data(libpng::png_structp png_ptr, libpng::png_bytep data,
                  libpng::png_size_t length)
{
    using namespace libpng;
    if ( !(length > 0) || (png_get_io_ptr(png_ptr) == NULL))
        return;

    my_png_mgr *src_mgr = (my_png_mgr *) png_get_io_ptr(png_ptr);

    if (length > src_mgr->left) return;
    
    for (png_size_t x=0; x<length; x++, src_mgr->left--)
        *data++ = *(src_mgr->buf++);
}

void my_write_data(libpng::png_structp png_ptr, libpng::png_bytep data,
                   libpng::png_size_t length)
{
    using namespace libpng;
    if ( !(length > 0) || (png_get_io_ptr(png_ptr) == NULL))
        return;

    my_png_mgr *dst_mgr = (my_png_mgr *) png_get_io_ptr(png_ptr);

    if (length > dst_mgr->left){
        printf("png my_write_data ERROR: length %d bytes > buffer left %d bytes\n",
               (unsigned int)length, (unsigned int)dst_mgr->left);
        fflush(stdout);
        return;
    }
    
    for (png_size_t x=0; x<length; x++, dst_mgr->left--)
        *(dst_mgr->buf++) = *data++;
}

void my_flush_data(libpng::png_structp png_ptr)
{
}

size_t rescalePNGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                        libpng::png_structp png_ptr, libpng::png_infop info_ptr, bool palette_flag, int num_of_cells )
{
    using namespace libpng;
    png_structp png_dst_ptr;
    my_png_mgr png_dst_mgr;
    int rowbytes;

    png_dst_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_dst_ptr){
        fprintf(stderr, "png_create_write_struct failed\n");
        return 0;
    }

    png_set_write_fn(png_dst_ptr, (voidp) &png_dst_mgr, my_write_data,
                     my_flush_data);

    png_dst_mgr.info_ptr = png_create_info_struct(png_dst_ptr);
    if (!png_dst_mgr.info_ptr){
        png_destroy_read_struct(&png_dst_ptr, NULL, NULL);
        fprintf(stderr, "png_create_info_struct failed\n");
        return 0;
    }

    unsigned int image_width = (unsigned int)((width / num_of_cells) * scale_ratio_upper / scale_ratio_lower) * num_of_cells;
    if ( image_width == 0 ) image_width = num_of_cells;
    unsigned int image_height = (unsigned int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( image_height == 0 ) image_height = 1;
    
    size_t total_size = image_width * byte_per_pixel * image_height;
    if ((total_size + 0x400) > restored_length){
        restored_length = total_size + 0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }

    png_dst_mgr.buf = *rescaled_buffer;
    png_dst_mgr.left = restored_length;

    //printf("writing rescaled png: (%d,%d)\n", image_width, image_height); fflush(stdout);
    if (png_ptr && info_ptr){
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)){
            png_colorp pal;
            int num_pal;
            int bit_depth = 8;
            png_get_PLTE(png_ptr, info_ptr, &pal, &num_pal);
            //printf("pallette, %d colors\n", num_pal); fflush(stdout);
            if (num_pal <= 2)
                bit_depth = 1;
            else if (num_pal <= 4)
                bit_depth = 2;
            else if (num_pal <= 16)
                bit_depth = 4;
            png_set_IHDR(png_dst_ptr, png_dst_mgr.info_ptr,
                         image_width, image_height,
                         bit_depth,
                         png_get_color_type(png_ptr,info_ptr),
                         PNG_INTERLACE_NONE, //interlacing not useful for ONS
                         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            png_set_PLTE(png_dst_ptr, png_dst_mgr.info_ptr, pal, num_pal);
        } else {
            png_set_IHDR(png_dst_ptr, png_dst_mgr.info_ptr,
                         image_width, image_height,
                         png_get_bit_depth(png_ptr,info_ptr),
                         png_get_color_type(png_ptr,info_ptr),
                         PNG_INTERLACE_NONE, //interlacing not useful for ONS
                         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        }
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)){
            png_bytep trans_pal;
            png_color_16p trans_color;
            int num_trans;
            png_get_tRNS(png_ptr, info_ptr, &trans_pal, &num_trans, &trans_color);
            //printf("%d transparent colors, single=%d\n", num_trans, trans_color->index);
            //fflush(stdout);
            png_set_tRNS(png_dst_ptr, png_dst_mgr.info_ptr, trans_pal, num_trans, trans_color);
        }
    } else {
        png_set_IHDR(png_dst_ptr, png_dst_mgr.info_ptr, image_width, image_height,
                     8,
                     palette_flag ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    }
    png_set_compression_level(png_dst_ptr, 6);

    png_write_info(png_dst_ptr, png_dst_mgr.info_ptr);

    if (palette_flag){
        png_set_packing(png_dst_ptr);
        rowbytes = image_width;
    } else {
        rowbytes = png_get_rowbytes(png_dst_ptr, png_dst_mgr.info_ptr);
    }
    png_dst_mgr.row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image_height);

    if (!png_ptr || !info_ptr){
        //no png struct, so from a BMP
        png_set_bgr(png_dst_ptr);
        for (unsigned int y=0; y<image_height; y++){
            png_dst_mgr.row_pointers[y] = &rescaled_tmp_buffer[(image_height - y - 1) * rowbytes];
        }
    } else {
        unsigned char *buf_p = rescaled_tmp_buffer;
        for (unsigned int y=0; y<image_height; y++){
            png_dst_mgr.row_pointers[y] = (png_byte*) buf_p;
            buf_p += rowbytes;
        }
    }
    png_write_image(png_dst_ptr, png_dst_mgr.row_pointers);

    free(png_dst_mgr.row_pointers);
    png_write_end(png_dst_ptr, png_dst_mgr.info_ptr);
    png_destroy_write_struct(&png_dst_ptr, &png_dst_mgr.info_ptr);

    return (size_t) restored_length - png_dst_mgr.left;
}

size_t rescalePNG( unsigned char *original_buffer, size_t length,
                    unsigned char **rescaled_buffer, int num_of_cells=1 )
{
    using namespace libpng;
    unsigned int width, height;
    bool palette_flag = false;
    png_byte color_type, bit_depth;
    png_structp png_src_ptr;
    my_png_mgr png_src_mgr;

    if (png_sig_cmp(original_buffer, 0, 8)){
        fprintf(stderr, "Not a PNG file\n");
        return 0;
    }

    png_src_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_src_ptr){
        fprintf(stderr, "png_create_read_struct failed\n");
        return 0;
    }

    png_set_read_fn(png_src_ptr, (voidp) &png_src_mgr, my_read_data);

    png_src_mgr.info_ptr = png_create_info_struct(png_src_ptr);
    if (!png_src_mgr.info_ptr){
        png_destroy_read_struct(&png_src_ptr, NULL, NULL);
        fprintf(stderr, "png_create_info_struct failed\n");
        return 0;
    }

    png_src_mgr.buf = original_buffer;
    png_src_mgr.left = length;

    if (setjmp(png_jmpbuf(png_src_ptr))){
        png_destroy_read_struct(&png_src_ptr, &png_src_mgr.info_ptr, NULL);
        fprintf(stderr, "error during png init_io\n");
        return 0;
    }

    png_read_info(png_src_ptr, png_src_mgr.info_ptr);

    width = png_get_image_width(png_src_ptr, png_src_mgr.info_ptr);
    height = png_get_image_height(png_src_ptr, png_src_mgr.info_ptr);
    color_type = png_get_color_type(png_src_ptr, png_src_mgr.info_ptr);
    bit_depth = png_get_bit_depth(png_src_ptr, png_src_mgr.info_ptr);
    int rowbytes = png_get_rowbytes(png_src_ptr, png_src_mgr.info_ptr);
    int byte_per_pixel = rowbytes / width;
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        palette_flag = true;
    //printf("got png image: %d x %d, %d rowbytes, %dBPP, bit depth %d, ", width, height, rowbytes, byte_per_pixel, bit_depth);
    //switch (color_type){
    //  case PNG_COLOR_TYPE_GRAY: printf("gray\n"); break;
    //  case PNG_COLOR_TYPE_GRAY_ALPHA: printf("gray alpha\n"); break;
    //  case PNG_COLOR_TYPE_PALETTE: printf("palette\n"); break;
    //  case PNG_COLOR_TYPE_RGB: printf("RGB\n"); break;
    //  case PNG_COLOR_TYPE_RGB_ALPHA: printf("RGBA\n"); break;
    //  default: printf("\n"); break;
    //}
    //fflush(stdout);

    if (bit_depth == 16)
        png_set_strip_16(png_src_ptr);
    png_set_packing(png_src_ptr);

    png_src_mgr.number_of_passes = png_set_interlace_handling(png_src_ptr);
    png_read_update_info(png_src_ptr, png_src_mgr.info_ptr);

    if (setjmp(png_jmpbuf(png_src_ptr))){
        png_destroy_read_struct(&png_src_ptr, &png_src_mgr.info_ptr, NULL);
        fprintf(stderr, "error during png read_image\n");
        return 0;
    }

    rowbytes = png_get_rowbytes(png_src_ptr, png_src_mgr.info_ptr);
    color_type = png_get_color_type(png_src_ptr, png_src_mgr.info_ptr);
    bit_depth = png_get_bit_depth(png_src_ptr, png_src_mgr.info_ptr);
    byte_per_pixel = rowbytes / width;
    //printf("restored image: %d rowbytes, %dBPP, bit depth %d\n", rowbytes, byte_per_pixel, bit_depth); fflush(stdout);

    if ( (height * rowbytes + 0x400) > restored_length ){
        restored_length = height * rowbytes + 0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }

    png_src_mgr.row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (unsigned int y=0; y<height; y++)
        png_src_mgr.row_pointers[y] = (png_byte*) malloc(rowbytes);

    png_read_image(png_src_ptr, png_src_mgr.row_pointers);

    unsigned char *buf_p = restored_buffer;
    for (unsigned int y=0; y<height; y++){
        memcpy( buf_p, png_src_mgr.row_pointers[y], rowbytes );
        free(png_src_mgr.row_pointers[y]);
        buf_p += rowbytes;
    }
    free(png_src_mgr.row_pointers);

    rescaleImage( restored_buffer, width, height, byte_per_pixel,
                  false, false, palette_flag, num_of_cells );

    size_t datacount = rescalePNGWrite( width, height, byte_per_pixel,
                                        rescaled_buffer, png_src_ptr,
                                        png_src_mgr.info_ptr,
                                        palette_flag, num_of_cells );
    printf(" PNG");

    png_destroy_read_struct(&png_src_ptr, &png_src_mgr.info_ptr, NULL);

    return datacount;
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
