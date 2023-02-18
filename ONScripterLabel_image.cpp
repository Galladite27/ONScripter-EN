/* -*- C++ -*-
 * 
 *  ONScripterLabel_image.cpp - Image processing in ONScripter-EN
 *
 *  Copyright (c) 2001-2011 Ogapee. All rights reserved.
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

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#include "ONScripterLabel.h"
#include <new>
#include <cstdio>

#include "graphics_blend.h"

SDL_Surface *ONScripterLabel::loadImage( char *filename, bool *has_alpha )
{
    if ( !filename ) return NULL;

    SDL_Surface *tmp = NULL;
    int location = BaseReader::ARCHIVE_TYPE_NONE;

    if (filename[0] == '>')
        tmp = createRectangleSurface(filename);
    else if (filename[0] != '*') // layers begin with *
        tmp = createSurfaceFromFile(filename, &location);
    if (tmp == NULL) return NULL;

    bool has_colorkey = false;
    Uint32 colorkey = 0;

    if ( has_alpha ){
        *has_alpha = (tmp->format->Amask != 0);
        if (!(*has_alpha) && (tmp->flags & SDL_SRCCOLORKEY)){
            has_colorkey = true;
            colorkey = tmp->format->colorkey;
            if (tmp->format->palette){
                //palette will be converted to RGBA, so don't do colorkey check
                has_colorkey = false;
            }
            *has_alpha = true;
        }
    }

    SDL_Surface *ret = SDL_ConvertSurface( tmp, image_surface->format, SDL_SWSURFACE );
    SDL_FreeSurface( tmp );

    //  A PNG image may contain an alpha channel, which complicates
    // handling loaded images when the ":a" alphablend tag is used,
    // since the standard method was to assume the right half of the image
    // contains an alpha data mask for the left half.
    //  The current default behavior is to use the PNG image's alpha
    // channel if available, and only process for an old-style mask
    // when no alpha channel was provided.
    // However, this could cause problems running older NScr games
    // which have PNG images containing old-style masks but also an
    // opaque alpha channel.
    //  Therefore, we provide a hack, set with the --detect-png-nscmask
    // command-line option, to auto-detect if a PNG image is likely to
    // have an old-style mask.  We assume that an old-style mask is intended
    // if the image either has no alpha channel, or the alpha channel it has
    // is completely opaque.  (Note that this used to be the default
    // behavior for onscripter-en.)
    //  Note that using the --force-png-nscmask option will always assume
    // old-style masks, while --force-png-alpha will produce the current
    // default behavior.
    if ((png_mask_type != PNG_MASK_USE_ALPHA) &&
        has_alpha && *has_alpha) {
        if (png_mask_type == PNG_MASK_USE_NSCRIPTER)
            *has_alpha = false;
        else if (png_mask_type == PNG_MASK_AUTODETECT) {
            SDL_LockSurface(ret);
            const Uint32 aval = *(Uint32*)ret->pixels & ret->format->Amask;
            if (aval != ret->format->Amask) goto breakalpha;
            *has_alpha = false;
            for (int y=0; y<ret->h; ++y) {
                Uint32* pixbuf = (Uint32*)((char*)ret->pixels + y * ret->pitch);
                for (int x=ret->w; x>0; --x, ++pixbuf) {
                    // Resolving ambiguity per Tatu's patch, 20081118.
                    // I note that this technically changes the meaning of the
                    // code, since != is higher-precedence than &, but this
                    // version is obviously what I intended when I wrote this.
                    // Has this been broken all along?  :/  -- Haeleth
                    if ((*pixbuf & ret->format->Amask) != aval) {
                        *has_alpha = true;
                        goto breakalpha;
                    }
                }
            }
          breakalpha:
            if (!*has_alpha && has_colorkey) {
                // has a colorkey, so run a match against rgb values
                const Uint32 aval = colorkey & ~(ret->format->Amask);
                if (aval == (*(Uint32*)ret->pixels & ~(ret->format->Amask)))
                    goto breakkey;
                *has_alpha = false;
                for (int y=0; y<ret->h; ++y) {
                    Uint32* pixbuf = (Uint32*)((char*)ret->pixels + y * ret->pitch);
                    for (int x=ret->w; x>0; --x, ++pixbuf) {
                        if ((*pixbuf & ~(ret->format->Amask)) == aval) {
                            *has_alpha = true;
                            goto breakkey;
                        }
                    }
                }
            }
          breakkey:
            SDL_UnlockSurface(ret);
        }
    }
    
    return ret;
}

SDL_Surface *ONScripterLabel::createRectangleSurface(char *filename)
{
    int c=1, w=0, h=0;
    while (filename[c] != 0x0a && filename[c] != 0x00){
        if (filename[c] >= '0' && filename[c] <= '9')
            w = w*10 + filename[c]-'0';
        if (filename[c] == ','){
            c++;
            break;
        }
        c++;
    }

    while (filename[c] != 0x0a && filename[c] != 0x00){
        if (filename[c] >= '0' && filename[c] <= '9')
            h = h*10 + filename[c]-'0';
        if (filename[c] == ','){
            c++;
            break;
        }
        c++;
    }
        
    while (filename[c] == ' ' || filename[c] == '\t') c++;
    int n=0, c2 = c;
    while(filename[c] == '#'){
        uchar3 col;
        readColor(&col, filename+c);
        n++;
        c += 7;
        while (filename[c] == ' ' || filename[c] == '\t') c++;
    }

    SDL_PixelFormat *fmt = image_surface->format;
    SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
                                            fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    c = c2;
    for (int i=0 ; i<n ; i++){
        uchar3 col;
        readColor(&col, filename+c);
        c += 7;
        while (filename[c] == ' ' || filename[c] == '\t') c++;
        
        SDL_Rect rect;
        rect.x = w*i/n;
        rect.y = 0;
        rect.w = w*(i+1)/n - rect.x;
        if (i == n-1) rect.w = w - rect.x;
        rect.h = h;
        SDL_FillRect(tmp, &rect, SDL_MapRGBA( accumulation_surface->format, col[0], col[1], col[2], 0xff));
    }
    
    return tmp;
}

SDL_Surface *ONScripterLabel::createSurfaceFromFile(char *filename, int *location)
{
    char* alt_buffer = 0;
    unsigned long length = script_h.cBR->getFileLength( filename );

    if (length == 0) {
        alt_buffer = new char[strlen(filename) + strlen(script_h.save_path) + 1];
        sprintf(alt_buffer, "%s%s", script_h.save_path, filename);
        char* si = alt_buffer;
        do { if (*si == '\\') *si = DELIMITER; } while (*(++si));
        FILE* fp = std::fopen(alt_buffer, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            length = ftell(fp);
            fclose(fp);
        }
        else delete[] alt_buffer;
    }

    if ( length == 0 ){
        //don't complain about missing cursors
        if (strcmp(filename, "uoncur.bmp" ) &&
            strcmp(filename, "uoffcur.bmp") &&
            strcmp(filename, "doncur.bmp" ) &&
            strcmp(filename, "doffcur.bmp") &&
            strcmp(filename, "cursor0.bmp") &&
            strcmp(filename, "cursor1.bmp")) {
            snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                     "can't find file [%s]", filename);
            errorAndCont( script_h.errbuf, NULL, "I/O Issue" );
        }
        return NULL;
    }
    if ( filelog_flag )
        script_h.findAndAddLog( script_h.log_info[ScriptHandler::FILE_LOG], filename, true );
    //printf(" ... loading %s length %ld\n", filename, length );

    mean_size_of_loaded_images += length*6/5; // reserve 20% larger size
    num_loaded_images++;
    if (tmp_image_buf_length < mean_size_of_loaded_images/num_loaded_images){
        tmp_image_buf_length = mean_size_of_loaded_images/num_loaded_images;
        if (tmp_image_buf) delete[] tmp_image_buf;
        tmp_image_buf = NULL;
    }

    unsigned char *buffer = NULL;
    if (length > tmp_image_buf_length){
        buffer = new(std::nothrow) unsigned char[length];
        if (buffer == NULL) {
            snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                     "failed to load image file [%s] (%lu bytes)",
                     filename, length);
            errorAndCont( script_h.errbuf, "unable to allocate buffer", "Memory Issue" );
            return NULL;
        }
    }
    else{
        if (!tmp_image_buf) tmp_image_buf = new unsigned char[tmp_image_buf_length];
        buffer = tmp_image_buf;
    }

    if (!alt_buffer) {
        script_h.cBR->getFile( filename, buffer, location );
    }
    else {
        FILE* fp;
        if ((fp = std::fopen(alt_buffer, "rb"))) {
            if (fread(buffer, 1, length, fp) != length)
                fprintf(stderr, "Warning: error reading from %s\n", alt_buffer);
            fclose(fp);
        }
        delete[] alt_buffer;
    }
    char *ext = strrchr(filename, '.');

    SDL_RWops *src = SDL_RWFromMem(buffer, length);
    SDL_Surface *tmp = IMG_Load_RW(src, 0);
    if (!tmp && ext && (!strcmp(ext+1, "JPG") || !strcmp(ext+1, "jpg"))){
        fprintf(stderr, " *** force-loading a JPG image [%s]\n", filename);
        tmp = IMG_LoadJPG_RW(src);

    }
    SDL_RWclose(src);

    if (buffer != tmp_image_buf) delete[] buffer;
    if (!tmp)
        fprintf( stderr, " *** can't load file [%s]: %s ***\n", filename, IMG_GetError() );

    return tmp;
}


// effectBlend
// dst: accumulation_surface
// src1: effect_src_surface
// src2: effect_dst_surface
void ONScripterLabel::effectBlend( SDL_Surface *mask_surface, int trans_mode,
                                   Uint32 mask_value, SDL_Rect *clip,
                                   SDL_Surface *src1, SDL_Surface *src2, SDL_Surface *dst )
{
    SDL_Rect rect = {0, 0, screen_width, screen_height};

    if (src1 == NULL)
        src1 = effect_src_surface;
    if (src2 == NULL)
        src2 = effect_dst_surface;
    if (dst == NULL)
        dst = accumulation_surface;

    /* ---------------------------------------- */
    /* clipping */
    if ( clip ){
        if ( AnimationInfo::doClipping( &rect, clip ) ) return;
    }

    /* ---------------------------------------- */

    SDL_LockSurface( src1 );
    SDL_LockSurface( src2 );
    SDL_LockSurface( dst );
    if ( mask_surface ) SDL_LockSurface( mask_surface );
    
    ONSBuf *src1_buffer = (ONSBuf *)src1->pixels + src1->w * rect.y + rect.x;
    ONSBuf *src2_buffer = (ONSBuf *)src2->pixels + src2->w * rect.y + rect.x;
    ONSBuf *dst_buffer  = (ONSBuf *)dst->pixels + dst->w * rect.y + rect.x;

    const int rwidth = screen_width - rect.w;
    SDL_PixelFormat *fmt = dst->format;
    Uint32 overflow_mask = 0xffffffff;
    if ( trans_mode != ALPHA_BLEND_FADE_MASK )
        overflow_mask = ~fmt->Bmask;

    mask_value >>= fmt->Bloss;

    if (( trans_mode == ALPHA_BLEND_FADE_MASK ||
          trans_mode == ALPHA_BLEND_CROSSFADE_MASK ) && mask_surface) {
        for ( int i=0 ; i<rect.h ; i++ ) {
            ONSBuf *mask_buffer = (ONSBuf *)mask_surface->pixels + mask_surface->w * ((rect.y+i)%mask_surface->h);

            int offset = rect.x % mask_surface->w;
#if !defined(BPP16)
            int j=0, len = mask_surface->w - offset;
            while (j < rect.w) {
                if (len > (rect.w - j)) len = rect.w - j;
                ons_gfx::imageFilterEffectMaskBlend(dst_buffer, src1_buffer,
                                                    src2_buffer, mask_buffer+offset,
                                                    overflow_mask, mask_value,
                                                    len);
                src1_buffer += len;
                src2_buffer += len;
                dst_buffer  += len;
                j += len;
                offset = 0;
                len = mask_surface->w;
            }
#else //BPP16
            for ( int j=rect.w ; j!=0 ; j-- ){
                Uint32 mask2 = 0;
                Uint32 mask = *(mask_buffer + offset) & fmt->Bmask;
                if ( mask_value > mask ){
                    mask2 = mask_value - mask;
                    if ( mask2 & overflow_mask ) mask2 = fmt->Bmask;
                }
                BLEND_EFFECT_PIXEL();
                ++dst_buffer, ++src1_buffer, ++src2_buffer, ++offset;
                if (offset >= mask_surface->w) offset = 0;
            }
#endif
            src1_buffer += rwidth;
            src2_buffer += rwidth;
            dst_buffer  += rwidth;
        }
    }
    else{ // ALPHA_BLEND_CONST
        Uint32 mask2 = mask_value & fmt->Bmask;

        for ( int i=rect.h ; i!=0 ; i-- ) {
#if !defined(BPP16)
            ons_gfx::imageFilterEffectBlend(dst_buffer, src1_buffer,
                                            src2_buffer, mask2, rect.w);
            src1_buffer += screen_width;
            src2_buffer += screen_width;
            dst_buffer  += screen_width;
#else //BPP16
            for ( int j=rect.w ; j!=0 ; j-- ){
                BLEND_EFFECT_PIXEL();
                ++dst_buffer, ++src1_buffer, ++src2_buffer;
            }
            src1_buffer += rwidth;
            src2_buffer += rwidth;
            dst_buffer  += rwidth;
#endif
        }
    }
    
    if ( mask_surface ) SDL_UnlockSurface( mask_surface );
    SDL_UnlockSurface( dst );
    SDL_UnlockSurface( src2 );
    SDL_UnlockSurface( src1 );
}

