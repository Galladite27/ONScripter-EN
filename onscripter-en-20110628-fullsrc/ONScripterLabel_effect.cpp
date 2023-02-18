/* -*- C++ -*-
 *
 *  ONScripterLabel_effect.cpp - Effect executer of ONScripter-EN
 *
 *  Copyright (c) 2001-2009 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2008-2011 "Uncle" Mion Sonozaki
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
 *  Uses emulation of Takashi Toyama's "cascade.dll", "whirl.dll",
 *  "trvswave.dll", and "breakup.dll" NScripter plugin effects.
 */

#include "ONScripterLabel.h"

#define EFFECT_STRIPE_WIDTH ExpandPos(16)
#define EFFECT_STRIPE_CURTAIN_WIDTH ExpandPos(24)
#define EFFECT_QUAKE_AMP ExpandPos(12)

static char *dll=NULL, *params=NULL; //for dll-based effects

bool ONScripterLabel::setEffect( EffectLink *effect, bool generate_effect_dst, bool update_backup_surface )
{
    if ( effect->effect == 0 ) return true;

    if (update_backup_surface)
        refreshSurface(backup_surface, &dirty_rect.bounding_box, REFRESH_NORMAL_MODE);
    
    int effect_no = effect->effect;

    SDL_BlitSurface( accumulation_surface, NULL, effect_src_surface, NULL );

    if (generate_effect_dst){
        int refresh_mode = refreshMode();
        if (update_backup_surface && refresh_mode == REFRESH_NORMAL_MODE){
            SDL_BlitSurface( backup_surface, &dirty_rect.bounding_box, effect_dst_surface, &dirty_rect.bounding_box );
        }
        else{
            if (effect_no == 1)
                refreshSurface( effect_dst_surface, &dirty_rect.bounding_box, refresh_mode );
            else
                refreshSurface( effect_dst_surface, NULL, refresh_mode );
        }
    }
    
    effect_counter = 0;
    effect_start_time_old = SDL_GetTicks();
    effect_duration = effect->duration;
    if (ctrl_pressed_status || skip_mode & SKIP_NORMAL) {
        // shorten the duration of effects while skipping
        if ( effect_cut_flag ) {
            effect_duration = 0;
            return false; //don't parse effects if effectcut skip
        } else if (effect_duration > 100) {
            effect_duration = effect_duration / 10;
        } else if (effect_duration > 10) {
            effect_duration = 10;
        } else {
            effect_duration = 1;
        }
    } else if (effectspeed == EFFECTSPEED_INSTANT) {
        effect_duration = 0;
        return false; //don't parse effects if instant speed
    } else if (effectspeed == EFFECTSPEED_QUICKER) {
        effect_duration = effect_duration / 2;
        if (effect_duration <= 0)
            effect_duration = 1;
    }

    /* Load mask image */
    if ( effect_no == 15 || effect_no == 18 ){
        if ( !effect->anim.image_surface ){
            parseTaggedString( &effect->anim, true );
#ifdef RCA_SCALE
            setupAnimationInfo( &effect->anim, NULL, scr_stretch_x, scr_stretch_y );
#else
            setupAnimationInfo( &effect->anim );
#endif
        }
    }
    if ( effect_no == 11 || effect_no == 12 || effect_no == 13 || effect_no == 14 ||
         effect_no == 16 || effect_no == 17 )
        dirty_rect.fill( screen_width, screen_height );

    dll = params = NULL;
    if (effect_no == 99) { // dll-based
        dll = effect->anim.image_name;
        if (dll != NULL) { //just in case no dll is given
            if (debug_level > 0)
                printf("dll effect: Got dll/params '%s'\n", dll);

            params = dll;
            while (*params != 0 && *params != '/') params++;
            if (*params == '/') params++;

            if (!strncmp(dll, "whirl.dll", 9)) {
                buildSinTable();
                buildCosTable();
                buildWhirlTable();
                dirty_rect.fill( screen_width, screen_height );
            }
            else if (!strncmp(dll, "trvswave.dll", 12)) {
                buildSinTable();
                dirty_rect.fill( screen_width, screen_height );
            }
            else if (!strncmp(dll, "breakup.dll", 11)) {
                initBreakup(params);
                dirty_rect.fill( screen_width, screen_height );
            }
            else {
                dirty_rect.fill( screen_width, screen_height );
            }
        }
    }

    return false;
}

