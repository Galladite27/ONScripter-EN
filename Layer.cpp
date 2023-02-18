/* -*- C++ -*-
 * 
 *  Layer.cpp - Code for effect layers for ONScripter-EN
 *
 *  Copyright (c) 2009-2011 "Uncle" Mion Sonozaki
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

/*
 *  Emulation of Takashi Toyama's "oldmovie.dll", "snow.dll", and "hana.dll"
 *  NScripter plugin filters.
 */
#include "Layer.h"
#include "graphics_sum.h"

#ifndef NO_LAYER_EFFECTS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_SPRITE_NUM 1000

inline static void drawTaggedSurface( SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect &clip )
{
        anim->blendOnSurface( dst_surface, anim->pos.x, anim->pos.y,
                              clip, anim->trans );
}

#ifndef BPP16 //not supporting 16bpp for "oldmovie" yet
/*
 *  Emulation of Takashi Toyama's "oldmovie.dll" NScripter filter for ONScripter.
 *
 *  The old-movie effect itself is created in a rather inefficient way, compared
 *  to Toyama-san's tight ASM original, but we have to run on PowerPC as well. ^^
 *  Using MMX-optimised composition routines should claw back some of
 *  the lost performance on x86 systems.
 *
 *  Copyright (c) 2006 Peter Jolly.
 *
 *  haeleth@haeleth.net
 *
 */
// Modified extensively by Mion, 2008
// Modified by Mion, Dec 2009, to optimize and cleanup code

#define MAX_NOISE          8 // Number of noise surfaces.
#define MAX_GLOW          25 // Number of glow levels.
#define MAX_DUST_COUNT    10 // Number of dust particles.
#define MAX_SCRATCH_COUNT  6 // Number of scratches.
static int scratch_count;    // Number of scratches visible.

class Scratch {
private:
    int offs;   // Tint of the line: 64 for light, -64 for dark, 0 for no scratch.
    int x1, x2; // Horizontal position of top and bottom of the line.
    int dx;     // Distance by which the line moves each frame.
    int time;   // Number of frames remaining before reinitialisation.
    int width, height;
    void init(int level);
public:
    Scratch() : offs(0), time(1) {}
    void setwindow(int w, int h){ width = w; height = h; }
    void update(int level);
    void draw(SDL_Surface* surface, SDL_Rect clip);
};

	// Create a new scratch.
void Scratch::init(int level)
{
    // If this scratch was visible, decrement the counter.
    if (offs) --scratch_count;
    offs = 0;

    // Each scratch object is reinitialised every 3-9 frames.
    time = rand() % 7 + 3;

    if ((rand() % 600) < level) {
        ++scratch_count;
        offs = rand() % 2 ? 64 : -64;
        x1 = rand() % (width - 20) + 10;
        dx = rand() % 12 - 6;
        x2 = x1 - dx; // The angle of the line is determined by the speed of motion.
    }
}

// Called each frame.
void Scratch::update(int level)
{
    if (--time == 0) 
        init(level);
    else if (offs) {
        x1 += dx;
        x2 += dx;
    }
}

// Called each time the screen is refreshed.  Draws a simple line, without antialiasing.
void Scratch::draw(SDL_Surface* surface, SDL_Rect clip) 
{
    // Don't draw unless this scratch is visible and likely to pass through the updated rectangle.
    if ( (offs == 0) || (x1 < clip.x) || (x2 < clip.x) ||
         (x1 >= (clip.x + clip.w)) || (x2 >= (clip.x + clip.w)) )
        return;

    const int sp = surface->pitch;
    float dx = (float)(x2 - x1) / width;
    float realx = (float) x1;
    int y = 0;
    while (y != clip.y) {
         // Skip all scanlines above the clipping rectangle.
        ++y;
        realx += dx;
    }
    while (y < clip.y + clip.h) {
        int lx = (int) floor(realx + 0.5);
        if (lx >= clip.x && lx < clip.x + clip.w) { // Only draw within the clipping rectangle.
            // Get pixel...
            Uint32* p = (Uint32*)((char*)surface->pixels + y * sp + lx * 4);
            const Uint32 c = *p;
            // ...add to or subtract from its colour...
            int c1 = (c & 0xff) + offs, c2 = ((c >> 8) & 0xff) + offs, c3 = ((c >> 16) & 0xff) + offs;
            if (c1 < 0) c1 = 0; else if (c1 > 255) c1 = 255;
            if (c2 < 0) c2 = 0; else if (c2 > 255) c2 = 255;
            if (c3 < 0) c3 = 0; else if (c3 > 255) c3 = 255;
            // ...and put it back.
            *p = c1 | c2 << 8 | c3 << 16;
        }
        ++y;
        realx += dx;
    }
}

