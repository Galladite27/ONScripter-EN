
#include <cstring>

#include "conv_shared.h"

//Mion: for png image file support
#include <png.h>

//Mion: for png image file support
typedef struct {
    png_infop info_ptr;
    int number_of_passes;
    png_bytep * row_pointers;

    unsigned char *buf;
    size_t left;
} my_png_mgr;

//Mion: functions for png image file support
void my_read_data(png_structp png_ptr, png_bytep data,
                  png_size_t length)
{
    if ( !(length > 0) || (png_get_io_ptr(png_ptr) == NULL))
        return;

    my_png_mgr *src_mgr = (my_png_mgr *) png_get_io_ptr(png_ptr);

    if (length > src_mgr->left) return;
    
    for (png_size_t x=0; x<length; x++, src_mgr->left--)
        *data++ = *(src_mgr->buf++);
}

void my_write_data(png_structp png_ptr, png_bytep data,
                   png_size_t length)
{
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

void my_flush_data(png_structp /*png_ptr*/)
{
}

size_t rescalePNGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                        void* /*png_structp*/ png_ptr_tmp, void* /*png_infop*/ info_ptr_tmp, bool palette_flag, int num_of_cells )
{
    png_structp png_ptr = (png_structp)png_ptr_tmp;
    png_infop info_ptr = (png_infop)info_ptr_tmp;
    png_structp png_dst_ptr;
    my_png_mgr png_dst_mgr;
    int rowbytes;

    png_dst_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_dst_ptr){
        fprintf(stderr, "png_create_write_struct failed\n");
        return 0;
    }

    png_set_write_fn(png_dst_ptr, (png_voidp) &png_dst_mgr, my_write_data,
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
                    unsigned char **rescaled_buffer, int num_of_cells )
{
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

    png_set_read_fn(png_src_ptr, (png_voidp) &png_src_mgr, my_read_data);

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