// alphaBlendText
// dst: ONSBuf surface (accumulation_surface)
// txt: 8bit surface (TTF_RenderGlyph_Shaded())
void ONScripterLabel::alphaBlendText( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                                      SDL_Surface *txt_surface, SDL_Color &color, SDL_Rect *clip, bool rotate_flag )
{
    int x2=0, y2=0;
    SDL_Rect clipped_rect;

    /* ---------------------------------------- */
    /* 1st clipping */
    if ( clip ){
        if ( AnimationInfo::doClipping( &dst_rect, clip, &clipped_rect ) ) return;

        x2 += clipped_rect.x;
        y2 += clipped_rect.y;
    }

    /* ---------------------------------------- */
    /* 2nd clipping */
    SDL_Rect clip_rect = {0, 0, dst_surface->w, dst_surface->h};
    if ( AnimationInfo::doClipping( &dst_rect, &clip_rect, &clipped_rect ) ) return;
    
    x2 += clipped_rect.x;
    y2 += clipped_rect.y;

    /* ---------------------------------------- */

    SDL_LockSurface( dst_surface );
    SDL_LockSurface( txt_surface );

#ifdef BPP16
    Uint32 src_color = ((color.r >> RLOSS) << RSHIFT) |
                       ((color.g >> GLOSS) << GSHIFT) |
                       (color.b >> BLOSS);
    src_color = (src_color | src_color << 16) & BLENDMASK;
#else
    Uint32 src_color1 = color.r << RSHIFT | color.b;
    Uint32 src_color2 = color.g << GSHIFT;
    Uint32 src_color3 = src_color1 | src_color2;
#endif

    ONSBuf *dst_buffer = (ONSBuf *)dst_surface->pixels +
                         dst_surface->w * dst_rect.y + dst_rect.x;

    if (!rotate_flag){
        unsigned char *src_buffer = (unsigned char*)txt_surface->pixels +
                                    txt_surface->pitch * y2 + x2;
        for ( int i=dst_rect.h ; i!=0 ; i-- ){
            for ( int j=dst_rect.w ; j!=0 ; j--, dst_buffer++, src_buffer++ ){
                BLEND_TEXT();
            }
            dst_buffer += dst_surface->w - dst_rect.w;
            src_buffer += txt_surface->pitch - dst_rect.w;
        }
    }
    else{
        for ( int i=0 ; i<dst_rect.h ; i++ ){
            unsigned char *src_buffer = (unsigned char*)txt_surface->pixels + txt_surface->pitch*(txt_surface->h - x2 - 1) + y2 + i;
            for ( int j=dst_rect.w ; j!=0 ; j--, dst_buffer++ ){
                BLEND_TEXT();
                src_buffer -= txt_surface->pitch;
            }
            dst_buffer += dst_surface->w - dst_rect.w;
        }
    }
    
    SDL_UnlockSurface( txt_surface );
    SDL_UnlockSurface( dst_surface );
}

