/* -*- C++ -*-
 *
 *  ONScripterLabel_text.cpp - Text parser of ONScripter-EN
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

// Modified by Mion , March 2008, to update from
// Ogapee's 20080121 release source code.
//   Modified April 2008 to redo the text linebreak processing.

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#include "ONScripterLabel.h"

extern unsigned short convSJIS2UTF16( unsigned short in );

static inline bool isRotationRequired(const char *text)
{
    if ( !IS_TWO_BYTE(text[0]) )    // ascii, halfwidth kana
        return true;
    //fullwidth commas or full-stops
    return ((text[0] == (char)0x81) &&
            ( ((text[1] >= 0x5b) && (text[1] <= 0x5d)) ||  //hyphens
              ((text[1] >= 0x60) && (text[1] <= 0x64)) ||  //tilde,ellipses
              ((text[1] >= 0x69) && (text[1] <= 0x7a)) ||  //parens,brackets
              (text[1] == 0x50) || (text[1] == 0x51) ||    //macron,lowline
              (text[1] == (char)0x80) ));                  //division sign
}

static inline bool isTranslationRequired(const char *text)
{
    //fullwidth commas or full-stops
    return ((text[0] == (char)0x81) &&
            (text[1] >= 0x41) && (text[1] <= 0x44));
}

static inline bool isNonPrinting(const char *text)
{
    //control chars or fullwidth space
    return ( ((unsigned char) text[0] <= 0x20) ||
             ((text[0] == (char)0x81) && (text[1] == (char)0x40)) );
}

SDL_Surface *ONScripterLabel::renderGlyph(TTF_Font *font, Uint16 text)
{
    GlyphCache *gc = root_glyph_cache;
    GlyphCache *pre_gc = gc;
    while(1){
        if (gc->text == text &&
            gc->font == font){
            if (gc != pre_gc){
                pre_gc->next = gc->next;
                gc->next = root_glyph_cache;
                root_glyph_cache = gc;
            }
            return gc->surface;
        }
        if (gc->next == NULL) break;
        pre_gc = gc;
        gc = gc->next;
    }

    pre_gc->next = NULL;
    gc->next = root_glyph_cache;
    root_glyph_cache = gc;

    gc->text = text;
    gc->font = font;
    if (gc->surface) SDL_FreeSurface(gc->surface);

    /* Initializing SDL_Color.unused here to silence warnings about unused
       variables. 32 bit operations should be faster than 24 bit ones anyway.
       Users will be delighted by this 0.0000001 microsecond increase in speed.
     (contribution by Andrius, March 2010) */
    static SDL_Color fcol={0xff, 0xff, 0xff, 0xff}, bcol={0, 0, 0, 0};
    gc->surface = TTF_RenderGlyph_Shaded( font, text, fcol, bcol );

    return gc->surface;
}

void ONScripterLabel::drawGlyph( SDL_Surface *dst_surface, Fontinfo *info, SDL_Color &color, char* text, int xy[2], bool shadow_flag, AnimationInfo *cache_info, SDL_Rect *clip, SDL_Rect &dst_rect )
{
    //in case of font size 0
    if ((info->font_size_xy[0] == 0) || (info->font_size_xy[1] == 0))
        return;

    unsigned short unicode;
    if (IS_TWO_BYTE(text[0])){
        unsigned index = ((unsigned char*)text)[0];
        index = index << 8 | ((unsigned char*)text)[1];
        unicode = convSJIS2UTF16( index );
    }
    else{
        unicode = convSJIS2UTF16( ((unsigned char*)text)[0] );
    }

    int minx, maxx, miny, maxy, advanced;
#if 0
    if (TTF_GetFontStyle( (TTF_Font*)info->ttf_font ) !=
        (info->is_bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL) )
        TTF_SetFontStyle( (TTF_Font*)info->ttf_font, (info->is_bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL));
#endif
    TTF_GlyphMetrics( (TTF_Font*)info->ttf_font, unicode,
                      &minx, &maxx, &miny, &maxy, &advanced );
    //printf("min %d %d %d %d %d %d\n", minx, maxx, miny, maxy, advanced,TTF_FontAscent((TTF_Font*)info->ttf_font)  );

    SDL_Surface *tmp_surface = renderGlyph( (TTF_Font*)info->ttf_font, unicode );

    if (tmp_surface == NULL) return;

    bool rotate_flag = false;
    if ( (info->getTateyokoMode() == Fontinfo::TATE_MODE) &&
         isRotationRequired(text) )
        rotate_flag = true;

    //Mion: to display vertical text more cleanly
    if ( (info->getTateyokoMode() == Fontinfo::TATE_MODE) &&
         isTranslationRequired(text) ){
        dst_rect.x = xy[0] + ExpandPos(info->font_size_xy[0]) - maxx;
        dst_rect.y = xy[1] + minx;
    }
    else if ( rotate_flag ) {
        dst_rect.x = xy[0] + ExpandPos(info->font_size_xy[0]) -
                     TTF_FontAscent((TTF_Font*)info->ttf_font) + miny;
        dst_rect.y = xy[1] + minx;
    }
    else {
        dst_rect.x = xy[0] + minx;
        dst_rect.y = xy[1] + TTF_FontAscent((TTF_Font*)info->ttf_font) - maxy;
    }

    if ( shadow_flag ){
        dst_rect.x += shade_distance[0];
        dst_rect.y += shade_distance[1];
    }

    if (rotate_flag){
        dst_rect.w = tmp_surface->h;
        dst_rect.h = tmp_surface->w;
    }
    else{
        dst_rect.w = tmp_surface->w;
        dst_rect.h = tmp_surface->h;
    }

    if (cache_info)
        cache_info->blendText( tmp_surface, dst_rect.x, dst_rect.y, color, clip, rotate_flag );

    if (dst_surface)
        alphaBlendText( dst_surface, dst_rect, tmp_surface, color, clip, rotate_flag );
}

