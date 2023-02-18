/* -*- C++ -*-
 * 
 *  AnimationInfo.h - General image storage class of ONScripter-EN
 *
 *  Copyright (c) 2001-2011 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2007-2011 "Uncle" Mion Sonozaki
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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ANIMATION_INFO_H__
#define __ANIMATION_INFO_H__

#include <SDL.h>
#include <SDL_image.h>
#include <string.h>
#include "BaseReader.h"

#define TRANSBTN_CUTOFF 15 //alpha threshold for ignoring transparent areas

typedef unsigned char uchar3[3];

//useful utility functions
inline bool equalStrings(const char *str1, const char *str2) {
    return ( ((str1 == NULL) && (str2 == NULL)) ||
             ((str1 != NULL) && (str2 != NULL) &&
              (str1[0] == str2[0]) && (strcmp(str1,str2) == 0)) );
}

inline bool equalColors(const uchar3 &color1, const uchar3 &color2) {
    return ((color1[0] == color2[0]) && (color1[1] == color2[1]) &&
            (color1[2] == color2[2]));
}

class AnimationInfo{
public:
#ifdef BPP16
    typedef Uint16 ONSBuf;
#else
    typedef Uint32 ONSBuf;
#endif    
    enum { TRANS_ALPHA          = 1,
           TRANS_TOPLEFT        = 2,
           TRANS_COPY           = 3,
           TRANS_STRING         = 4,
           TRANS_DIRECT         = 5,
           TRANS_PALETTE        = 6,
           TRANS_TOPRIGHT       = 7,
           TRANS_MASK           = 8,
#ifndef NO_LAYER_EFFECTS
           TRANS_LAYER          = 9
#endif
    };

    bool is_copy; // allocated buffers should not be deleted from a copied instance
    bool stale_image; //set to true when the image needs to be created/redone

    SDL_Rect orig_pos; //Mion: position and size of the image before resizing
    SDL_Rect pos; // position and size of the current cell

    /* variables set from the image tag */
    int trans_mode;
    uchar3 direct_color;
    int palette_number;
    uchar3 color;
    int num_of_cells;
    int current_cell;
    int direction;
    int *duration_list;
    uchar3 *color_list;
    int loop_mode;
    bool is_animatable;
    bool is_single_line;
    bool is_tight_region; // valid under TRANS_STRING
    bool is_ruby_drawable;
    bool skip_whitespace;
#ifndef NO_LAYER_EFFECTS
    int layer_no; //Mion: for Layer effects
#endif
    char *file_name;
    char *mask_file_name;

    /* Variables from AnimationInfo */
    bool visible;
    bool abs_flag;
    bool affine_flag;
    int trans;
    char *image_name;
    SDL_Surface *image_surface;
    unsigned char *alpha_buf;
    /* Variables for extended sprite (lsp2, drawsp2, etc.) */
    int scale_x, scale_y, rot;
    int mat[2][2], inv_mat[2][2];
    int corner_xy[4][2];
    SDL_Rect bounding_rect;

    enum { BLEND_NORMAL      = 0,
           BLEND_ADD         = 1,
           BLEND_SUB         = 2
    };
    int blending_mode;
    int cos_i, sin_i;
    
    int font_size_xy[2]; // used by prnum and lsp string
    int font_pitch; // used by lsp string
    int remaining_time;

    int param; // used by prnum and bar
    int max_param; // used by bar
    int max_width; // used by bar
    
    AnimationInfo();
    AnimationInfo(const AnimationInfo &anim);
    ~AnimationInfo();

    AnimationInfo& operator =(const AnimationInfo &anim);
    void deepcopyTag(const AnimationInfo &anim);
    void deepcopy(const AnimationInfo &anim);

    void reset();
    
    void deleteImageName();
    void setImageName( const char *name );
    void deleteImage();
    void remove();
    void removeTag();

    bool proceedAnimation();

    void setCell(int cell);
    static int doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped=NULL );
    SDL_Rect findOpaquePoint(SDL_Rect *clip=NULL);
    int getPixelAlpha( int x, int y );
    void blendOnSurface( SDL_Surface *dst_surface, int dst_x, int dst_y,
                         SDL_Rect &clip, int alpha=256 );
    void blendOnSurface2( SDL_Surface *dst_surface, int dst_x, int dst_y,
                          SDL_Rect &clip, int alpha=256 );
    void blendText( SDL_Surface *surface, int dst_x, int dst_y,
                    SDL_Color &color, SDL_Rect *clip, bool rotate_flag );
    void calcAffineMatrix();
    
    static SDL_Surface *allocSurface( int w, int h );
    void allocImage( int w, int h );
    void copySurface( SDL_Surface *surface, SDL_Rect *src_rect, SDL_Rect *dst_rect = NULL );
    void fill( Uint8 r, Uint8 g, Uint8 b, Uint8 a );
    SDL_Surface *setupImageAlpha( SDL_Surface *surface, SDL_Surface *surface_m, bool has_alpha );
#ifdef RCA_SCALE
    SDL_Surface *resize( SDL_Surface *surface, int ratio1=1, int ratio2=1, float stretch_x=1.0, float stretch_y=1.0 );
#else
    SDL_Surface *resize( SDL_Surface *surface, int ratio1=1, int ratio2=1 );
#endif
    void setImage( SDL_Surface *surface );
};

#endif // __ANIMATION_INFO_H__
