/* -*- C++ -*-
 *
 *  FontInfo.cpp - Font information storage class of ONScripter-EN
 *
 *  Copyright (c) 2001-2007 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2008 "Uncle" Mion Sonozaki
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

#include "FontInfo.h"
#include "Encoding.h"
#include <stdio.h>
#include <SDL_ttf.h>

#define FI_TEST

static struct FontContainer{
    FontContainer *next;
    int size;
    TTF_Font *font;

    FontContainer(){
        size = 0;
        next = NULL;
        font = NULL;
    };
} root_font_container;

Fontinfo::Fontinfo()
{
    ttf_font = NULL;

    color[0]        = color[1]        = color[2]        = 0xff;
    on_color[0]     = on_color[1]     = on_color[2]     = 0xff;
    off_color[0]    = off_color[1]    = off_color[2]    = 0xaa;
    nofile_color[0] = 0x55;
    nofile_color[1] = 0x55;
    nofile_color[2] = 0x99;

    reset();
}

void Fontinfo::reset()
{
    tateyoko_mode = YOKO_MODE;
    rubyon_flag = false;
    ruby_offset_xy[0] = ruby_offset_xy[1] = 0;
    clear();

    is_bold = true;
    is_shadow = true;
    is_transparent = true;
    is_newline_accepted = false;
}

void *Fontinfo::openFont( char *font_file, int ratio1, int ratio2 )
{
    int font_size;
    if ( font_size_xy[0] < font_size_xy[1] )
        font_size = font_size_xy[0];
    else
        font_size = font_size_xy[1];

    FontContainer *fc = &root_font_container;
    while( fc->next ){
        if ( fc->next->size == font_size ) break;
        fc = fc->next;
    }
    if ( !fc->next ){
        fc->next = new FontContainer();
        fc->next->size = font_size;
        FILE *fp = fopen( font_file, "r" );
        if ( fp == NULL ) return NULL;
        fclose( fp );
        //printf("Opening font.\tSize (full-width): %d\tEffective size: %d\n", font_size, font_size * ratio1 / ratio2);
        fc->next->font = TTF_OpenFont( font_file, font_size * ratio1 / ratio2 );
    }

    ttf_font = (void*)fc->next->font;

    return fc->next->font;
}

void Fontinfo::setTateyokoMode( int tateyoko_mode )
{
    this->tateyoko_mode = tateyoko_mode;
    clear();
}

int Fontinfo::getTateyokoMode()
{
    return tateyoko_mode;
}

int Fontinfo::getRemainingLine()
{
    // TODO needs changing - num_xy is total number of rows and
    // columns in text window - needs to work in pixels
    if (tateyoko_mode == YOKO_MODE)
        return num_xy[1] - xy[1]/2;
    else
        return num_xy[1] - num_xy[0] + xy[0]/2 + 1;
}

int Fontinfo::x(int encoding)
{
    if (encoding == Encoding::CODE_CP932)
        // Multiplies current column by character pixel count to get offset
        return xy[0]*pitch_xy[0]/2 + top_xy[0] + line_offset_xy[0] + ruby_offset_xy[0];
    else
        return xy[0] + top_xy[0] + line_offset_xy[0] + ruby_offset_xy[0];
}

int Fontinfo::y()
{
    // Multiplies current row count by character pixel count (halved to get hw) to get offset
    // Since SJIS behaviour will be unchanged, it's unlikely that tateyoko will ever be used.
    // For this reason, at least for now, I'm still going to count xy[1] in rows instead of
    // pixels like xy[0].
    return xy[1]*pitch_xy[1]/2 + top_xy[1] + line_offset_xy[1] + ruby_offset_xy[1];
}

void Fontinfo::setXY( int x, int y )
{
    // Is it worth changin the doubling multiplier for UTF-8 mode? It
    // would mean rewriting sections which call setXY and other XY-
    // related functions, but it would make it clearer to read code...
    if ( x != -1 ) xy[0] = x*2;
    if ( y != -1 ) xy[1] = y*2;
}

void Fontinfo::clear()
{
    if (tateyoko_mode == YOKO_MODE)
        setXY(0, 0);
    else
        setXY(num_xy[0]-1, 0);
    line_offset_xy[0] = line_offset_xy[1] = 0;
    setRubyOnFlag(rubyon_flag);
}

void Fontinfo::newLine()
{
    if (tateyoko_mode == YOKO_MODE){
        xy[0] = 0;
        xy[1] += 2;
    }
    else{
        xy[0] -= 2;
        xy[1] = 0;
    }
    line_offset_xy[0] = line_offset_xy[1] = 0;
}

void Fontinfo::setLineArea(int num)
{
    // Doesn't require fix; UTF-8 will always be yoko
    num_xy[tateyoko_mode] = num;
    num_xy[1-tateyoko_mode] = 1;
}

bool Fontinfo::isEndOfLine(int margin)
{
    if (pitch_xy[tateyoko_mode] == 0)
        return false;

    // It's unnecessary to "fix" this (or at least I think it is)
    // because tateyoko mode will always be 0 in UTF-8 mode, and
    // it already looks like it uses px
    int offset = 2 * line_offset_xy[tateyoko_mode] / pitch_xy[tateyoko_mode];
    if (((2 * line_offset_xy[tateyoko_mode]) % pitch_xy[tateyoko_mode]) > 0)
        offset++;
    if (xy[tateyoko_mode] + offset + margin >= num_xy[tateyoko_mode]*2)
        return true;

    return false;
}

bool Fontinfo::isLineEmpty()
{
    if (xy[tateyoko_mode] == 0) return true;

    return false;
}

void Fontinfo::advanceCharInHankaku(int offset)
{
    xy[tateyoko_mode] += offset;
}

void Fontinfo::addLineOffset(int offset)
{
    //TODO make caller functions encoding-aware
    line_offset_xy[tateyoko_mode] += offset;
}

void Fontinfo::setRubyOnFlag(bool flag)
{
    rubyon_flag = flag;
    ruby_offset_xy[0] = ruby_offset_xy[1] = 0;
    if (rubyon_flag && tateyoko_mode == TATE_MODE)
	ruby_offset_xy[0] = font_size_xy[0]-pitch_xy[0];
    if (rubyon_flag && tateyoko_mode == YOKO_MODE)
        //Nice, I'm pretty sure this already uses pixels :D
	ruby_offset_xy[1] = pitch_xy[1] - font_size_xy[1];
}

SDL_Rect Fontinfo::calcUpdatedArea(int start_xy[2], int ratio1, int ratio2, int encoding)
{
    SDL_Rect rect;

    if (tateyoko_mode == YOKO_MODE){
        if (start_xy[1] == xy[1]){
            if (encoding == Encoding::CODE_CP932) {
                rect.x = top_xy[0] + pitch_xy[0]*start_xy[0]/2;
                rect.w = pitch_xy[0]*(xy[0]-start_xy[0])/2+1;
            } else {
                rect.x = top_xy[0] + start_xy[0];
                rect.w = xy[0] - start_xy[0] + 1;
                // I'm keeping the +1 because I don't know what it does
            }
        }
        else{
            rect.x = top_xy[0];
            if (encoding == Encoding::CODE_CP932) {
                rect.w = pitch_xy[0]*num_xy[0];
            } else {
                // My rehashed analysis: top_xy is based on Fontinfo::xy
                // and so it will naturally contain either columns or px
                // depending on the encoding in use :D
                rect.w = xy[0]-start_xy[0];
            }
        }
        if (encoding == Encoding::CODE_CP932) {
            rect.y = top_xy[1] + start_xy[1]*pitch_xy[1]/2;
            rect.h = pitch_xy[1]*(xy[1]-start_xy[1]+2)/2;
        } else {
            // I think this still holds true since I haven't changed how
            // Y values work (that is, counting in rows instead of px)
            // -Galladite 2023-6-23
            rect.y = top_xy[1] + start_xy[1]*pitch_xy[1]/2;
            rect.h = pitch_xy[1]*(xy[1]-start_xy[1]+2)/2;
        }
    }
    // This can stay the same; it only applies to SJIS mode
    else{
        rect.x = top_xy[0] + pitch_xy[0]*xy[0]/2;
        rect.w = pitch_xy[0]*(start_xy[0]-xy[0]+2)/2;
        if (start_xy[0] == xy[0]){
            rect.y = top_xy[1] + pitch_xy[1]*start_xy[1]/2;
            rect.h = pitch_xy[1]*(xy[1]-start_xy[1])/2+1;
        }
        else{
            rect.y = top_xy[1];
            rect.h = pitch_xy[1]*num_xy[1];
        }
        num_xy[0] = (xy[0]-start_xy[0])/2+1;
    }

    rect.x = rect.x * ratio1 / ratio2;
    rect.y = rect.y * ratio1 / ratio2;
    if (((rect.w*ratio1)%ratio2)==0)
        rect.w = rect.w * ratio1 / ratio2;
    else
        rect.w = rect.w * ratio1 / ratio2 + 1;
    if (((rect.h*ratio1)%ratio2)==0)
        rect.h = rect.h * ratio1 / ratio2;
    else
        rect.h = rect.h * ratio1 / ratio2 + 1;

    return rect;
}

void Fontinfo::addShadeArea(SDL_Rect &rect, int shade_distance[2])
{
    if (is_shadow){
        if (shade_distance[0]>0)
            rect.w += shade_distance[0];
        else{
            rect.x += shade_distance[0];
            rect.w -= shade_distance[0];
        }
        if (shade_distance[1]>0)
            rect.h += shade_distance[1];
        else{
            rect.y += shade_distance[1];
            rect.h -= shade_distance[1];
        }
    }
}

//TODO: functions that call this func should be aware of giving ints in px
//TODO: also the return value (margin) will be given in px
int Fontinfo::initRuby(Fontinfo &body_info, int body_count, int ruby_count, int encoding)
{
    //Uses px I think
    top_xy[0] = body_info.x(encoding);
    top_xy[1] = body_info.y();
    pitch_xy[0] = font_size_xy[0];
    pitch_xy[1] = font_size_xy[1];

    int margin=0;

    if (tateyoko_mode == YOKO_MODE){
        //Shift 1 row up in px
        top_xy[1] -= font_size_xy[1];
        //Set number of x and y chars available
        num_xy[0] = ruby_count;
        num_xy[1] = 1;
    }
    else{
        top_xy[0] += body_info.font_size_xy[0];
        num_xy[0] = 1;
        num_xy[1] = ruby_count;
    }

    if (encoding == Encoding::CODE_UTF8) {
        //TODO: may need to be reverted. This has been changed to assume body_count and ruby_count in px
        if (ruby_count >= body_count){
            margin = (ruby_count - body_count + 1)/2;
        }
        else{
            int offset = 0;
            if (ruby_count > 0)
                //TODO: double-check this maths stays true in px
                offset = (body_count - ruby_count) / ruby_count;
            top_xy[0] += (offset+1)/2; //Ig to prevent truncation
            pitch_xy[0] += offset;
        }
    } else {
        //SJIS
        if (ruby_count*font_size_xy[tateyoko_mode] >= body_count*body_info.pitch_xy[tateyoko_mode]){
            margin = (ruby_count*font_size_xy[tateyoko_mode] - body_count*body_info.pitch_xy[tateyoko_mode] + 1)/2;
        }
        else{
            int offset = 0;
            if (ruby_count > 0)
                offset = (body_count*body_info.pitch_xy[tateyoko_mode] - ruby_count*font_size_xy[tateyoko_mode]) / ruby_count;
            top_xy[tateyoko_mode] += (offset+1)/2;
            pitch_xy[tateyoko_mode] += offset;
        }
    }
    //body_info.line_offset_xy[tateyoko_mode] += margin;

    clear();

    return margin;
}