void ONScripterLabel::drawChar( char* text, Fontinfo *info, bool flush_flag,
                                bool lookback_flag, SDL_Surface *surface,
                                AnimationInfo *cache_info, int abs_offset, SDL_Rect *clip )
{
    //printf("draw %x-%x[%s] %d, %d\n", text[0], text[1], text, info->xy[0], info->xy[1] );

    if ( info->ttf_font == NULL ){
        if ( info->openFont( font_file, screen_ratio1, screen_ratio2 ) == NULL ){
            snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                     "can't open font file: %s\n", font_file );
            errorAndExit(script_h.errbuf);
        }
    }

    if ( info->isEndOfLine() ){
        info->newLine();
        for (int i=0 ; i<indent_offset ; i++)
            sentence_font.advanceCharInHankaku(2);

        if ( lookback_flag ){
            for (int i=0 ; i<indent_offset ; i++){
                current_page->add(0x81);
                current_page->add(0x40);
            }
        }
    }

    char out_text[3]= {'\0', '\0', '\0'};
    if (text[0] == ScriptHandler::LEFT_PAREN) {
        out_text[0] = '(';
    } else if (text[0] == ScriptHandler::RIGHT_PAREN) {
        out_text[0] = ')';
    } else if (text[0] == '\t') {
        out_text[0] = ' '; //draw tabs as spaces, for now
    } else if ((unsigned char)text[0] < 0x20) {
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "drawChar: got unrecognized control character 0x%02x", text[0]);
        errorAndCont(script_h.errbuf);
        out_text[0] = ' ';
    } else {
        out_text[0] = text[0];
        out_text[1] = text[1];
    }

    int xy[2];
    xy[0] = abs_offset + ExpandPos(info->x());
    xy[1] = ExpandPos(info->y());

    if ( !isNonPrinting(&out_text[0]) ){
        //don't bother drawing non-printing glyphs
        SDL_Color color;
        SDL_Rect dst_rect;
        if ( info->is_shadow ){
            color.r = color.g = color.b = 0;
            drawGlyph(surface, info, color, out_text, xy, true, cache_info, clip, dst_rect);
        }
        color.r = info->color[0];
        color.g = info->color[1];
        color.b = info->color[2];
        drawGlyph( surface, info, color, out_text, xy, false, cache_info, clip, dst_rect );

        if ( surface == accumulation_surface &&
             !flush_flag &&
             (!clip || AnimationInfo::doClipping( &dst_rect, clip ) == 0) ){
            info->addShadeArea(dst_rect, shade_distance);
            dirty_rect.add( dst_rect );
        }
        else if ( flush_flag ){
            info->addShadeArea(dst_rect, shade_distance);
            flushDirect( dst_rect, REFRESH_NONE_MODE );
        }
    }

    /* ---------------------------------------- */
    /* Update text buffer */
    if (IS_TWO_BYTE(text[0]))
        info->advanceCharInHankaku(2);
    else
        info->advanceCharInHankaku(1);

    if ( lookback_flag ){
        current_page->add( text[0] );
        if (IS_TWO_BYTE(text[0]))
            current_page->add( text[1] );
    }
}

void ONScripterLabel::drawString( const char *str, uchar3 color, Fontinfo *info,
                                  bool flush_flag, SDL_Surface *surface, 
                                  int abs_offset, SDL_Rect *rect,
                                  AnimationInfo *cache_info, bool skip_whitespace_flag )
{
    int i;

    int start_xy[2];
    start_xy[0] = info->xy[0];
    start_xy[1] = info->xy[1];

    /* ---------------------------------------- */
    /* Draw selected characters */
    uchar3 org_color;
    setColor(org_color, info->color);
    setColor(info->color, color);

    bool tateyoko = (info->getTateyokoMode() == Fontinfo::TATE_MODE);
    char text[3] = { '\0', '\0', '\0' };
    while( *str ){
        while (*str == ' ' && skip_whitespace_flag) str++;
        if (!*str) break;

        if ( *str == '`' ){
            str++;
            skip_whitespace_flag = false;
            continue;
        }
        if (cache_info && !cache_info->is_tight_region){
            if (*str == '('){
                startRuby((char *)str+1, *info);
                info->addLineOffset(ruby_struct.margin);
                str++;
                continue;
            }
            else if (*str == '/' && ruby_struct.stage == RubyStruct::BODY ){
                info->addLineOffset(ruby_struct.margin);
                str = ruby_struct.ruby_end;
                if (*ruby_struct.ruby_end == ')'){
                    endRuby(false, false, NULL, cache_info);
                    str++;
                }
                continue;
            }
            else if (*str == ')' && ruby_struct.stage == RubyStruct::BODY ){
                ruby_struct.stage = RubyStruct::NONE;
                str++;
                continue;
            }
            //Mion: handling special text locate chars that could be found in
            //a "getlog" string
            else if (*str == ScriptHandler::TEXT_FF) {
                //form feed - reset to the top of the page (locate)
                if (tateyoko)
                    info->xy[0] = (info->num_xy[0]-1) * 2;
                else
                    info->xy[1] = 0;
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_VTAB) {
                //vertical tab - move one line down the page (locate)
                if (tateyoko)
                    info->xy[0] -= 2;
                else
                    info->xy[1] += 2;
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_CR) {
                //carriage return - reset to the start of the line (locate)
                if (tateyoko)
                    info->xy[1] = 0;
                else
                    info->xy[0] = 0;
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_UP) {
                info->setXY(-1, info->xy[1]/2 - 1);
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_DOWN) {
                info->setXY(-1, info->xy[1]/2 + 1);
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_LEFT) {
                info->setXY(info->xy[0]/2 - 1);
                str++;
                continue;
            }
            else if (*str == ScriptHandler::TEXT_RIGHT) {
                info->setXY(info->xy[0]/2 + 1);
                str++;
                continue;
            }
        }

        if ( IS_TWO_BYTE(*str) ){
            /* Kinsoku process */
            if (isEndKinsoku( str+2 )){
                int i = 2;
                while (!info->isEndOfLine(i) &&
                       isEndKinsoku( str+2+i )){
                    i += 2;
                }
                if (info->isEndOfLine(i)){
                    info->newLine();
                    for (int i=0 ; i<indent_offset ; i++){
                        sentence_font.advanceCharInHankaku(2);
                    }
                }
            }

            text[0] = *str++;
            text[1] = *str++;
            drawChar( text, info, false, false, surface, cache_info, abs_offset );
        }
        else if ((*str == 0x0a) ||
                 ((*str == '\\') && info->is_newline_accepted)){
            info->newLine();
            str++;
        }
        else{
            text[0] = *str++;
            text[1] = '\0';
            drawChar( text, info, false, false, surface, cache_info, abs_offset );
            //Mion: fix for allowing mixed 1 & 2 byte chars in English mode
            if (script_h.preferred_script == ScriptHandler::JAPANESE_SCRIPT) {
                if (*str && *str != 0x0a){
                    text[0] = *str++;
                    drawChar( text, info, false, false, surface, cache_info, abs_offset );
                }
            }
        }
    }
    for ( i=0 ; i<3 ; i++ ) info->color[i] = org_color[i];

    /* ---------------------------------------- */
    /* Calculate the area of selection */
    SDL_Rect clipped_rect = info->calcUpdatedArea(start_xy, screen_ratio1, screen_ratio2);
    clipped_rect.x += abs_offset;
    info->addShadeArea(clipped_rect, shade_distance);

    if ( flush_flag )
        flush( refresh_window_text_mode, &clipped_rect );

    if ( rect ) *rect = clipped_rect;
}