void ONScripterLabel::makeNegaSurface( SDL_Surface *surface, SDL_Rect &clip )
{
    SDL_LockSurface( surface );
    ONSBuf *buf = (ONSBuf *)surface->pixels + clip.y * surface->w + clip.x;

    ONSBuf mask = surface->format->Rmask | surface->format->Gmask | surface->format->Bmask;
    for ( int i=clip.h ; i>0 ; i-- ){
        for ( int j=clip.w ; j>0 ; j-- )
            *buf++ ^= mask;
        buf += surface->w - clip.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::makeMonochromeSurface( SDL_Surface *surface, SDL_Rect &clip )
{
    SDL_LockSurface( surface );
    ONSBuf *buffer = (ONSBuf *)surface->pixels + clip.y * surface->w + clip.x;

    for ( int i=clip.h ; i>0 ; i-- ){
        for ( int j=clip.w ; j>0 ; j--, buffer++ ){
            //Mion: NScr seems to use more "equal" 85/86/85 RGB blending, instead
            // of the 77/151/28 that onscr used to have. Using 85/86/85 now,
            // might add a parameter to "monocro" to allow choosing 77/151/28
            MONOCRO_PIXEL();
        }
        buffer += surface->w - clip.w;
    }

    SDL_UnlockSurface( surface );
}

void ONScripterLabel::refreshSurface( SDL_Surface *surface, SDL_Rect *clip_src, int refresh_mode )
{
    if (refresh_mode == REFRESH_NONE_MODE) return;

    SDL_Rect clip = {0, 0, surface->w, surface->h};
    if (clip_src) if ( AnimationInfo::doClipping( &clip, clip_src ) ) return;

    int i, top;
    SDL_BlitSurface( bg_info.image_surface, &clip, surface, &clip );

    if ( !all_sprite_hide_flag ){
        if ( z_order < 10 && refresh_mode & REFRESH_SAYA_MODE )
            top = 9;
        else
            top = z_order;
        for ( i=MAX_SPRITE_NUM-1 ; i>top ; i-- ){
            if ( sprite_info[i].image_surface && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip );
            }
        }
    }

    for ( i=0 ; i<3 ; i++ ){
        if (human_order[2-i] >= 0 && tachi_info[human_order[2-i]].image_surface){
            drawTaggedSurface( surface, &tachi_info[human_order[2-i]], clip );
        }
    }

    if ( windowback_flag ){
        if ( nega_mode == 1 ) makeNegaSurface( surface, clip );
        if ( monocro_flag )   makeMonochromeSurface( surface, clip );
        if ( nega_mode == 2 ) makeNegaSurface( surface, clip );

        if (!all_sprite2_hide_flag){
            for ( i=MAX_SPRITE2_NUM-1 ; i>=0 ; i-- ){
                if ( sprite2_info[i].image_surface && sprite2_info[i].visible ){
                    drawTaggedSurface( surface, &sprite2_info[i], clip );
                }
            }
        }

        if (refresh_mode & REFRESH_WINDOW_MODE)
            displayTextWindow( surface, clip );
        if (refresh_mode & REFRESH_TEXT_MODE)
            text_info.blendOnSurface( surface, 0, 0, clip );
    }

    if ( !all_sprite_hide_flag ){
        if ( refresh_mode & REFRESH_SAYA_MODE )
            top = 10;
        else
            top = 0;
        for ( i=z_order ; i>=top ; i-- ){
            if ( sprite_info[i].image_surface && sprite_info[i].visible ){
                drawTaggedSurface( surface, &sprite_info[i], clip );
            }
        }
    }

    if ( !windowback_flag ){
        //Mion - ogapee2008
        if (!all_sprite2_hide_flag){
            for ( i=MAX_SPRITE2_NUM-1 ; i>=0 ; i-- ){
                if ( sprite2_info[i].image_surface && sprite2_info[i].visible ){
                    drawTaggedSurface( surface, &sprite2_info[i], clip );
                }
            }
        }
        if ( nega_mode == 1 ) makeNegaSurface( surface, clip );
        if ( monocro_flag )   makeMonochromeSurface( surface, clip );
        if ( nega_mode == 2 ) makeNegaSurface( surface, clip );
    }
    
    if ( !( refresh_mode & REFRESH_SAYA_MODE ) ){
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( bar_info[i] ) {
                drawTaggedSurface( surface, bar_info[i], clip );
            }
        }
        for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
            if ( prnum_info[i] ){
                drawTaggedSurface( surface, prnum_info[i], clip );
            }
        }
    }

    if ( !windowback_flag ){
        if (refresh_mode & REFRESH_WINDOW_MODE)
            displayTextWindow( surface, clip );
        if (refresh_mode & REFRESH_TEXT_MODE)
            text_info.blendOnSurface( surface, 0, 0, clip );
    }

    if ( refresh_mode & REFRESH_CURSOR_MODE && !textgosub_label ){
        if ( clickstr_state == CLICK_WAIT )
            drawTaggedSurface( surface, &cursor_info[CURSOR_WAIT_NO], clip );
        else if ( clickstr_state == CLICK_NEWPAGE )
            drawTaggedSurface( surface, &cursor_info[CURSOR_NEWPAGE_NO], clip );
    }

    //Mion: fix for the menu title bug noted in the past by Seung Park:
    // the right-click menu title must be drawn close to last during refresh,
    // not in the textwindow, since there could be sprites above the
    // textwindow if windowback is used.
    if (system_menu_title)
        drawTaggedSurface( surface, system_menu_title, clip );

    ButtonLink *p_button_link = root_button_link.next;
    while( p_button_link ){
        ButtonLink *cur_button_link = p_button_link;
        while (cur_button_link) {
            if (cur_button_link->show_flag > 0){
                drawTaggedSurface( surface, cur_button_link->anim[cur_button_link->show_flag-1], clip );
            }
            cur_button_link = cur_button_link->same;
        }
        p_button_link = p_button_link->next;
    }
}

