
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <jpeglib.h>

#include "conv_shared.h"
#include "resize_image.h"

typedef struct {
    struct jpeg_source_mgr pub;

    unsigned char *buf;
    size_t left;
} my_source_mgr;

typedef struct {
    struct jpeg_destination_mgr pub;

    unsigned char *buf;
    size_t left;
} my_destination_mgr;

void init_source (j_decompress_ptr cinfo)
{
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte = src->buf;
    src->pub.bytes_in_buffer = src->left;

    return TRUE;
}

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

void term_source (j_decompress_ptr cinfo)
{
}

void init_destination (j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
}

boolean empty_output_buffer (j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
    
    return TRUE;
}

void term_destination (j_compress_ptr cinfo)
{
}

size_t rescaleJPEGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                         int quality, bool bmp2jpeg_flag, int num_of_cells )
{
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
    cinfo2.optimize_coding = TRUE;
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
                    unsigned char **rescaled_buffer, int quality, int num_of_cells)
{
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