void ONScripterLabel::restoreTextBuffer(SDL_Surface *surface)
{
    text_info.fill( 0, 0, 0, 0 );

    char out_text[3] = { '\0','\0','\0' };
    Fontinfo f_info = sentence_font;
    f_info.clear();
    bool tateyoko = (f_info.getTateyokoMode() == Fontinfo::TATE_MODE);
    for ( int i=0 ; i<current_page->text_count ; i++ ){
        if ( current_page->text[i] == 0x0a ){
            f_info.newLine();
        }
        else{
            out_text[0] = current_page->text[i];
            if (out_text[0] == '('){
                startRuby(current_page->text + i + 1, f_info);
                f_info.addLineOffset(ruby_struct.margin);
                continue;
            }
            else if (out_text[0] == '/' && ruby_struct.stage == RubyStruct::BODY ){
                f_info.addLineOffset(ruby_struct.margin);
                i = ruby_struct.ruby_end - current_page->text - 1;
                if (*ruby_struct.ruby_end == ')'){
                    endRuby(false, false, NULL, &text_info);
                    i++;
                }
                continue;
            }
            else if (out_text[0] == ')' && ruby_struct.stage == RubyStruct::BODY ){
                ruby_struct.stage = RubyStruct::NONE;
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_FF) {
                //form feed - reset to the top of the page (locate)
                if (tateyoko)
                    f_info.xy[0] = (f_info.num_xy[0]-1) * 2;
                else
                    f_info.xy[1] = 0;
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_VTAB) {
                //vertical tab - move one line down the page (locate)
                if (tateyoko)
                    f_info.xy[0] -= 2;
                else
                    f_info.xy[1] += 2;
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_CR) {
                //carriage return - reset to the start of the line (locate)
                if (tateyoko)
                    f_info.xy[1] = 0;
                else
                    f_info.xy[0] = 0;
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_UP) {
                f_info.setXY(-1, f_info.xy[1]/2 - 1);
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_DOWN) {
                f_info.setXY(-1, f_info.xy[1]/2 + 1);
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_LEFT) {
                f_info.setXY(f_info.xy[0]/2 - 1);
                continue;
            }
            else if (out_text[0] == ScriptHandler::TEXT_RIGHT) {
                f_info.setXY(f_info.xy[0]/2 + 1);
                continue;
            }
            if ( IS_TWO_BYTE(out_text[0]) ){
                out_text[1] = current_page->text[i+1];
                i++;
            }
            else{
                out_text[1] = '\0';
            }
            drawChar( out_text, &f_info, false, false, surface, &text_info );
        }
    }
}

void ONScripterLabel::enterTextDisplayMode(bool text_flag)
{
    if (line_enter_status <= 1 && saveon_flag && internal_saveon_flag && text_flag){
        saveSaveFile( -1 );
        internal_saveon_flag = false;
    }

    did_leavetext = false;
    if ( !(display_mode & DISPLAY_MODE_TEXT) ){
        refreshSurface( effect_dst_surface, NULL, refresh_window_text_mode );
        dirty_rect.clear();
        dirty_rect.add( sentence_font_info.pos );

        if (setEffect(&window_effect, false, true)) return;
        while(doEffect(&window_effect, false));

        display_mode = DISPLAY_MODE_TEXT;
        text_on_flag = true;
    }
}

void ONScripterLabel::leaveTextDisplayMode(bool force_leave_flag)
{
    //ons-en feature: when in certain skip modes, don't actually leave
    //text display mode unless forced to (but say you did)
    if (!force_leave_flag && ( skip_mode & (SKIP_NORMAL | SKIP_TO_EOP) || ctrl_pressed_status )) {
        did_leavetext = true;
        return;
    }
    if (force_leave_flag) did_leavetext = false;

    if ( !did_leavetext && (display_mode & DISPLAY_MODE_TEXT) &&
         (force_leave_flag || (erase_text_window_mode != 0)) ){

        SDL_BlitSurface(backup_surface, NULL, effect_dst_surface, NULL);
        dirty_rect.add(sentence_font_info.pos);

        if (setEffect(&window_effect, false, false)) return;
        while(doEffect(&window_effect, false));

        display_mode = DISPLAY_MODE_NORMAL;
    }

    display_mode |= DISPLAY_MODE_UPDATED;
}

bool ONScripterLabel::doClickEnd()
{
    bool ret = false;

    draw_cursor_flag = true;

    if (!((skip_mode & SKIP_TO_EOL) && clickskippage_flag))
        skip_mode &= ~(SKIP_TO_WAIT | SKIP_TO_EOL);

    if ( automode_flag ){
        event_mode =  WAIT_TEXT_MODE | WAIT_INPUT_MODE |
                      WAIT_VOICE_MODE | WAIT_TIMER_MODE;
        if ( automode_time < 0 )
            ret = waitEvent( -automode_time * num_chars_in_sentence );
        else
            ret = waitEvent( automode_time );
    }
    else if ( autoclick_time > 0 ){
        event_mode = WAIT_SLEEP_MODE | WAIT_TIMER_MODE;
        ret = waitEvent( autoclick_time );
    }
    else{
        event_mode = WAIT_TEXT_MODE | WAIT_INPUT_MODE | WAIT_TIMER_MODE;
        ret = waitEvent(-1);
    }

    num_chars_in_sentence = 0;
    draw_cursor_flag = false;

    return ret;
}