static Scratch scratches[MAX_SCRATCH_COUNT];
// We store multiple screens of random noise, and flip between them at random.
static SDL_Surface* NoiseSurface[MAX_NOISE];
// For the glow effect, we store a single surface with a scanline for each glow level.
static SDL_Surface* GlowSurface;
static int om_count = 0;
static bool initialized_om_surfaces = false;

OldMovieLayer::OldMovieLayer( int w, int h )
{
    width = w;
    height = h;

    blur_level = noise_level = glow_level = scratch_level = dust_level = 0;
    dust_sprite = dust = NULL;
    dust_pts = NULL;

    initialized = false;
}

OldMovieLayer::~OldMovieLayer() {
    if (initialized) {
        --om_count;
        if (om_count == 0) {
            for (int i=0; i<MAX_NOISE; i++) {
                SDL_FreeSurface(NoiseSurface[i]);
                NoiseSurface[i] = NULL;
            }
            SDL_FreeSurface(GlowSurface);
            GlowSurface = NULL;
            initialized_om_surfaces = false;
        }
        if (dust) delete dust;
        if (dust_pts) delete[] dust_pts;
    }
}

void OldMovieLayer::om_init()
{
    ++om_count;

    gv = 0;
    go = 1;
    rx = ry = 0;
    ns = 0;

    if (dust_sprite) {
        // Copy dust sprite to dust
        if (dust) delete dust;
        dust = new AnimationInfo(*dust_sprite);
        dust->visible = true;
    }
    if (dust_pts) delete[] dust_pts;
    dust_pts = new Pt[MAX_DUST_COUNT];

    initialized = true;

    //don't reinitialise existing noise and glow surfaces or scratches
    if (initialized_om_surfaces) return;

    // set up scratches
    for (int i = 0; i < MAX_SCRATCH_COUNT; i++)
        scratches[i].setwindow(width, height);

    // Generate screens of random noise.
    for (int i = 0; i < MAX_NOISE; i++) {
        NoiseSurface[i] = AnimationInfo::allocSurface(width, height);
        SDL_LockSurface(NoiseSurface[i]);
        char* px = (char*) NoiseSurface[i]->pixels;
        const int pt = NoiseSurface[i]->pitch;
        for (int y = 0; y < height; ++y, px += pt) {
            Uint32* row = (Uint32*) px;
            for (int x = 0; x < width; ++x, ++row) {
                const int rm = (rand() % (noise_level + 1)) * 2;
                *row = 0 | (rm << 16) | (rm << 8) | rm;
            }
        }
        SDL_UnlockSurface(NoiseSurface[i]);
    }

    // Generate scanlines of solid greyscale, used for the glow effect.
    GlowSurface = AnimationInfo::allocSurface(width, MAX_GLOW);
    for (SDL_Rect r = { 0, 0, width, 1 }; r.y < MAX_GLOW; r.y++) {
        const int ry = (r.y * 30 / MAX_GLOW) + 4;
        SDL_FillRect(GlowSurface, &r, SDL_MapRGB(GlowSurface->format, ry, ry, ry));
    }
}

// Called once each frame.  Updates effect parameters.
void OldMovieLayer::update()
{
    if (!initialized) return;

    const int last_x = rx, last_y = ry, last_n = ns;
    // Generate blur offset and noise screen randomly.
    // Ensure neither setting is the same two frames running.
    if (blur_level > 0) {
        do {
            rx = rand() % (blur_level + 1) - 1;
            ry = rand() % (blur_level + 1);
        } while (rx == last_x && ry == last_y);
    }
    do {
        ns = rand() % MAX_NOISE;
    } while (ns == last_n);

    // Increment glow; reverse direction if we've reached either limit.
    gv += go;
    if (gv >= 5) { gv = 3; go = -1; }
    if (gv < 0) { gv = 1; go = 1; }

    // Update scratches.
    for (int i=0; i<MAX_SCRATCH_COUNT; i++)
        scratches[i].update(scratch_level);

    // Update dust
    if (dust->num_of_cells > 0) {
        for (int i=0; i<MAX_DUST_COUNT; i++) {
            dust_pts[i].cell = rand() % (dust->num_of_cells);
            dust_pts[i].x = rand() % (width + 10) - 5;
            dust_pts[i].y = rand() % (height + 10) - 5;
        }
    }
}

