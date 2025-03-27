#ifndef __CONV_SHARED__
#define __CONV_SHARED__

#include <stdlib.h>

extern int scale_ratio_upper;
extern int scale_ratio_lower;
extern unsigned char *rescaled_tmp2_buffer;
extern size_t rescaled_tmp2_length;
extern unsigned char *rescaled_tmp_buffer;
extern size_t rescaled_tmp_length;
extern unsigned char *restored_buffer;
extern size_t restored_length;
#define INPUT_BUFFER_SIZE       4096


void rescaleImage( unsigned char *original_buffer, int width, int height, int byte_per_pixel,
                   bool src_pad_flag, bool dst_pad_flag, bool palette_flag, int num_of_cells );

//boolean fill_input_buffer (j_decompress_ptr cinfo);
//void skip_input_data (j_decompress_ptr cinfo, long num_bytes);
//void term_source (j_decompress_ptr cinfo);
//void init_destination (j_compress_ptr cinfo);
//boolean empty_output_buffer (j_compress_ptr cinfo);
//void term_destination (j_compress_ptr cinfo);
size_t rescaleJPEGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                         int quality, bool bmp2jpeg_flag, int num_of_cells );
size_t rescaleJPEG( unsigned char *original_buffer, size_t length,
                    unsigned char **rescaled_buffer, int quality, int num_of_cells=1 );

#endif