bool ONScripterLabel::clickWait()
{
    int tmp_skip = skip_mode;
    skip_mode &= ~(SKIP_TO_WAIT | SKIP_TO_EOL);
    flush( REFRESH_NONE_MODE );

    //Mion: apparently NScr doesn't call textgosub on clickwaits
    // while in skip mode (but does call it on pagewaits)
    if ( (skip_mode & (SKIP_NORMAL | SKIP_TO_EOP)) ||
         ((tmp_skip & SKIP_TO_EOL) && clickskippage_flag) ||
          ctrl_pressed_status ){
        skip_mode = tmp_skip;
        clickstr_state = CLICK_NONE;
        num_chars_in_sentence = 0;
        if ( textgosub_label && (script_h.getNext()[0] != 0x0a))
            new_line_skip_flag = true;
        event_mode = IDLE_EVENT_MODE;
        if ( waitEvent(0) ) return false;
    }
    else{

        key_pressed_flag = false;

        if ( textgosub_label ){
            if ((tmp_skip & SKIP_TO_EOL) && clickskippage_flag)
                skip_mode = tmp_skip;
            saveoffCommand();
            clickstr_state = CLICK_NONE;

            char *next = script_h.getNext();
            if (*next == 0x0a) {
                textgosub_clickstr_state = CLICK_WAITEOL;
            } else {
                new_line_skip_flag = true;
                textgosub_clickstr_state = CLICK_WAIT;
            }
            gosubReal( textgosub_label, next, true );

            return false;
        }

        clickstr_state = CLICK_WAIT;
        if (doClickEnd()) return false;

        clickstr_state = CLICK_NONE;
        key_pressed_flag = false;
    }
    script_h.cur_rgosub_wait++;

    return true;
}

bool ONScripterLabel::clickNewPage()
{
    skip_mode &= ~(SKIP_TO_WAIT | SKIP_TO_EOL);
    flush( REFRESH_NONE_MODE );
    clickstr_state = CLICK_NEWPAGE;

    if ( (skip_mode & SKIP_NORMAL || ctrl_pressed_status) && !textgosub_label ){
        clickstr_state = CLICK_NONE;
        num_chars_in_sentence = 0;

        event_mode = IDLE_EVENT_MODE;
        if (waitEvent(0)) return false;
    }
    else{
        key_pressed_flag = false;

        if ( textgosub_label ){
            saveoffCommand();
            clickstr_state = CLICK_NONE;

            char *next = script_h.getNext();
            textgosub_clickstr_state = CLICK_NEWPAGE;
            gosubReal( textgosub_label, next, true );

            return false;
        }

        if (doClickEnd()) return false;
    }

    newPage( true );
    clickstr_state = CLICK_NONE;
    key_pressed_flag = false;
    script_h.cur_rgosub_wait++;

    return true;
}

void ONScripterLabel::startRuby(char *buf, Fontinfo &info)
{
    ruby_struct.stage = RubyStruct::BODY;
    ruby_font = info;
    ruby_font.ttf_font = NULL;
    if ( ruby_struct.font_size_xy[0] != -1 )
        ruby_font.font_size_xy[0] = ruby_struct.font_size_xy[0];
    else
        ruby_font.font_size_xy[0] = info.font_size_xy[0]/2;
    if ( ruby_struct.font_size_xy[1] != -1 )
        ruby_font.font_size_xy[1] = ruby_struct.font_size_xy[1];
    else
        ruby_font.font_size_xy[1] = info.font_size_xy[1]/2;

    ruby_struct.body_count = 0;
    ruby_struct.ruby_count = 0;

    while(1){
        if ( *buf == '/' ){
            ruby_struct.stage = RubyStruct::RUBY;
            ruby_struct.ruby_start = buf+1;
        }
        else if ( *buf == ')' || *buf == '\0' ){
            break;
        }
        else{
            if ( ruby_struct.stage == RubyStruct::BODY )
                ruby_struct.body_count++;
            else if ( ruby_struct.stage == RubyStruct::RUBY )
                ruby_struct.ruby_count++;
        }
        buf++;
    }
    ruby_struct.ruby_end = buf;
    ruby_struct.stage = RubyStruct::BODY;
    ruby_struct.margin = ruby_font.initRuby(info, ruby_struct.body_count/2, ruby_struct.ruby_count/2);
}

void ONScripterLabel::endRuby(bool flush_flag, bool lookback_flag, SDL_Surface *surface, AnimationInfo *cache_info)
{
    char out_text[3]= {'\0', '\0', '\0'};
    if ( rubyon_flag ){
        ruby_font.clear();
        char *buf = ruby_struct.ruby_start;
        while( buf < ruby_struct.ruby_end ){
            out_text[0] = *buf;
            if ( IS_TWO_BYTE(*buf) ){
                out_text[1] = *(buf+1);
                drawChar( out_text, &ruby_font, flush_flag, lookback_flag, surface, cache_info );
                buf++;
            }
            else{
                out_text[1] = '\0';
                drawChar( out_text, &ruby_font, flush_flag,  lookback_flag, surface, cache_info );
            }
            buf++;
        }
    }
    ruby_struct.stage = RubyStruct::NONE;
}

int ONScripterLabel::textCommand()
{
    if (line_enter_status <= 1 && saveon_flag && internal_saveon_flag){
        saveSaveFile( -1 );
        internal_saveon_flag = false;
    }

    char *buf = script_h.getStringBuffer();

    if (pretextgosub_label && 
        ( (script_h.current_cmd_type == ScriptHandler::CMD_PRETEXT) ||
          ((!pagetag_flag || (page_enter_status == 0)) &&
           (line_enter_status == 0)) )){

        bool tag_flag = (script_h.current_cmd_type == ScriptHandler::CMD_PRETEXT);

        if (current_page->tag) delete[] current_page->tag;
        if (tag_flag && (buf[0] != '\0')){
            int len = strlen(buf);
            string_buffer_offset = len;
            current_page->tag = new char[len+1];
            memcpy(current_page->tag, buf, strlen(buf));
            current_page->tag[len] = 0;
        }
        else{
            current_page->tag = NULL;
        }

        gosubReal( pretextgosub_label, script_h.getNext(), true );
        line_enter_status = 1;

        return RET_CONTINUE;
    }

    refresh_window_text_mode = REFRESH_NORMAL_MODE | REFRESH_WINDOW_MODE | REFRESH_TEXT_MODE;
    enterTextDisplayMode();

    line_enter_status = 2;
    if (pagetag_flag) page_enter_status = 1;

    if (debug_level > 1)
        printf("textCommand %s %d %d %d\n", script_h.getStringBuffer() + string_buffer_offset, string_buffer_offset, event_mode, line_enter_status);
    while(processText());

    return RET_CONTINUE;
}

