/* -*- C++ -*-
 *
 *  Layer.h - Base class for effect layers for ONScripter-EN
 *
 *  Copyright (c) 2009 "Uncle" Mion Sonozaki
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

#ifndef __LAYER_H__
#define __LAYER_H__

#include "AnimationInfo.h"
#include "BaseReader.h"

#ifndef NO_LAYER_EFFECTS

struct Pt { int x; int y; int type; int cell; };

struct Layer
{
    BaseReader *reader;
    AnimationInfo *sprite_info, *sprite;
    int width, height;

    virtual ~Layer(){};
    
    void setSpriteInfo( AnimationInfo *sinfo, AnimationInfo *anim ){
        sprite_info = sinfo;
        sprite = anim;
    };
    virtual void update( ) = 0;
    virtual char* message( const char *message, int &ret_int ) = 0;
    virtual void refresh( SDL_Surface* surface, SDL_Rect &clip ) = 0;
};

#ifndef BPP16
// OldMovieLayer: emulation of Takashi Toyama's "oldmovie.dll" NScripter plugin filter
class OldMovieLayer : public Layer
{
public:
    OldMovieLayer( int w, int h );
    ~OldMovieLayer();
    void update();
    char* message( const char *message, int &ret_int );
    void refresh( SDL_Surface* surface, SDL_Rect &clip );

private:
    // message parameters
    int blur_level;
    int noise_level;
    int glow_level;
    int scratch_level;
    int dust_level;
    AnimationInfo *dust_sprite;
    AnimationInfo *dust;

    Pt *dust_pts;
    int rx, ry, // Offset of blur (second copy of background image)
        ns;     // Current noise surface
    int gv, // Current glow level
        go; // Glow delta: flips between 1 and -1 to fade the glow in and out.
    bool initialized;

    void om_init();
    //void BlendOnSurface(SDL_Surface* src, SDL_Surface* dst, SDL_Rect clip);
};
#endif //BPP16

#define N_FURU_ELEMENTS 3
#define FURU_ELEMENT_BUFSIZE 512 // should be a power of 2
#define FURU_AMP_TABLE_SIZE 256 // should also be power of 2, it helps

// FuruLayer: emulation of Takashi Toyama's "snow.dll" and "hana.dll" NScripter plugin filters
class FuruLayer : public Layer
{
public:
    FuruLayer( int w, int h, bool animated, BaseReader *br=NULL );
    ~FuruLayer();
    void update();
    char* message( const char *message, int &ret_int );
    void refresh( SDL_Surface* surface, SDL_Rect &clip );

private:
    bool tumbling; // true (hana) or false (snow)

    // message parameters
    int interval; // 1 ~ 10000; # frames between a new element release
    int fall_velocity; // 1 ~ screen_height; pix/frame
    int wind; // -screen_width/2 ~ screen_width/2; pix/frame 
    int amplitude; // 0 ~ screen_width/2; pix/frame
    int freq; // 0 ~ 359; degree/frame
    int angle;
    bool paused, halted;

    struct OscPt { // point plus base oscillation angle
        int base_angle;
        Pt pt;
    };
    struct Element {
        AnimationInfo *sprite;
        int *amp_table;
        // rolling buffer
        OscPt *points;
        int pstart, pend, frame_cnt, fall_speed;
        Element(){
            sprite = NULL;
            amp_table = NULL;
            points = NULL;
            pstart = pend = frame_cnt = fall_speed = 0;
        };
        ~Element(){
            if (sprite) delete sprite;
            if (amp_table) delete[] amp_table;
            if (points) delete[] points;
        };
        void init(){
            if (!points) points = new OscPt[FURU_ELEMENT_BUFSIZE];
            pstart = pend = frame_cnt = 0;
        };
        void clear(){
            if (sprite) delete sprite;
            sprite = NULL;
            if (amp_table) delete[] amp_table;
            amp_table = NULL;
            if (points) delete[] points;
            points = NULL;
            pstart = pend = frame_cnt = 0;
        };
        void setSprite(AnimationInfo *anim){
            if (sprite) delete sprite;
            sprite = anim;
        };
    } elements[N_FURU_ELEMENTS];
    int max_sp_w;

    bool initialized;

    void furu_init();
    void validate_params();
    void buildAmpTables();
};

#endif //ndef NO_LAYER_EFFECTS

#endif // __LAYER_H__