char *OldMovieLayer::message( const char *message, int &ret_int )
{
    int sprite_no = 0;
    ret_int = 0;
    if (!sprite_info)
        return NULL;

    printf("OldMovieLayer: got message '%s'\n", message);
    if (sscanf(message, "s|%d,%d,%d,%d,%d,%d", 
               &blur_level, &noise_level, &glow_level, 
               &scratch_level, &dust_level, &sprite_no)) {
        if (blur_level < 0) blur_level = 0;
        else if (blur_level > 3) blur_level = 3;
        if (noise_level < 0) noise_level = 0;
        else if (noise_level > 24) noise_level = 24;
        if (glow_level < 0) glow_level = 0;
        else if (glow_level > 24) glow_level = 24;
        if (scratch_level < 0) scratch_level = 0;
        else if (scratch_level > 400) scratch_level = 400;
        if (dust_level < 0) dust_level = 0;
        else if (dust_level > 400) dust_level = 400;
        if ((sprite_no >= 0) && (sprite_no < MAX_SPRITE_NUM))
            dust_sprite = &sprite_info[sprite_no];
        om_init();
    }
    return NULL;
}

// Apply blur effect by averaging two offset copies of a source surface together.
static void BlurOnSurface(SDL_Surface* src, SDL_Surface* dst, SDL_Rect clip, int rx, int ry, int width)
{
    // Calculate clipping bounds to avoid reading outside the source surface.
    const int srcx = clip.x - rx;
    const int srcy = clip.y - ry;
    const int length = ((srcx + clip.w > width) ? (width - srcx) : clip.w) * 4;
    int rows = clip.h;
    const int skipfirstrows = (srcy < 0) ? -srcy : 0;
    const int srcp = src->pitch;
    const int dstp = dst->pitch;

    SDL_LockSurface(src);
    SDL_LockSurface(dst);
    unsigned char* src1px = ((unsigned char*) src->pixels) + srcx * 4 + srcy * srcp;
    unsigned char* src2px = ((unsigned char*) src->pixels) + clip.x * 4 + clip.y * srcp;
    unsigned char* dstpx = ((unsigned char*) dst->pixels) + clip.x * 4 + clip.y * dstp;

    // If the vertical offset is positive, we are reading one copy from (x, -1), so we need to
    // skip the first scanline to avoid reading outside the source surface.
    for (int i=skipfirstrows; i; --i) {
        --rows;
        src1px += srcp;
        src2px += srcp;
        dstpx += dstp;
    }

    // Blend the remaining scanlines.
    while (rows--) {
        ons_gfx::imageFilterMean(src1px, src2px, dstpx, length);
        src1px += srcp;
        src2px += srcp;
        dstpx += dstp;
    }

    // If the horizontal offset is -1, the rightmost column has not been written to.
    // Rectify that by copying it directly from the source image.
    if (rx && (clip.x + clip.w >= width)) {
        Uint32* r = ((Uint32*) src->pixels) + (width - 1) + clip.y * width;
        Uint32* d = ((Uint32*) dst->pixels) + (width - 1) + clip.y * width;
        while (clip.h--) {
            *d = *r;
            d += width;
            r += width;
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dst);

    // If we skipped the first scanlines, rectify that by copying directly from the source image.
    if (skipfirstrows) {
        clip.h = skipfirstrows;
        SDL_BlitSurface(src, &clip, dst, &clip);
    }
}

// Called every time the screen is refreshed.
// Draws the background image with the old-movie effect applied, using the settings adopted at the
// last call to updateOldMovie().
void OldMovieLayer::refresh(SDL_Surface *surface, SDL_Rect &clip)
{
    if (!initialized) return;

    // Blur background.
    // If no offset is applied, we can just copy the given surface directly.
    // If an offset is present, we average the given surface with an offset version

    if ( (rx != 0) || (ry != 0) ) {
        SDL_BlitSurface(surface, &clip, sprite->image_surface, &clip);
        BlurOnSurface(sprite->image_surface, surface, clip, rx, ry, width);
    }

    // Add noise and glow.
    SDL_LockSurface(surface);
    SDL_LockSurface(NoiseSurface[ns]);
    SDL_LockSurface(GlowSurface);
    unsigned char* g = (unsigned char*)GlowSurface->pixels + (gv * glow_level / 4) * GlowSurface->pitch;
    const int sp = surface->pitch;
    if ((clip.x == 0) && (clip.y == 0) && (clip.w == width) && (clip.h == height)) {
        // If no clipping rectangle is defined, we can apply the noise in one go.
        unsigned char* s = (unsigned char*) surface->pixels;
        if (noise_level > 0)
            ons_gfx::imageFilterSubFrom(s, (unsigned char*) NoiseSurface[ns]->pixels, sp * surface->h);
        // Since the glow is stored as a single scanline for each level, we always apply
        // the glow scanline by scanline.
        if (glow_level > 0) {
            for (int i = height; i; --i, s += sp)
                ons_gfx::imageFilterAddTo(s, g, width * 4);
        }
    }
    else {
        // Otherwise we do everything scanline by scanline.
        const int length = clip.w * 4;
        if (noise_level > 0) {
            const int np = NoiseSurface[ns]->pitch;
            unsigned char* s = ((unsigned char*) surface->pixels) + clip.x * 4 + clip.y * sp;
            unsigned char* n = ((unsigned char*) NoiseSurface[ns]->pixels) + clip.x * 4 + clip.y * np;
            for (int i = clip.h; i; --i, s += sp, n += np)
                ons_gfx::imageFilterSubFrom(s, n, length); // subtract noise
        }
        if (glow_level > 0) {
            unsigned char* s = ((unsigned char*) surface->pixels) + clip.x * 4 + clip.y * sp;
            for (int i = clip.h; i; --i, s += sp)
                ons_gfx::imageFilterAddTo(s, g, length); // add glow
        }
    }
    SDL_UnlockSurface(NoiseSurface[ns]);
    SDL_UnlockSurface(GlowSurface);

    // Add scratches.
    if (scratch_level > 0)
        for (int i = 0; i < MAX_SCRATCH_COUNT; i++)
            scratches[i].draw(surface, clip);

    // Add dust specks.
    if (dust && (dust_level > 0)) {
        for (int i=0; i<MAX_DUST_COUNT; i++) {
            if ((rand() & 1023) < dust_level) {
                dust->current_cell = dust_pts[i].cell;
                dust->pos.x = dust_pts[i].x;
                dust->pos.y = dust_pts[i].y;
                drawTaggedSurface( surface, dust, clip );
            }
        }
    }

    // And we're done.
    SDL_UnlockSurface(surface);

}

#endif //BPP16

/*
 * FuruLayer: for snow & hana layer effects (falling stuff)
 * Emulation of Takashi Toyama's "snow.dll" & "hana.dll" NScripter filters
 *
 * C++ coding by Mion, Sep 2008
 */

#define FURU_RATE_COEF 0.2

static float *base_disp_table = NULL;
static int furu_count = 0;
static const float fall_mult[N_FURU_ELEMENTS] = {0.9, 0.7, 0.6};

static void buildBaseDispTable()
{
    if (base_disp_table) return;

    base_disp_table = new float[FURU_AMP_TABLE_SIZE];
    // a = sin? * Z(cos?)
    // Z(z) = rate_z * z +1
    for (int i=0; i<FURU_AMP_TABLE_SIZE; ++i) {
        float rad = (float) i * M_PI * 2 / FURU_AMP_TABLE_SIZE;
        base_disp_table[i] = sin(rad) * (FURU_RATE_COEF * cos(rad) + 1);
    }
}

FuruLayer::FuruLayer( int w, int h, bool animated, BaseReader *br )
{
    width = w;
    height = h;
    tumbling = animated;
    reader = br;

    interval = fall_velocity = wind = amplitude = freq = angle = 0;
    paused = halted = false;
    max_sp_w = 0;
    
    initialized = false;
}

FuruLayer::~FuruLayer(){
    if (initialized) {
        --furu_count;
        if (furu_count == 0) {
            delete[] base_disp_table;
            base_disp_table = NULL;
        }
    }
}

void FuruLayer::furu_init()
{
    for (int i=0; i<N_FURU_ELEMENTS; i++) {
        elements[i].init();
    }
    angle = 0;
    halted = false;
    paused = false;

    buildBaseDispTable();

    ++furu_count;
    initialized = true;
}

void FuruLayer::update()
{
    if (initialized && !paused) {
        if (amplitude != 0)
            angle = (angle - freq + FURU_AMP_TABLE_SIZE) % FURU_AMP_TABLE_SIZE;
        for (int j=0; j<N_FURU_ELEMENTS; ++j) {
            Element *cur = &elements[j];
            int i = cur->pstart;
            const int virt_w = width + max_sp_w;
            while (i != cur->pend) {
                cur->points[i].pt.x = (cur->points[i].pt.x + wind + virt_w) % virt_w;
                cur->points[i].pt.y += cur->fall_speed;
                ++(cur->points[i].pt.cell) %= cur->sprite->num_of_cells;
                ++i %= FURU_ELEMENT_BUFSIZE;
            }
            if (!halted) {
                if (--(cur->frame_cnt) <= 0) {
                    const int tmp = (cur->pend + 1) % FURU_ELEMENT_BUFSIZE;
                    cur->frame_cnt += interval;
                    if (tmp != cur->pstart) {
                        // add a point for this element
                        OscPt *item = &cur->points[cur->pend];
                        item->pt.x = rand() % virt_w;
                        item->pt.y = -(cur->sprite->pos.h);
                        item->pt.type = j;
                        item->pt.cell = 0;
                        item->base_angle = rand() % FURU_AMP_TABLE_SIZE;
                        cur->pend = tmp;
                    }
                }
            }
            while ((cur->pstart != cur->pend) &&
                   (cur->points[cur->pstart].pt.y >= height))
                ++(cur->pstart) %= FURU_ELEMENT_BUFSIZE;
        }
    }
}

static void setStr( char **dst, const char *src, int num=-1 )
{
    if ( *dst ) delete[] *dst;
    *dst = NULL;
    
    if ( src ){
        if (num >= 0){
            *dst = new char[ num + 1 ];
            memcpy( *dst, src, num );
            (*dst)[num] = '\0';
        }
        else{
            *dst = new char[ strlen( src ) + 1];
            strcpy( *dst, src );
        }
    }
}

static SDL_Surface *loadImage( char *file_name, bool *has_alpha, SDL_Surface *surface, BaseReader *br )
{
    if ( !file_name ) return NULL;
    unsigned long length = br->getFileLength( file_name );

    if ( length == 0 )
        return NULL;
    unsigned char *buffer = new unsigned char[length];
    int location;
    br->getFile( file_name, buffer, &location );
    SDL_Surface *tmp = IMG_Load_RW(SDL_RWFromMem( buffer, length ), 1);

    char *ext = strrchr(file_name, '.');
    if ( !tmp && ext && (!strcmp( ext+1, "JPG" ) || !strcmp( ext+1, "jpg" ) ) ){
        fprintf( stderr, " *** force-loading a JPG image [%s]\n", file_name );
        SDL_RWops *src = SDL_RWFromMem( buffer, length );
        tmp = IMG_LoadJPG_RW(src);
        SDL_RWclose(src);
    }
    if ( tmp && has_alpha ) *has_alpha = tmp->format->Amask;

    delete[] buffer;
    if ( !tmp ){
        fprintf( stderr, " *** can't load file [%s] ***\n", file_name );
        return NULL;
    }

    SDL_Surface *ret = SDL_ConvertSurface( tmp, surface->format, SDL_SWSURFACE );
    SDL_FreeSurface( tmp );
    return ret;
}

void FuruLayer::buildAmpTables()
{
    float amp[N_FURU_ELEMENTS];
    amp[0] = (float) amplitude;
    for (int i=1; i<N_FURU_ELEMENTS; ++i)
        amp[i] = amp[i-1] * 0.8;

    for (int i=0; i<N_FURU_ELEMENTS; ++i) {
        Element *cur = &elements[i];
        if (!cur->amp_table)
            cur->amp_table = new int[FURU_AMP_TABLE_SIZE];
        for (int j=0; j<FURU_AMP_TABLE_SIZE; ++j)
            cur->amp_table[j] = (int) (amp[i] * base_disp_table[j]);
    }
}

void FuruLayer::validate_params()
{
    const int half_wx = width / 2;

    if (interval < 1) interval = 1;
    else if (interval > 10000) interval = 10000;
    if (fall_velocity < 1) fall_velocity = 1;
    else if (fall_velocity > height) fall_velocity = height;
    for (int i=0; i<N_FURU_ELEMENTS; i++)
        elements[i].fall_speed = (int)(fall_mult[i] * (fall_velocity+1));
    if (wind < -half_wx) wind = -half_wx;
    else if (wind > half_wx) wind = half_wx;
    if (amplitude < 0) amplitude = 0;
    else if (amplitude > half_wx) amplitude = half_wx;
    if (amplitude != 0) buildAmpTables();
    if (freq < 0) freq = 0;
    else if (freq > 359) freq = 359;
    //adjust the freq to range 0-FURU_AMP_TABLE_SIZE-1
    freq = freq * FURU_AMP_TABLE_SIZE / 360;
}

char *FuruLayer::message( const char *message, int &ret_int )
{
    int num_cells[3], tmp[5];
    char buf[3][128];

    char *ret_str = NULL;
    ret_int = 0;

    if (!sprite)
        return NULL;

    //printf("FuruLayer: got message '%s'\n", message);
//Image loading
    if (!strncmp(message, "i|", 2)) {
        max_sp_w = 0;
        SDL_Surface *ref_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, 1, 1, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );
        if (tumbling) {
        // "Hana"
            if (sscanf(message, "i|%d,%d,%d,%d,%d,%d",
                       &tmp[0], &num_cells[0],
                       &tmp[1], &num_cells[1],
                       &tmp[2], &num_cells[2])) {
                for (int i=0; i<3; i++) {
                    elements[i].setSprite(new AnimationInfo(sprite_info[tmp[i]]));
                    elements[i].sprite->num_of_cells = num_cells[i];
                    if (elements[i].sprite->pos.w > max_sp_w)
                        max_sp_w = elements[i].sprite->pos.w;
                }
            } else
            if (sscanf(message, "i|%120[^,],%d,%120[^,],%d,%120[^,],%d",
                              &buf[0][0], &num_cells[0],
                              &buf[1][0], &num_cells[1],
                              &buf[2][0], &num_cells[2])) {
                for (int i=0; i<3; i++) {
                    bool has_alpha = false;
                    SDL_Surface *img = loadImage( &buf[i][0], &has_alpha, ref_surface, reader );
                    AnimationInfo *anim = new AnimationInfo();
                    anim->num_of_cells = num_cells[i];
                    anim->duration_list = new int[ anim->num_of_cells ];
                    for (int j=anim->num_of_cells - 1; j>=0; --j )
                        anim->duration_list[j] = 0;
                    anim->loop_mode = 3; // not animatable
                    anim->trans_mode = AnimationInfo::TRANS_TOPLEFT;
                    setStr( &anim->file_name, &buf[i][0] );
                    anim->setImage(anim->setupImageAlpha(img, NULL, has_alpha));
                    elements[i].setSprite(anim);
                    if (anim->pos.w > max_sp_w)
                        max_sp_w = anim->pos.w;
                }
            }
        } else {
        // "Snow"
            if (sscanf(message, "i|%d,%d,%d", 
                       &tmp[0], &tmp[1], &tmp[2])) {
                for (int i=0; i<3; i++) {
                    elements[i].setSprite(new AnimationInfo(sprite_info[tmp[i]]));
                    if (elements[i].sprite->pos.w > max_sp_w)
                        max_sp_w = elements[i].sprite->pos.w;
                }
            } else if (sscanf(message, "i|%[^,],%[^,],%[^,]",
                              &buf[0][0], &buf[1][0], &buf[2][0])) {
                for (int i=0; i<3; i++) {
                    Uint32 firstpix = 0;
                    bool has_alpha = false;
                    SDL_Surface *img = loadImage( &buf[i][0], &has_alpha, ref_surface, reader );
                    AnimationInfo *anim = new AnimationInfo();
                    anim->num_of_cells = 1;
                    SDL_LockSurface( img );
                    firstpix = *((Uint32*) img->pixels) & ~(img->format->Amask);
                    if (firstpix > 0) {
                        anim->trans_mode = AnimationInfo::TRANS_TOPLEFT;
                    } else {
                        // if first pix is black, this is an "additive" sprite
                        anim->trans_mode = AnimationInfo::TRANS_COPY;
                        anim->blending_mode = AnimationInfo::BLEND_ADD;
                    }
                    SDL_UnlockSurface( img );
                    setStr( &anim->file_name, &buf[i][0] );
                    anim->setImage(anim->setupImageAlpha(img, NULL, has_alpha));
                    elements[i].setSprite(anim);
                    if (anim->pos.w > max_sp_w)
                        max_sp_w = anim->pos.w;
                }
            }
        }
        SDL_FreeSurface(ref_surface);
//Set Parameters
    } else if (sscanf(message, "s|%d,%d,%d,%d,%d", 
                      &interval, &fall_velocity, &wind, 
                      &amplitude, &freq)) {
        furu_init();
        validate_params();
//Transition (adjust) Parameters
    } else if (sscanf(message, "t|%d,%d,%d,%d,%d", 
                      &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4])) {
        interval += tmp[0];
        fall_velocity += tmp[1];
        wind += tmp[2];
        amplitude += tmp[3];
        freq += tmp[4];
        validate_params();
//Fill Screen w/Elements
    } else if (!strcmp(message, "f")) {
        if (initialized) {
            for (int j=0; j<N_FURU_ELEMENTS; j++) {
                Element *cur = &elements[j];
                int y = 0;
                while (y < height) {
                    const int tmp = (cur->pend + 1) % FURU_ELEMENT_BUFSIZE;
                    if (tmp != cur->pstart) {
                    // add a point for each element
                        OscPt *item = &cur->points[cur->pend];
                        item->pt.x = rand() % (width + max_sp_w);
                        item->pt.y = y;
                        item->pt.type = j;
                        item->pt.cell = rand() % cur->sprite->num_of_cells;
                        item->base_angle = rand() % FURU_AMP_TABLE_SIZE;
                        cur->pend = tmp;
                    }
                    y += interval * cur->fall_speed;
                }
            }
        }
//Get Parameters
    } else if (!strcmp(message, "g")) {
        ret_int = paused ? 1 : 0;
        sprintf(&buf[0][0], "s|%d,%d,%d,%d,%d", interval, fall_velocity,
                wind, amplitude, (freq * 360 / FURU_AMP_TABLE_SIZE));
        setStr( &ret_str, &buf[0][0]);
//Halt adding new elements
    } else if (!strcmp(message, "h")) {
        halted = true;
//Get number of elements displayed
    } else if (!strcmp(message, "n")) {
        for (int i=0; i<N_FURU_ELEMENTS; i++)
            ret_int += (elements[i].pend - elements[i].pstart + FURU_ELEMENT_BUFSIZE)
                % FURU_ELEMENT_BUFSIZE;
//Pause
    } else if (!strcmp(message, "p")) {
        paused = true;
//Restart
    } else if (!strcmp(message, "r")) {
        paused = false;
//eXtinguish
    } else if (!strcmp(message, "x")) {
        for (int i=0; i<N_FURU_ELEMENTS; i++)
            elements[i].clear();
        initialized = false;
    }
    return ret_str;
}