void ONScripterLabel::processEOT()
{
    skip_mode &= ~SKIP_TO_WAIT;
    if ((skip_mode & SKIP_TO_EOL) || (sentence_font.wait_time == 0)){
        flush(refreshMode());
        if (!clickskippage_flag && !skip_past_newline)
            skip_mode &= ~SKIP_TO_EOL;
    }
    indent_offset = 0;
    if (!sentence_font.isLineEmpty() && !new_line_skip_flag){
        doLineBreak(true);
    }
    line_enter_status = 0;
}

bool ONScripterLabel::processText()
//Mion: extensively modified the text processing
{
    //printf("processText %s %d %d %d\n", script_h.getStringBuffer() + string_buffer_offset, string_buffer_offset, event_mode, line_enter_status);

    char out_text[3]= {'\0', '\0', '\0'};

    bool old_new_line_skip_flag = new_line_skip_flag; //Mion: for temp Umineko8 fix

    // process linebreaking when at the start of (non-pretext) buffer
    if (line_enter_status == 2) {
        if (!new_line_skip_flag)
            line_has_nonspace = true;
        if (!sentence_font.isLineEmpty()) {
            new_line_skip_flag = true;
        }
        if ( script_h.preferred_script == ScriptHandler::JAPANESE_SCRIPT ) {
            processBreaks(new_line_skip_flag, KINSOKU);
        } else {
            if (! processBreaks(new_line_skip_flag, SPACEBREAK)) {
                // no spaces or printable ASCII found in this line,
                // use kinsoku rules
                processBreaks(new_line_skip_flag, KINSOKU);
            }
        }
        line_enter_status = 3;
    }

    if (script_h.getStringBuffer()[string_buffer_offset] == 0x00){
        processEOT();
        return false;
    }

    new_line_skip_flag = false;
    
    char ch = script_h.getStringBuffer()[string_buffer_offset];

    if (!line_has_nonspace) {
        // skip over leading spaces in a continued line
        while (ch == ' ')
            ch = script_h.getStringBuffer()[++string_buffer_offset];
    }

    int cmd = isTextCommand(script_h.getStringBuffer() + string_buffer_offset);
    if (cmd <= 0) {
        line_has_nonspace = true;
        if (sentence_font.isEndOfLine(0) ||
            (IS_TWO_BYTE(ch) && sentence_font.isEndOfLine(1))) {
            // no room for current char on the line
            //printf("at end; breaking before %s", script_h.getStringBuffer() + string_buffer_offset);
            ch = doLineBreak();
        }
        else {
            int break_offset, length, margins, tmp;
            break_offset = findNextBreak(string_buffer_offset, length);
            if (break_offset == string_buffer_offset) {
                if (cmd < 0) {
                    break_offset++;
                    tmp = 0;
                }
                else if (IS_TWO_BYTE(ch)) {
                    break_offset += 2;
                    tmp = 2;
                } else {
                    break_offset += 1;
                    tmp = 1;
                }
                break_offset = findNextBreak(break_offset, length);
                //printf("next break before %s", script_h.getStringBuffer() + break_offset);
                margins = 0;
                for (int i = string_buffer_offset; i < break_offset; i++)
                    margins += string_buffer_margins[i];
                sentence_font.addLineOffset(margins);
                if (sentence_font.isEndOfLine(length+tmp-1)) {
                    //printf("breaking before %s", script_h.getStringBuffer() + string_buffer_offset);
                    ch = doLineBreak();
                } else {
                    sentence_font.addLineOffset(-margins);
                }
            }
        }
    }

    new_line_skip_flag = false;

    if ( IS_TWO_BYTE(ch) ){ // Shift jis

        bool flush_flag = !(skip_mode || ctrl_pressed_status || (sentence_font.wait_time == 0));

        out_text[0] = script_h.getStringBuffer()[string_buffer_offset];
        out_text[1] = script_h.getStringBuffer()[string_buffer_offset+1];

        last_textpos_xy[0] = sentence_font.x()-sentence_font.ruby_offset_xy[0];
        last_textpos_xy[1] = sentence_font.y()-sentence_font.ruby_offset_xy[1];
        drawChar( out_text, &sentence_font, flush_flag, true, accumulation_surface, &text_info );

        if (flush_flag) {
            if (!( (skip_mode & (SKIP_TO_WAIT | SKIP_TO_EOL)) ||
                   (sentence_font.wait_time == 0) )) {
                event_mode = WAIT_TEXTOUT_MODE;
                if ( sentence_font.wait_time == -1 )
                    waitEvent( default_text_speed[text_speed_no] );
                else
                    waitEvent( sentence_font.wait_time );
            }
        }
        num_chars_in_sentence += 2;
        string_buffer_offset += 2;

        return true;
    }
    else if (script_h.checkClickstr(&script_h.getStringBuffer()[string_buffer_offset]) == -2) {
        // got the special "\@" clickwait-or-page
        if (in_txtbtn)
            terminateTextButton();
        string_buffer_offset += 2;
        if (sentence_font.getRemainingLine() <= clickstr_line)
            return clickNewPage();
        else
            return clickWait();
    }
    else if ( ch == '`' ) {
        string_buffer_offset++;
        return true;
    }
    else if ( ch == '@' ){ // wait for click
        if (in_txtbtn)
            terminateTextButton();
        string_buffer_offset++;
        return clickWait();
    }
    else if ( ch == '\\' ){ // new page
        if (in_txtbtn)
            terminateTextButton();
        string_buffer_offset++;
        return clickNewPage();
    }
    else if ( ch == '!' ){
        string_buffer_offset++;
        if ( script_h.getStringBuffer()[ string_buffer_offset ] == 's' ){
            if (in_txtbtn)
                terminateTextButton();
            string_buffer_offset++;
            if (!skip_mode && (sentence_font.wait_time == 0)) flush(refreshMode());
            if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ){
                sentence_font.wait_time = -1;
                string_buffer_offset++;
            }
            else{
                int t = 0;
                while( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                       script_h.getStringBuffer()[ string_buffer_offset ] <= '9' ){
                    t = t*10 + script_h.getStringBuffer()[ string_buffer_offset ] - '0';
                    string_buffer_offset++;
                }
                sentence_font.wait_time = t;
                while (script_h.getStringBuffer()[ string_buffer_offset ] == ' ' ||
                       script_h.getStringBuffer()[ string_buffer_offset ] == '\t') string_buffer_offset++;
            }
        }
        else if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'w' ||
                  script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ){
            event_mode = WAIT_SLEEP_MODE;

            bool flag = false;
            bool in_skip = (skip_mode & (SKIP_NORMAL | SKIP_TO_EOP)) || ctrl_pressed_status;
            if ( script_h.getStringBuffer()[ string_buffer_offset ] == 'd' ) flag = true;
            string_buffer_offset++;
            int t = 0;
            while( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                   script_h.getStringBuffer()[ string_buffer_offset ] <= '9' ){
                t = t*10 + script_h.getStringBuffer()[ string_buffer_offset ] - '0';
                string_buffer_offset++;
            }
            if (in_txtbtn)
                terminateTextButton();
            flush(refreshMode());
            if ( flag && in_skip) {
                if (!clickskippage_flag)
                    skip_mode &= ~(SKIP_TO_EOL | SKIP_TO_WAIT);
            }
            else{
                if (!flag && in_skip) {
                    //Mion: instead of skipping entirely, let's do a shortened wait (safer)
                    if (t > 100) {
                        t = t / 10;
                    } else if (t > 10) {
                        t = 10;
                    }
                }
                if (!clickskippage_flag)
                    skip_mode &= ~(SKIP_TO_EOL | SKIP_TO_WAIT);
                event_mode |= WAIT_TEXTOUT_MODE;
                if ( flag ) event_mode |= WAIT_INPUT_MODE;
                key_pressed_flag = false;
                waitEvent(t);
            }
        }
        else{
            string_buffer_offset--;
            goto notacommand;
        }
        return true;
    }
    else if ( ch == '#' ){
         char hexchecker;
         for ( int tmpctr = 0; tmpctr <= 5; tmpctr++) {
             hexchecker = script_h.getStringBuffer()[ string_buffer_offset+tmpctr+1 ];
             if(!((hexchecker >= '0' && hexchecker <= '9') || (hexchecker >= 'a' && hexchecker <= 'f') || (hexchecker >= 'A' && hexchecker <= 'F'))) goto notacommand;
         }
        readColor( &sentence_font.color, script_h.getStringBuffer() + string_buffer_offset );
        readColor( &ruby_font.color, script_h.getStringBuffer() + string_buffer_offset );
        string_buffer_offset += 7;
        return true;
    }
    else if ( ch == '(' ){
        current_page->add('(');
        startRuby( script_h.getStringBuffer() + string_buffer_offset + 1, sentence_font );
        sentence_font.addLineOffset(ruby_struct.margin);
        string_buffer_offset++;
        return true;
    }
    else if ( ch == '/'){
        if ( ruby_struct.stage == RubyStruct::BODY ){
            current_page->add('/');
            sentence_font.addLineOffset(ruby_struct.margin);
            string_buffer_offset = ruby_struct.ruby_end - script_h.getStringBuffer();
            if (*ruby_struct.ruby_end == ')'){
                if ( skip_mode & (SKIP_NORMAL | SKIP_TO_EOP) || ctrl_pressed_status )
                    endRuby(false, true, accumulation_surface, &text_info);
                else
                    endRuby(true, true, accumulation_surface, &text_info);
                current_page->add(')');
                string_buffer_offset++;
            }

            return true;
        }
        else if (script_h.getStringBuffer()[string_buffer_offset+1] != 0x00)
            goto notacommand;
        else{ // skip new line
            new_line_skip_flag = true;
            string_buffer_offset++;
            if (in_txtbtn)
                terminateTextButton();
            if (script_h.getStringBuffer()[string_buffer_offset] != 0x00)
                errorAndExit( "'new line' must follow '/'." );
            return true; // skip the following eol
        }
    }
    else if ( ch == ')' && ruby_struct.stage == RubyStruct::BODY ){
        current_page->add(')');
        string_buffer_offset++;
        ruby_struct.stage = RubyStruct::NONE;
        return true;
    }
    else if ( ch == ScriptHandler::TXTBTN_START ) { //begins a textbutton
        if (!in_txtbtn) {
            textbtnColorChange();
            text_button_info.insert(new TextButtonInfoLink);
            text_button_info.next->xy[0] = sentence_font.xy[0];
            text_button_info.next->xy[1] = sentence_font.xy[1];
            string_buffer_offset++;
            // need to read in optional integer value
            int num = 0;
            while ( script_h.getStringBuffer()[ string_buffer_offset ] >= '0' &&
                    script_h.getStringBuffer()[ string_buffer_offset ] <= '9' ) {
                num *= 10;
                num += script_h.getStringBuffer()[ string_buffer_offset ] - '0';
                string_buffer_offset++;
            }
            if (num > 0) {
                next_txtbtn_num = num;
                text_button_info.next->no = num;
            } else {
                text_button_info.next->no = next_txtbtn_num++;
            }
            text_button_info.next->prtext = current_page->text
                + current_page->text_count;
            text_button_info.next->text = script_h.getStringBuffer() + 
                string_buffer_offset;
            in_txtbtn = true;
        } else
            string_buffer_offset++;
        return true;
    }
    else if ( ch == ScriptHandler::TXTBTN_END ) {   //ends a textbutton
        if (in_txtbtn) {
            char *tmptext;
            int txtbtn_len = script_h.getStringBuffer() + string_buffer_offset
                - text_button_info.next->text;
            tmptext = new char[txtbtn_len + 1];
            strncpy(tmptext, text_button_info.next->text, txtbtn_len);
            tmptext[txtbtn_len] = '\0';
            text_button_info.next->text = tmptext;
            txtbtn_len = current_page->text + current_page->text_count
                - text_button_info.next->prtext;
            tmptext = new char[txtbtn_len + 1];
            strncpy(tmptext, text_button_info.next->prtext, txtbtn_len);
            tmptext[txtbtn_len] = '\0';
            text_button_info.next->prtext = tmptext;
            textbtnColorChange();
            in_txtbtn = false;
        }
        string_buffer_offset++;
        return true;
    }
    else{
        notacommand: //insani

        line_has_nonspace = true;
        out_text[0] = ch;

        if ((string_buffer_offset == 0) && (script_h.getStringBuffer()[1] == 0)) {
            // For now, when in double-byte mode, don't output single-byte
            // text unless string_buffer has more than 1 character
            // (temporary Umineko8 bugfix - Uncle Mion, March 8 2011) FIXME
            new_line_skip_flag = old_new_line_skip_flag;
            string_buffer_offset++;
            return false;
        }
        last_textpos_xy[0] = sentence_font.x()-sentence_font.ruby_offset_xy[0];
        last_textpos_xy[1] = sentence_font.y()-sentence_font.ruby_offset_xy[1];
        bool flush_flag = !(skip_mode || ctrl_pressed_status || (sentence_font.wait_time == 0));
        drawChar( out_text, &sentence_font, flush_flag, true, accumulation_surface, &text_info );
        num_chars_in_sentence++;
        string_buffer_offset++;
        if (flush_flag){
            if (!((skip_mode & (SKIP_TO_WAIT | SKIP_TO_EOL)) ||
                  (sentence_font.wait_time == 0)) ){
                event_mode = WAIT_TEXTOUT_MODE;
                if ( sentence_font.wait_time == -1 )
                    waitEvent( default_text_speed[text_speed_no] );
                else
                    waitEvent( sentence_font.wait_time );
            }
        }
        return true;
    }

    return false;
}