void ONScripterLabel::refreshSprite( int sprite_no, bool active_flag,
				     int cell_no, SDL_Rect *check_src_rect,
				     SDL_Rect *check_dst_rect )
{
    if (( sprite_info[sprite_no].image_name ||
          ((sprite_info[sprite_no].trans_mode == AnimationInfo::TRANS_STRING) &&
           sprite_info[sprite_no].file_name) ) && 
        ( (sprite_info[ sprite_no ].visible != active_flag) ||
          ((cell_no >= 0) && (sprite_info[ sprite_no ].current_cell != cell_no)) ||
          (AnimationInfo::doClipping(check_src_rect, &sprite_info[ sprite_no ].pos) == 0) ||
          (AnimationInfo::doClipping(check_dst_rect, &sprite_info[ sprite_no ].pos) == 0) ))
    {
        if ( cell_no >= 0 )
            sprite_info[ sprite_no ].setCell(cell_no);

        sprite_info[ sprite_no ].visible = active_flag;

        dirty_rect.add( sprite_info[ sprite_no ].pos );
    }
}

void ONScripterLabel::createBackground()
{
    bg_info.num_of_cells = 1;
    bg_info.trans_mode = AnimationInfo::TRANS_COPY;
    bg_info.pos.x = 0;
    bg_info.pos.y = 0;
    bg_info.allocImage( screen_width, screen_height );

    if ( !strcmp( bg_info.file_name, "white" ) ){
        bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0xff;
    }
    else if ( !strcmp( bg_info.file_name, "black" ) ){
        bg_info.color[0] = bg_info.color[1] = bg_info.color[2] = 0x00;
    }
    else if ( bg_info.file_name[0] == '#' ){
        readColor( &bg_info.color, bg_info.file_name );
    }
    else{
        AnimationInfo anim;
        setStr( &anim.image_name, bg_info.file_name );
        parseTaggedString( &anim );
        anim.trans_mode = AnimationInfo::TRANS_COPY;
        anim.num_of_cells = 1;
#ifdef RCA_SCALE
        //stretch the bg image to match against the original screen sizing
        if ( scr_stretch_y > 1.0 || scr_stretch_x > 1.0 )
            setupAnimationInfo( &anim, NULL, scr_stretch_x, scr_stretch_y );
        else
#endif
        setupAnimationInfo( &anim );

        bg_info.fill(0, 0, 0, 0xff);
        if (anim.image_surface){
            SDL_Rect src_rect = {0, 0, anim.image_surface->w, anim.image_surface->h};
            SDL_Rect dst_rect = {0, 0, screen_width, screen_height};
            if (screen_width >= anim.image_surface->w){
                dst_rect.x = (screen_width - anim.image_surface->w) / 2;
            }
            else{
                src_rect.x = (anim.image_surface->w - screen_width) / 2;
                src_rect.w = screen_width;
            }
            if (screen_height >= anim.image_surface->h){
                dst_rect.y = (screen_height - anim.image_surface->h) / 2;
            }
            else{
                src_rect.y = (anim.image_surface->h - screen_height) / 2;
                src_rect.h = screen_height;
            }
            bg_info.copySurface(anim.image_surface, &src_rect, &dst_rect);
        }
        return;
    }

    bg_info.fill(bg_info.color[0], bg_info.color[1], bg_info.color[2], 0xff);
}
