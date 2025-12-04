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

size_t rescaleJPEGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                         int quality, bool bmp2jpeg_flag, int num_of_cells );
size_t rescaleJPEG( unsigned char *original_buffer, size_t length,
                    unsigned char **rescaled_buffer, int quality, int num_of_cells=1 );

size_t rescalePNGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                        void* /*png_structp*/ png_ptr, void* /*png_infop*/ info_ptr, bool palette_flag, int num_of_cells );

size_t rescalePNG( unsigned char *original_buffer, size_t length,
                    unsigned char **rescaled_buffer, int num_of_cells=1 );

#endif