char ONScripterLabel::doLineBreak(bool isHardBreak)
// Mion: for text processing
{
    sentence_font.newLine();
    /* Mion: working on proper handling for "soft" page breaks,
         but commented out for now until log text processing
         can do intelligent linebreaking */
    //if (isHardBreak) {
        //add break char to the page log
        current_page->add( 0x0a );
        for (int i=0 ; i<indent_offset ; i++){
            current_page->add(0x81);
            current_page->add(0x40);
            sentence_font.advanceCharInHankaku(2);
        }
    //}
    line_has_nonspace = false;
    char ch = script_h.getStringBuffer()[string_buffer_offset];
    // skip leading spaces after a newline
    while (ch == ' ')
        ch = script_h.getStringBuffer()[++string_buffer_offset];
    return ch;
}

int ONScripterLabel::isTextCommand(const char *buf)
// Mion: for text processing
// if buf starts with a text command, return the # of chars in it
// if it's a ruby command, return -(# of chars)
// return 0 if not a text command
{
    int offset = 0;
    if (script_h.checkClickstr(buf) == -2) {
        // got the special "\@" clickwait-or-page
        return 2;
    }
    else if (*buf == '`') {
        return 1;
    }
    else if ((*buf == '@') || (*buf == '\\')) {
        // clickwait, new page
        return 1;
    }
    else if ( *buf == ScriptHandler::TXTBTN_START ||
              *buf == ScriptHandler::TXTBTN_END ){
        return 1;
    }
    else if ( *buf == '/') {
        if (buf[1] == 0x0a)
            return 1;
        else
            return 0;
    }
    else if ( *buf == '!' ){
        offset++;
        if ( buf[offset] == 's' ){
            offset++;
            if ( buf[offset] == 'd' ){
                offset++;
            }
            else{
                while( buf[offset] >= '0' && buf[offset] <= '9' ){
                    offset++;
                }
            }
        }
        else if ( buf[offset] == 'w' || buf[offset] == 'd' ){
            offset++;
            while( buf[offset] >= '0' && buf[offset] <= '9' ){
                offset++;
            }
        } else
            return 0;
        return offset;
    }
    else if ( *buf == '#' ){
         char hexchecker;
         for ( int tmpctr = 1; tmpctr <= 6; tmpctr++) {
             hexchecker = buf[tmpctr];
             if (!((hexchecker >= '0' && hexchecker <= '9') ||
                 (hexchecker >= 'a' && hexchecker <= 'f') ||
                 (hexchecker >= 'A' && hexchecker <= 'F')))
                 return 0;
         }
         return 7;
    }
    else if ( *buf == '(' && rubyon_flag) {
        // if ruby command, return as -length to indicate ruby
        bool found_slash = false;
        while (*buf != ')' && *buf != '\0') {
            if (IS_TWO_BYTE(*buf)) {
                buf++;
                offset--;
            } else if (*buf == '/') {
                found_slash = true;
            }
            buf++;
            offset--;
        }
        if (*buf == ')' && found_slash) {
            return --offset;
        } else
            return 0;
    }
    else
        return 0;
}