void FuruLayer::refresh(SDL_Surface *surface, SDL_Rect &clip)
{
    if (initialized) {
        const int virt_w = width + max_sp_w;
        for (int j=0; j<N_FURU_ELEMENTS; j++) {
            Element *cur = &elements[j];
            if (cur->sprite) {
                cur->sprite->visible = true;
                const int n = (cur->pend - cur->pstart + FURU_ELEMENT_BUFSIZE) % FURU_ELEMENT_BUFSIZE;
                int p = cur->pstart;
                if (amplitude == 0) {
                    //no need to mess with angles if no displacement
                    for (int i=n; i>0; i--) {
                        OscPt *curpt = &cur->points[p];
                        ++p %= FURU_ELEMENT_BUFSIZE;
                        cur->sprite->current_cell = curpt->pt.cell;
                        cur->sprite->pos.x = ((curpt->pt.x + virt_w) % virt_w) - max_sp_w;
                        cur->sprite->pos.y = curpt->pt.y;
                        drawTaggedSurface( surface, cur->sprite, clip );
                    }
                } else {
                    for (int i=n; i>0; i--) {
                        OscPt *curpt = &cur->points[p];
                        ++p %= FURU_ELEMENT_BUFSIZE;
                        const int disp_angle = (angle + curpt->base_angle + FURU_AMP_TABLE_SIZE) % FURU_AMP_TABLE_SIZE;
                        cur->sprite->current_cell = curpt->pt.cell;
                        cur->sprite->pos.x = ((curpt->pt.x + cur->amp_table[disp_angle] + virt_w) % virt_w) - max_sp_w;
                        cur->sprite->pos.y = curpt->pt.y;
                        drawTaggedSurface( surface, cur->sprite, clip );
                    }
                }
            }
        }
    }
}

#endif // ndef NO_LAYER_EFFECTS