bool ONScripterLabel::doEffect( EffectLink *effect, bool clear_dirty_region )
{
    bool first_time = (effect_counter == 0);

    effect_start_time = SDL_GetTicks();

    effect_timer_resolution = effect_start_time - effect_start_time_old;
    effect_start_time_old = effect_start_time;

    int effect_no = effect->effect;
    if (first_time) {
        if ( (effect_cut_flag &&
              ( ctrl_pressed_status || skip_mode & SKIP_NORMAL )) ||
             (effectspeed == EFFECTSPEED_INSTANT) )
            effect_no = 1;
    }

    skip_effect = false;

    int i, amp;
    int width, width2;
    int height, height2;
    SDL_Rect src_rect={0, 0, screen_width, screen_height};
    SDL_Rect dst_rect={0, 0, screen_width, screen_height};
    SDL_Rect quake_rect={0, 0, screen_width, screen_height};

    /* ---------------------------------------- */
    /* Execute effect */
    if (debug_level > 0 && first_time)
        printf("Effect number %d, %d ms\n", effect_no, effect_duration );

    bool not_implemented = false;
    switch ( effect_no ){
      case 0: // Instant display
      case 1: // Instant display
        //drawEffect( &src_rect, &src_rect, effect_dst_surface );
        break;

      case 2: // Left shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect_duration;
        for ( i=0 ; i<screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 3: // Right shutter
        width = EFFECT_STRIPE_WIDTH * effect_counter / effect_duration;
        for ( i=1 ; i<=screen_width/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = i * EFFECT_STRIPE_WIDTH - width - 1;
            src_rect.y = 0;
            src_rect.w = width;
            src_rect.h = screen_height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 4: // Top shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect_duration;
        for ( i=0 ; i<screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 5: // Bottom shutter
        height = EFFECT_STRIPE_WIDTH * effect_counter / effect_duration;
        for ( i=1 ; i<=screen_height/EFFECT_STRIPE_WIDTH ; i++ ){
            src_rect.x = 0;
            src_rect.y = i * EFFECT_STRIPE_WIDTH - height - 1;
            src_rect.w = screen_width;
            src_rect.h = height;
            drawEffect(&src_rect, &src_rect, effect_dst_surface);
        }
        break;

      case 6: // Left curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect_duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                src_rect.x = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 7: // Right curtain
        width = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect_duration;
        for ( i=0 ; i<=screen_width/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            width2 = width - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_width;
            if ( width2 >= 0 ){
                if ( width2 > EFFECT_STRIPE_CURTAIN_WIDTH ) width2 = EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.x = screen_width - i * EFFECT_STRIPE_CURTAIN_WIDTH - width2;
                src_rect.y = 0;
                src_rect.w = width2;
                src_rect.h = screen_height;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 8: // Top curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect_duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = i * EFFECT_STRIPE_CURTAIN_WIDTH;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      case 9: // Bottom curtain
        height = EFFECT_STRIPE_CURTAIN_WIDTH * effect_counter * 2 / effect_duration;
        for ( i=0 ; i<=screen_height/EFFECT_STRIPE_CURTAIN_WIDTH ; i++ ){
            height2 = height - EFFECT_STRIPE_CURTAIN_WIDTH * EFFECT_STRIPE_CURTAIN_WIDTH * i / screen_height;
            if ( height2 >= 0 ){
                src_rect.x = 0;
                src_rect.y = screen_height - i * EFFECT_STRIPE_CURTAIN_WIDTH - height2;
                src_rect.w = screen_width;
                src_rect.h = height2;
                drawEffect(&src_rect, &src_rect, effect_dst_surface);
            }
        }
        break;

      default:
        not_implemented = true;
        if (first_time) {
            snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                     "effect No. %d not implemented; substituting crossfade",
                     effect_no);
            errorAndCont(script_h.errbuf);
        }

      case 10: // Cross fade
        height = 256 * effect_counter / effect_duration;
        effectBlend( NULL, ALPHA_BLEND_CONST, height, &dirty_rect.bounding_box );
        break;

      case 11: // Left scroll
        width = screen_width * effect_counter / effect_duration;
        src_rect.x = 0;
        dst_rect.x = width;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = screen_width - width - 1;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 12: // Right scroll
        width = screen_width * effect_counter / effect_duration;
        src_rect.x = width;
        dst_rect.x = 0;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width - width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = 0;
        dst_rect.x = screen_width - width - 1;
        src_rect.y = dst_rect.y = 0;
        src_rect.w = dst_rect.w = width;
        src_rect.h = dst_rect.h = screen_height;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 13: // Top scroll
        width = screen_height * effect_counter / effect_duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = width;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = screen_height - width - 1;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 14: // Bottom scroll
        width = screen_height * effect_counter / effect_duration;
        src_rect.x = dst_rect.x = 0;
        src_rect.y = width;
        dst_rect.y = 0;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = screen_height - width;
        drawEffect(&dst_rect, &src_rect, effect_src_surface);

        src_rect.x = dst_rect.x = 0;
        src_rect.y = 0;
        dst_rect.y = screen_height - width - 1;
        src_rect.w = dst_rect.w = screen_width;
        src_rect.h = dst_rect.h = width;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case 15: // Fade with mask
        effectBlend( effect->anim.image_surface, ALPHA_BLEND_FADE_MASK, 256 * effect_counter / effect_duration, &dirty_rect.bounding_box );
        break;

      case 16: // Mosaic out
        generateMosaic( effect_src_surface, 5 - 6 * effect_counter / effect_duration );
        break;

      case 17: // Mosaic in
        generateMosaic( effect_dst_surface, 6 * effect_counter / effect_duration );
        break;

      case 18: // Cross fade with mask
        effectBlend( effect->anim.image_surface, ALPHA_BLEND_CROSSFADE_MASK, 256 * effect_counter * 2 / effect_duration, &dirty_rect.bounding_box );
        break;

      case (MAX_EFFECT_NUM + 0): // quakey
        if ( effect_timer_resolution > effect_duration / 4 / effect->no )
            effect_timer_resolution = effect_duration / 4 / effect->no;
        amp = (Sint16)(sin(M_PI * 2.0 * effect->no * effect_counter / effect_duration) *
                       EFFECT_QUAKE_AMP * effect->no * (effect_duration -  effect_counter) / effect_duration);
        dst_rect.x = 0;
        dst_rect.y = amp;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);

        if (amp >= 0){
            quake_rect.y = 0;
            quake_rect.h = amp;
        }
        else{
            quake_rect.y = screen_height + amp;
            quake_rect.h = -amp;
        }
        SDL_FillRect( accumulation_surface, &quake_rect, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
        break;

      case (MAX_EFFECT_NUM + 1): // quakex
        if ( effect_timer_resolution > effect_duration / 4 / effect->no )
            effect_timer_resolution = effect_duration / 4 / effect->no;
        amp = (Sint16)(sin(M_PI * 2.0 * effect->no * effect_counter / effect_duration) *
                       EFFECT_QUAKE_AMP * effect->no * (effect_duration -  effect_counter) / effect_duration);
        dst_rect.x = amp;
        dst_rect.y = 0;
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);

        if (amp >= 0){
            quake_rect.x = 0;
            quake_rect.w = amp;
        }
        else{
            quake_rect.x = screen_width + amp;
            quake_rect.w = -amp;
        }
        SDL_FillRect( accumulation_surface, &quake_rect, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
        break;

      case (MAX_EFFECT_NUM + 2 ): // quake
        dst_rect.x = effect->no*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        dst_rect.y = effect->no*((int)(3.0*rand()/(RAND_MAX+1.0)) - 1) * 2;
        SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
        drawEffect(&dst_rect, &src_rect, effect_dst_surface);
        break;

      case (MAX_EFFECT_NUM + 3 ): // flushout
        if (effect_counter > 0){
            width = 30 * effect_counter / effect_duration;
            height = 30 * (effect_counter + effect_timer_resolution) / effect_duration;
            if (height > width){
                doFlushout(height);
                effectBlend( NULL, ALPHA_BLEND_CONST, effect_counter * 256 / effect_duration, &dirty_rect.bounding_box, effect_tmp_surface );
            }
        }
        break;

      case 99: // dll-based
        if (dll != NULL) {
            if (!strncmp(dll, "cascade.dll", 11)) {
                effectCascade(params, effect_duration);
            } else if (!strncmp(dll, "whirl.dll", 9)) {
                effectWhirl(params, effect_duration);
            } else if (!strncmp(dll, "trvswave.dll", 12)) {
                effectTrvswave(params, effect_duration);
            } else if (!strncmp(dll, "breakup.dll", 11)) {
                effectBreakup(params, effect_duration);
            } else {
                not_implemented = true;
                if (first_time) {
                    snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                             "dll effect '%s' (%d) not implemented; substituting crossfade",
                             dll, effect_no);
                    errorAndCont(script_h.errbuf);
                }
            }
        } else { //just in case no dll is given
            not_implemented = true;
            if (first_time) {
                snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                         "no dll provided for effect %d; substituting crossfade",
                         effect_no);
                errorAndCont(script_h.errbuf);
            }
        }
        if (not_implemented) {
            // do crossfade
            height = 256 * effect_counter / effect_duration;
            effectBlend( NULL, ALPHA_BLEND_CONST, height, &dirty_rect.bounding_box );
        }
        break;
    }

    if (debug_level > 1)
        printf("\teffect count %d / dur %d\n", effect_counter, effect_duration);

    effect_counter += effect_timer_resolution;

    //check for events before drawing
    event_mode = IDLE_EVENT_MODE;
    event_mode |= WAIT_NO_ANIM_MODE;
    if (effectskip_flag) {
        event_mode |= WAIT_INPUT_MODE;
    }
    waitEvent(0);
    event_mode &= ~(WAIT_NO_ANIM_MODE | WAIT_INPUT_MODE);

    if ( effect_counter < effect_duration && effect_no != 1 ){
        if ( effect_no != 0 ) flush( REFRESH_NONE_MODE, NULL, false );

        if (effectskip_flag && skip_effect)
            effect_counter = effect_duration;

        return true;
    }
    else {
        //last call
        SDL_BlitSurface(effect_dst_surface, &dirty_rect.bounding_box,
                        accumulation_surface, &dirty_rect.bounding_box);

        if (effect_no != 0)
            flush(REFRESH_NONE_MODE, NULL, clear_dirty_region);
        if (effect_no == 1)
            effect_counter = 0;
        else if ((effect_no == 99) && (dll != NULL)){
            dll = params = NULL;
        }

        display_mode &= ~DISPLAY_MODE_UPDATED;

        if (effect_blank != 0 && effect_counter != 0) {
            event_mode = WAIT_TIMER_MODE;
            if ( ctrl_pressed_status || (skip_mode & SKIP_TO_WAIT) )
                waitEvent(1); //allow a moment to detect ctrl unpress, if any
            else
                waitEvent(effect_blank);
        }
        event_mode = IDLE_EVENT_MODE;

        return false;
    }
}

void ONScripterLabel::drawEffect(SDL_Rect *dst_rect, SDL_Rect *src_rect, SDL_Surface *surface)
{
    SDL_Rect clipped_rect;
    if (AnimationInfo::doClipping(dst_rect, &dirty_rect.bounding_box, &clipped_rect)) return;
    if (src_rect != dst_rect){
        src_rect->x += clipped_rect.x;
        src_rect->y += clipped_rect.y;
        src_rect->w = clipped_rect.w;
        src_rect->h = clipped_rect.h;
    }

    SDL_BlitSurface(surface, src_rect, accumulation_surface, dst_rect);
}

void ONScripterLabel::generateMosaic( SDL_Surface *src_surface, int level )
{
    int i, j, ii, jj;
    int width = 160;
    for ( i=0 ; i<level ; i++ ) width >>= 1;

#ifdef BPP16
    int total_width = accumulation_surface->pitch / 2;
#else
    int total_width = accumulation_surface->pitch / 4;
#endif
    SDL_LockSurface( src_surface );
    SDL_LockSurface( accumulation_surface );
    ONSBuf *src_buffer = (ONSBuf *)src_surface->pixels;

    for ( i=screen_height-1 ; i>=0 ; i-=width ){
        for ( j=0 ; j<screen_width ; j+=width ){
            ONSBuf p = src_buffer[ i*total_width+j ];
            ONSBuf *dst_buffer = (ONSBuf *)accumulation_surface->pixels + i*total_width + j;

            int height2 = width;
            if (i+1-width < 0) height2 = i+1;
            int width2 = width;
            if (j+width > screen_width) width2 = screen_width - j;
            for ( ii=height2 ; ii!=0 ; ii-- ){
                for ( jj=width2 ; jj!=0 ; jj-- ){
                    *dst_buffer++ = p;
                }
                dst_buffer -= total_width + width2;
            }
        }
    }

    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockSurface( src_surface );
}

// An interesting builtin effect... this causes a semi-transparent
// time-lapse expansion of the image, producing a sort of "hyperspace" effect
void ONScripterLabel::doFlushout( int level )
{
    int i, j, ii, jj;

#ifdef BPP16
    int total_width = accumulation_surface->pitch / 2;
#else
    int total_width = accumulation_surface->pitch / 4;
#endif
    SDL_LockSurface( effect_src_surface );
    SDL_LockSurface( accumulation_surface );
    ONSBuf *src_buffer = (ONSBuf *)effect_src_surface->pixels;

    ONSBuf *dst_buffer = (ONSBuf *)accumulation_surface->pixels;
    const int factor = 32;
    const int maxlevel = 30;
    level += factor - maxlevel;
    const int y_offset = screen_height*level/factor/2;
    const int x_offset = screen_width*level/factor/2;
    for ( i=0 ; i<screen_height ; i++ ){
        ii = i*(factor-level)/factor + y_offset;
        for ( j=0 ; j<screen_width ; j++ ){
            jj = j*(factor-level)/factor + x_offset;
            *dst_buffer++ = src_buffer[ ii*total_width+jj ];
        }
    }

    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockSurface( effect_src_surface );
    effectBlend( NULL, ALPHA_BLEND_CONST, 64, &dirty_rect.bounding_box, effect_tmp_surface, accumulation_surface, effect_tmp_surface );
}