void ONScripterLabel::processRuby(unsigned int i, int cmd)
{
    char *string_buffer = script_h.getStringBuffer();
    unsigned int j, k;
    // ruby - find the margins
    startRuby( string_buffer + i + 1, sentence_font );
    string_buffer_margins[i] = ruby_struct.margin;
    string_buffer_margins[i-cmd-1] = ruby_struct.margin;
    ruby_struct.stage = RubyStruct::NONE;
    //printf("ruby margin: %d; at %s", string_buffer_margins[i], string_buffer+i);
    j = i + -cmd;
    // no breaking within a ruby
    for (k=i+1; k<j; k++) {
        string_buffer_breaks[k] = false;
    }
}

bool ONScripterLabel::processBreaks(bool cont_line, LineBreakType style)
// Mion: for text processing
// cont_line: is this a continuation of a prior line (using "/")?
// style: SPACEBREAK or KINSOKU linebreak rules
{
    char *string_buffer = script_h.getStringBuffer();
    unsigned int i=0, j=0;
    unsigned int len = strlen(string_buffer);
    int cmd=0;
    bool return_val;
    bool is_ruby = false;
    if (string_buffer_breaks) delete[] string_buffer_breaks;
    string_buffer_breaks = new bool[len+2];
    if (string_buffer_margins) delete[] string_buffer_margins;
    string_buffer_margins = new char[len];
    for (i=0; i<len; i++) string_buffer_margins[i] = 0;

    i = 0;
    // first skip past starting text commands
    do {
        cmd = isTextCommand(string_buffer + i);
        if (cmd > 0) i += cmd;
    } while (cmd > 0);
    // don't allow break before first char unless it's a continuation
    if (cont_line)
        string_buffer_breaks[i] = true;
    else {
        string_buffer_breaks[i] = false;
        //printf("Can't break before:%s", string_buffer + i);
    }
    if (cmd < 0) {
        // at a ruby command
        processRuby(i,cmd);
    }

    if (style == KINSOKU) {
        // straight kinsoku, using current kinsoku char sets
        return_val = false; // does it contain any printable ASCII?
        while (i<strlen(string_buffer)) {
            is_ruby = false;
            if (cmd < 0) {
                is_ruby = true;
                j = -cmd;
                cmd = 0;
            } 
            else {
                j = (IS_TWO_BYTE(string_buffer[i])) ? 2 : 1;
                do {
                    cmd = isTextCommand(string_buffer + i + j);
                    // skip over regular text commands
                    if (cmd > 0) j += cmd;
                } while (cmd > 0);
            }
            if ((cmd >= 0) && ((unsigned char) string_buffer[i+j] < 0x80) &&
                !(string_buffer[i+j] == ' ' || string_buffer[i+j] == 0x00 ||
                  string_buffer[i+j] == 0x0a)) {
                return_val = true;
                //printf("Found ASCII at %s", string_buffer + i + j);
            }
            if (cmd < 0) {
                if (is_ruby || !isEndKinsoku(string_buffer + i))
                    string_buffer_breaks[i+j] = true;
                else
                    string_buffer_breaks[i+j] = false;
            }
            else if (isEndKinsoku(string_buffer + i) ||
                (!is_ruby && isStartKinsoku(string_buffer + i + j))) {
                // don't break before start-kinsoku or after end-kinsoku
                string_buffer_breaks[i+j] = false;
                //printf("Can't break before:%s", string_buffer + i + j);
            } else {
                if ((cmd >= 0) && ((unsigned char) string_buffer[i+j] < 0x80) &&
                    !(string_buffer[i+j] == ' ' || string_buffer[i+j] == 0x00 ||
                      string_buffer[i+j] == 0x0a)) {
                    // treat standard ASCII as start-kinsoku,
                    // except for space or line-end
//                    printf("Can't break before:%s", string_buffer + i + j);
                    string_buffer_breaks[i+j] = false;
                }
                else
                    string_buffer_breaks[i+j] = true;
            }
            i += j;
            if (cmd < 0) {
                // at a ruby command
                processRuby(i,cmd);
            }
        }
        return return_val;
    }
    else { // style == SPACEBREAK
        // straight space-breaking
        bool return_val = false; // does it contain space?
        while (i<strlen(string_buffer)) {
            is_ruby = false;
            if (cmd < 0) {
                is_ruby = true;
                j = -cmd;
            } else 
                j = (IS_TWO_BYTE(string_buffer[i])) ? 2 : 1;
            bool had_wait = false;
            do {
                cmd = isTextCommand(string_buffer + i + j);
                if (string_buffer[i+j] == '@' || string_buffer[i+j] == '\\')
                    had_wait = true;
                if (cmd > 0) j += cmd;
            } while (cmd > 0);
            if (cmd < 0) {
                string_buffer_breaks[i+j] = false;
            }
            else if (!IS_TWO_BYTE(string_buffer[i+j]) &&
                (string_buffer[i+j] == 0x0a ||
                 string_buffer[i+j] == 0x00 ||
                 string_buffer[i+j] == '/' ||
                 ((string_buffer[i+j] == ' ' || string_buffer[i+j] == '\t') &&
                  (had_wait || !(string_buffer[i] == ' ' || string_buffer[i] == '\t'))))) {
                // allow break before newline or line-cont command
                // allow a break before a run of spaces/tabs but not within,
                // except to allow break after a clickwait/newpage within the run
                //printf("Can break before %s", string_buffer + i + j);
                string_buffer_breaks[i+j] = true;
                if (string_buffer[i+j] == ' ' || string_buffer[i+j] == '\t') {
                    return_val = true;
                }
            }
            else if ((unsigned char) string_buffer[i] == 0x81 &&
                     string_buffer[i+1] == 0x40) {
                // allow breaks _after_ fullwidth spaces
                string_buffer_breaks[i+j] = true;
            } else {
                string_buffer_breaks[i+j] = false;
                if ((unsigned char) string_buffer[i+j] < 0x80) {
                    //printf("found ASCII: %d", string_buffer[i + j]);
                    return_val = true; // found ASCII
                }
            }
            i += j;
            if (cmd < 0) {
                // at a ruby command
                processRuby(i,cmd);
            }
        }
        return return_val;
    }
}

int ONScripterLabel::findNextBreak(int offset, int &len)
// Mion: for text processing
{
    // return offset of first break_before after/including current offset;
    // use len to return # of printed chars between (in half-width chars)
    char *string_buffer = script_h.getStringBuffer();
    int i = 0, cmd = 0, ruby_end = 0;
    bool in_ruby = false;
    len = 0;

    while (i<offset) {
        // skip over text commands, chars until offset reached
        cmd = isTextCommand(string_buffer + i);
        if (cmd > 0) {
            i += cmd;
        } else if (cmd < 0) {
            if ((i - cmd) > offset) {
                //starting offset is inside a ruby body
                in_ruby = true;
                ruby_end = i - cmd;
                cmd = 0;
                i++;
            } else {
                i -= cmd;
            }
        } else
            i += (IS_TWO_BYTE(string_buffer[i])) ? 2 : 1;
    }

    while (i<(int)(strlen(string_buffer)+2)) {
        if (in_ruby && (string_buffer[i] == '/')) {
            // done with ruby body; skip past ruby command
            len += 3; // seems we need to do this to match Nscr
            i = ruby_end;
            in_ruby = false;
            ruby_end = 0;
        }
        if (!in_ruby) {
            // don't look for text commands while inside a ruby
            do {
                cmd = isTextCommand(string_buffer + i);
                if (cmd > 0) i += cmd;
            } while (cmd > 0);
            if (cmd < 0) {
                //printf("found ruby: %s\n", string_buffer + i);
                in_ruby = true;
                ruby_end = i - cmd;
            }
        }
        if (string_buffer_breaks[i]) {
            return i;
        } else if (cmd < 0) {
            // skip the begin paren of a ruby
            i++;
            cmd = 0;
        } else if (IS_TWO_BYTE(string_buffer[i])) {
            i += 2;
            len += 2;
        } else {
            i++;
            len++;
        }
    }
    // didn't find a break
    return i;
}

void ONScripterLabel::terminateTextButton()
//Mion: use to destroy improperly terminated text button
{
    TextButtonInfoLink *tmp = text_button_info.next;
    tmp->text = tmp->prtext = NULL;
    text_button_info.next = tmp->next;
    delete tmp;
    in_txtbtn = false;
    textbtnColorChange();
}

void ONScripterLabel::textbtnColorChange()
//Mion: swap linkcolor[0] with sentence_font color, make color change
{
    uchar3 tmpcolor;
    setColor(tmpcolor, linkcolor[0]);
    setColor(linkcolor[0], sentence_font.color);
    setColor(sentence_font.color, tmpcolor);
    setColor(ruby_font.color, tmpcolor);
}
