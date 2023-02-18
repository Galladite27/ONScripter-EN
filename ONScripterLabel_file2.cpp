/* -*- C++ -*-
 *
 *  ONScripterLabel_file2.cpp - FILE I/O of ONScripter-EN
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

// Modified by Mion, March 2008, to update from
// Ogapee's 20080121 release source code.

// Modified by Mion, April 2009, to update from
// Ogapee's 20090331 release source code.

#include "ONScripterLabel.h"

int ONScripterLabel::loadSaveFile2( int file_version, bool input_flag )
{
//printf("loading file, version %d\n", file_version);

    if ( input_flag )
        deleteNestInfo();
    
    int i, j;
    char *str = NULL;
    
    readInt(); // 1
    if ( !input_flag ) {
        readInt();
        readInt();
    } else {
        if ( readInt() == 1 ) sentence_font.is_bold = true;
        else                  sentence_font.is_bold = false;
        if ( readInt() == 1 ) sentence_font.is_shadow = true;
        else                  sentence_font.is_shadow = false;
    }

    readInt(); // 0
    if ( !input_flag ) {
        for (i=0; i<4; i++)
            readInt();
    } else {
        rmode_flag = (readInt()==1)?true:false;
        sentence_font.color[0] = readInt();
        sentence_font.color[1] = readInt();
        sentence_font.color[2] = readInt();
        cursor_info[0].remove();
        readStr( &cursor_info[0].image_name );
        if ( cursor_info[0].image_name ){
            parseTaggedString( &cursor_info[0] );
            setupAnimationInfo( &cursor_info[0] );
            if ( cursor_info[0].image_surface )
                cursor_info[0 ].visible = true;
        }
        cursor_info[1].remove();
        readStr( &cursor_info[1].image_name );
        if ( cursor_info[1].image_name ){
            parseTaggedString( &cursor_info[1] );
            setupAnimationInfo( &cursor_info[1] );
            if ( cursor_info[1].image_surface )
                cursor_info[1 ].visible = true;
        }
    }

    if ( !input_flag ) {
        readInt();
        readInt();
        readStr(&str);
    } else {
        window_effect.effect = readInt();
        window_effect.duration = readInt();
        readStr( &window_effect.anim.image_name ); // probably
    }

    if ( !input_flag ) {
        for (i=0; i<8; i++)
            readInt();
        for (i=0; i<4; i++)
            readChar();
        for (i=0; i<5; i++)
            readInt();
        readStr(&str);
    } else {
        sentence_font.clear();
        sentence_font.ttf_font  = NULL;
        sentence_font.top_xy[0] = readInt();
        sentence_font.top_xy[1] = readInt();
#ifndef RCA_SCALE
        sentence_font.num_xy[0] = readInt();
        sentence_font.num_xy[1] = readInt();
#else
        sentence_font.num_xy[0] = readInt() * scr_stretch_x + 0.5;
        sentence_font.num_xy[1] = readInt() * scr_stretch_y + 0.5;
#endif
        sentence_font.font_size_xy[0] = readInt();
        sentence_font.font_size_xy[1] = readInt();
        sentence_font.pitch_xy[0] = readInt();
        sentence_font.pitch_xy[1] = readInt();
        for ( i=0 ; i<3 ; i++ )
            sentence_font.window_color[2-i] = readChar();
        if ( readChar() == 0x00 ) sentence_font.is_transparent = true;
        else                      sentence_font.is_transparent = false;
        sentence_font.wait_time = readInt();
        sentence_font_info.orig_pos.x = readInt();
        sentence_font_info.orig_pos.y = readInt();
        UpdateAnimPosXY(&sentence_font_info);
        sentence_font_info.orig_pos.w = readInt() + 1 - sentence_font_info.orig_pos.x;
        sentence_font_info.orig_pos.h = readInt() + 1 - sentence_font_info.orig_pos.y;
        UpdateAnimPosStretchWH(&sentence_font_info);
        readStr( &sentence_font_info.image_name );
        if ( !sentence_font.is_transparent && sentence_font_info.image_name ){
            parseTaggedString( &sentence_font_info );
            setupAnimationInfo( &sentence_font_info );
        }
    }

    if ( !input_flag ) {
        for (i=0; i<6; i++)
            readInt();
    } else {
        if ( readInt() == 1 ) cursor_info[0].abs_flag = false;
        else                  cursor_info[0].abs_flag = true;
        if ( readInt() == 1 ) cursor_info[1].abs_flag = false;
        else                  cursor_info[1].abs_flag = true;
        cursor_info[0].orig_pos.x = readInt();
        cursor_info[1].orig_pos.x = readInt();
        cursor_info[0].orig_pos.y = readInt();
        cursor_info[1].orig_pos.y = readInt();
        UpdateAnimPosXY(&cursor_info[0]);
        UpdateAnimPosXY(&cursor_info[1]);
    }

    // load background surface
    if ( !input_flag ) {
        readStr(&str);
    } else {
        bg_info.remove();
        readStr( &bg_info.file_name );
        createBackground();
    }

    if ( !input_flag ) {
        for ( i=0 ; i<3; i++ )
            readStr(&str);
        for ( i=0 ; i<6; i++ )
            readInt();
    } else {
        for ( i=0 ; i<3 ; i++ ){
            tachi_info[i].remove();
            readStr( &tachi_info[i].image_name );
            if ( tachi_info[i].image_name ){
                parseTaggedString( &tachi_info[i] );
#ifdef RCA_SCALE
                if (scr_stretch_y > 1.0) {
                    // RCA: Stretch characters to screen size.
                    // Note stretches are with Y-scale, so they don't get distorted
                    setupAnimationInfo( &tachi_info[ i ], NULL, scr_stretch_y, scr_stretch_y );
                } else
#endif
                setupAnimationInfo( &tachi_info[i] );
            }
        }

        for ( i=0 ; i<3 ; i++ ) {
            tachi_info[i].orig_pos.x = readInt() + tachi_info[i].orig_pos.w / 2;
        }

        for ( i=0 ; i<3 ; i++ ) {
            tachi_info[i].orig_pos.y = readInt() + tachi_info[i].orig_pos.h;
            UpdateAnimPosStretchXY(&tachi_info[i]);
            tachi_info[i].orig_pos.x -= tachi_info[i].orig_pos.w / 2;
            tachi_info[i].orig_pos.y -= tachi_info[i].orig_pos.h;
            tachi_info[i].pos.x -= tachi_info[i].pos.w / 2;
            tachi_info[i].pos.y -= tachi_info[i].pos.h;
        }
    }

    readInt(); // 0
    readInt(); // 0
    readInt(); // 0

    if (file_version >= 203){
        readInt(); // -1
        readInt(); // -1
        readInt(); // -1
    }
    
    int max_sprite_num = MAX_SPRITE_NUM;
    //if (file_version < 200) max_sprite_num = MAX_SPRITE2_NUM;
    if ( !input_flag ) {
        for ( i=0 ; i<max_sprite_num ; i++ ){
            readStr(&str);
            for (j=0; j<4; j++)
                readInt();
            if (file_version >= 203)
                readInt();
        }
    } else {
        for ( i=0 ; i<max_sprite_num ; i++ ){
            sprite_info[i].remove();
            readStr( &sprite_info[i].image_name );
            if ( sprite_info[i].image_name ){
                parseTaggedString( &sprite_info[i] );
#ifdef RCA_SCALE
                setupAnimationInfo( &sprite_info[i], NULL, scr_stretch_x, scr_stretch_y );
#else
                setupAnimationInfo( &sprite_info[i] );
#endif
            }
            sprite_info[i].orig_pos.x = readInt();
            sprite_info[i].orig_pos.y = readInt();
            UpdateAnimPosStretchXY(&sprite_info[i]);
            if ( readInt() == 1 ) sprite_info[i].visible = true;
            else                  sprite_info[i].visible = false;
            sprite_info[i].current_cell = readInt();
            if (file_version >= 203) {
                j = readInt();
                if (j == -1)
                    j = 256;
                sprite_info[i].trans = j;
            }
        }
    }

    if ( !input_flag ) {
        for (i=0 ; i<script_h.global_variable_border ; i++){
            readInt();
            readStr(&str);
        }
    } else {
        readVariables( 0, script_h.global_variable_border );
    }

    // nested info
    int num_nest =readInt();
    if ( input_flag ) last_nest_info = &root_nest_info;
    if (num_nest > 0){
        file_io_buf_ptr += (num_nest-1)*4;
        while( num_nest > 0 ){
            if ( !input_flag ) {
                i = readInt();
                if (i > 0){
                    file_io_buf_ptr -= 8;
                    num_nest--;
                }
                else{
                    file_io_buf_ptr -= 16;
                    for (i=0; i<3; i++)
                        readInt();
                    file_io_buf_ptr -= 16;
                    num_nest -= 4;
                }
            } else {
                NestInfo *info = new NestInfo();
                if (last_nest_info == &root_nest_info) last_nest_info = info;
        
                i = readInt();
                if (i > 0){
                    info->nest_mode = NestInfo::LABEL;
                    info->next_script = script_h.getAddress( i );
                    file_io_buf_ptr -= 8;
                    num_nest--;
                }
                else{
                    info->nest_mode = NestInfo::FOR;
                    info->next_script = script_h.getAddress( -i );
                    file_io_buf_ptr -= 16;
                    info->var_no = readInt();
                    info->to = readInt();
                    info->step = readInt();
                    file_io_buf_ptr -= 16;
                    num_nest -= 4;
                }
                info->next = root_nest_info.next;
                if (root_nest_info.next) root_nest_info.next->previous = info;
                root_nest_info.next = info;
                info->previous = &root_nest_info;
            }
        }
        num_nest = readInt();
        file_io_buf_ptr += num_nest*4;
    }

    if ( !input_flag ) {
        for (i=0; i<5; i++)
            readInt();
    } else {
        if (readInt() == 1) monocro_flag = true;
        else                monocro_flag = false;
        for ( i=0 ; i<3 ; i++ )
            monocro_color[2-i] = readInt();
        for ( i=0 ; i<256 ; i++ ){
            monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
            monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
            monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
        }
        nega_mode = readInt();
    }
    
    // ----------------------------------------
    // Sound
    if ( !input_flag ) {
        readStr(&str);
        readStr(&str);
        for (i=0; i<6; i++)
            readInt();
        readStr(&str);
    } else {
        stopCommand();
        loopbgmstopCommand();
        stopAllDWAVE();

        readStr( &seqmusic_file_name ); // MIDI file
        readStr( &wave_file_name ); // wave, waveloop
        i = readInt();
        if ( i >= 0 ) current_cd_track = i;

        // play, playonce MIDI
        if ( readInt() == 1 ){
            seqmusic_play_loop_flag = true;
            current_cd_track = -2;
            playSound(seqmusic_file_name, SOUND_SEQMUSIC, seqmusic_play_loop_flag);
        }
        else
            seqmusic_play_loop_flag = false;

        // wave, waveloop
        if ( readInt() == 1 ) wave_play_loop_flag = true;
        else                  wave_play_loop_flag = false;
        if ( wave_file_name && wave_play_loop_flag )
            playSound(wave_file_name, SOUND_WAVE|SOUND_OGG, wave_play_loop_flag, MIX_WAVE_CHANNEL);

        // play, playonce
        if ( readInt() == 1 ) cd_play_loop_flag = true;
        else                  cd_play_loop_flag = false;
        if ( current_cd_track >= 0 ) playCDAudio();

        // bgm, mp3, mp3loop
        if ( readInt() == 1 ) music_play_loop_flag = true;
        else                  music_play_loop_flag = false;
        if ( readInt() == 1 ) mp3save_flag = true;
        else                  mp3save_flag = false;
        readStr( &music_file_name );
        if ( music_file_name ){
            playSound(music_file_name,
                      SOUND_WAVE | SOUND_OGG_STREAMING | SOUND_MP3 | SOUND_SEQMUSIC,
                      music_play_loop_flag, MIX_BGM_CHANNEL);
        }
    }

    j = readInt();
    if ( input_flag ) erase_text_window_mode = j;
    readInt(); // 1

    if ( input_flag ) barclearCommand();
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        j = readInt();
        if ( input_flag && (j != 0) ){
            bar_info[i] = new AnimationInfo();
            bar_info[i]->trans_mode = AnimationInfo::TRANS_COPY;
            bar_info[i]->num_of_cells = 1;

            bar_info[i]->param = j;
            bar_info[i]->orig_pos.x = readInt();
            bar_info[i]->orig_pos.y = readInt();
            UpdateAnimPosXY(bar_info[i]);
            bar_info[i]->max_width = readInt();
            bar_info[i]->orig_pos.h = readInt();
            bar_info[i]->pos.h = ExpandPos(bar_info[i]->orig_pos.h);
            bar_info[i]->max_param = readInt();
            for ( j=0 ; j<3 ; j++ )
                bar_info[i]->color[2-j] = readChar();
            readChar(); // 0x00

            int w = ExpandPos(bar_info[i]->max_width) * bar_info[i]->param / bar_info[i]->max_param;
            if ( bar_info[i]->max_width > 0 && w > 0 ){
                bar_info[i]->pos.w = w;
                bar_info[i]->allocImage( bar_info[i]->pos.w, bar_info[i]->pos.h );
                bar_info[i]->fill( bar_info[i]->color[0], bar_info[i]->color[1], bar_info[i]->color[2], 0xff );
            }
        }
        else{
            readInt(); // -1
            readInt(); // 0
            readInt(); // 0
            readInt(); // 0
            readInt(); // 0
            readInt(); // 0
        }
    }

    if ( input_flag ) prnumclearCommand();
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        j = readInt();
        if ( input_flag && prnum_info[i] ){
            prnum_info[i] = new AnimationInfo();
            prnum_info[i]->trans_mode = AnimationInfo::TRANS_STRING;
            prnum_info[i]->num_of_cells = 1;
            prnum_info[i]->color_list = new uchar3[1];

            prnum_info[i]->param = j;
            prnum_info[i]->orig_pos.x = readInt();
            prnum_info[i]->orig_pos.y = readInt();
            UpdateAnimPosXY(prnum_info[i]);
            prnum_info[i]->font_size_xy[0] = readInt();
            prnum_info[i]->font_size_xy[1] = readInt();
            for ( j=0 ; j<3 ; j++ )
                prnum_info[i]->color_list[0][2-j] = readChar();
            readChar(); // 0x00

            char num_buf[7];
            script_h.getStringFromInteger( num_buf, prnum_info[i]->param, 3, false, true );
            setStr( &prnum_info[i]->file_name, num_buf );

            setupAnimationInfo( prnum_info[i] );
        }
        else{
            readInt(); // -1
            readInt(); // 0
            readInt(); // 0
            readInt(); // 0
            readInt(); // 0
        }
    }

    readInt(); // 1
    readInt(); // 0
    readInt(); // 1
    if ( !input_flag ) {
        readStr(&str);
    } else {
        btndef_info.remove();
        readStr( &btndef_info.image_name );
        if ( btndef_info.image_name && btndef_info.image_name[0] != '\0' ){
            parseTaggedString( &btndef_info );
            setupAnimationInfo( &btndef_info );
            SDL_SetAlpha( btndef_info.image_surface, DEFAULT_BLIT_FLAG, SDL_ALPHA_OPAQUE );
        }
    }

    if ( file_version >= 202 ) {
        if ( !input_flag ) {
            ScriptHandler::ArrayVariable *av = script_h.getRootArrayVariable();

            while(av){
                int i, dim = 1;
                for ( i=0 ; i<av->num_dim ; i++ )
                    dim *= av->dim[i];
                for ( i=0 ; i<dim ; i++ ){
                    if (file_io_buf_ptr+3 >= file_io_buf_len ) break;
                    file_io_buf_ptr += 4;
                }
                av = av->next;
            }
        } else
            readArrayVariable();
    }

    readInt(); // 0
    if ( readChar() == 1 ) 
        if ( input_flag ) erase_text_window_mode = 2;
    readChar(); // 0
    readChar(); // 0
    readChar(); // 0

    if ( !input_flag ) {
        readStr(&str);
        readStr(&str);
    } else {
        readStr( &loop_bgm_name[0] );
        readStr( &loop_bgm_name[1] );
        if ( loop_bgm_name[0] ) {
            if ( loop_bgm_name[1] )
                playSound(loop_bgm_name[1],
                          SOUND_PRELOAD|SOUND_WAVE|SOUND_OGG, false, MIX_LOOPBGM_CHANNEL1);
            playSound(loop_bgm_name[0],
                      SOUND_WAVE|SOUND_OGG, false, MIX_LOOPBGM_CHANNEL0);
        }
    }

    if ( file_version >= 201 ){
        if ( !input_flag ) {
            for (i=0; i<3; i++)
                readInt();
            readStr(&str);
        } else {
            if ( readInt() == 1 ) rubyon_flag = true;
            else                  rubyon_flag = false;
            ruby_struct.font_size_xy[0] = readInt();
            ruby_struct.font_size_xy[1] = readInt();
            readStr( &ruby_struct.font_name );
            sentence_font.setRubyOnFlag(rubyon_flag);
        }
    }

    if (file_version >= 204){
        readInt();
        if ( !input_flag ) {
            for ( i=0 ; i<MAX_SPRITE2_NUM ; i++ ){
                readStr(&str);
                for (j=0; j<8; j++)
                    readInt();
            }
        } else {
            for ( i=0 ; i<MAX_SPRITE2_NUM ; i++ ){
                sprite2_info[i].remove();
                readStr( &sprite2_info[i].image_name );
                if ( sprite2_info[i].image_name ){
                    parseTaggedString( &sprite2_info[i] );
#ifdef RCA_SCALE
                    setupAnimationInfo( &sprite2_info[i], NULL, scr_stretch_x, scr_stretch_y );
#else
                    setupAnimationInfo( &sprite2_info[i] );
#endif
                }
                sprite2_info[i].orig_pos.x = readInt();
                sprite2_info[i].orig_pos.y = readInt();
                UpdateAnimPosStretchXY(&sprite2_info[i]);
                sprite2_info[i].scale_x = readInt();
                sprite2_info[i].scale_y = readInt();
                sprite2_info[i].rot = readInt();
                if ( readInt() == 1 ) sprite2_info[i].visible = true;
                else                  sprite2_info[i].visible = false;
                j = readInt();
                if (j == -1)
                    sprite2_info[i].trans = 256;
                else
                    sprite2_info[i].trans = j;
                sprite2_info[i].blending_mode = readInt();
                sprite2_info[i].calcAffineMatrix();
            }
        }
        readInt();
        readInt();
        if (file_version >= 205) readInt(); // 1
        readInt();
        readInt();
        readInt();
        readInt();
        if (file_version >= 205) readChar(); // 0
    }

    if (file_version >= 206){
        readInt(); // 0
        if ( !input_flag ) {
            readInt(); // 160
            readInt(); // 320
            readInt(); // 480
            readInt(); // 480
        } else {
            for (i=0; i<3; i++)
                humanpos[i] = readInt();
            underline_value = readInt();
            //might as well update the tachi
            for ( i=0 ; i<3 ; i++ ) {
                tachi_info[i].orig_pos.x = humanpos[i];
                tachi_info[i].orig_pos.y = underline_value + 1;
                UpdateAnimPosStretchXY(&tachi_info[i]);
                tachi_info[i].orig_pos.x -= tachi_info[i].orig_pos.w/2;
                tachi_info[i].orig_pos.y -= tachi_info[i].orig_pos.h;
                tachi_info[i].pos.x -= tachi_info[i].pos.w/2;
                tachi_info[i].pos.y -= tachi_info[i].pos.h;
            }
        }
    }

    int text_num = readInt();
    if ( !input_flag ) {
        for ( i=0 ; i<text_num ; i++ ){
            readStr(&str);
            if (file_version == 203) readChar(); // 0
        }
    } else {
        //Mion: clearing page then moving to next eliminates buffer error
        start_page = current_page->next;
        clearCurrentPage();
        current_page = start_page;
        for ( i=0 ; i<text_num ; i++ ){
            clearCurrentPage();
            do{
                current_page->text[current_page->text_count] = readChar();
            }
            while( current_page->text[current_page->text_count++] );
            if (file_version == 203) readChar(); // 0
            current_page->text_count--;
            current_page = current_page->next;
        }
        clearCurrentPage();
    }

    if (file_version >= 205){
        if ( !input_flag ) {
            j = readInt();
            for (i=0 ; i<j ; i++)
                readStr(&str);
        } else {
            Page *page = start_page;
            j = readInt();
            for (i=0 ; i<j ; i++){
                readStr(&page->tag);
                page = page->next;
            }
        }
    }
    else if (file_version >= 204){
        readInt();
        readInt();
    }
    
    if ( !input_flag ) {
        readInt();
        readInt();
    } else {
        i = readInt();
        current_label_info = script_h.getLabelByLine( i );
        current_line = i - current_label_info.start_line;
        //printf("load %d:%d(%d-%d)\n", current_label_info.start_line, current_line, i, current_label_info.start_line);
        char *buf = script_h.getAddressByLine( i );
    
        j = readInt();
        for ( i=0 ; i<j ; i++ ){
            while( *buf != ':' ) buf++;
            buf++;
        }
        script_h.setCurrent( buf );

        display_mode = shelter_display_mode = DISPLAY_MODE_NORMAL;
        refresh_window_text_mode = REFRESH_NORMAL_MODE | REFRESH_WINDOW_MODE | REFRESH_TEXT_MODE;

        clickstr_state = CLICK_NONE;
        draw_cursor_flag = false;
    }
    
    if (str) delete[] str;
    str = NULL;

    return 0;
}

void ONScripterLabel::saveSaveFile2( bool output_flag )
{
    int i, j;
    
    writeInt( 1, output_flag );
    writeInt( (sentence_font.is_bold?1:0), output_flag );
    writeInt( (sentence_font.is_shadow?1:0), output_flag );

    writeInt( 0, output_flag );
    writeInt( (rmode_flag)?1:0, output_flag );
    writeInt( sentence_font.color[0], output_flag );
    writeInt( sentence_font.color[1], output_flag );
    writeInt( sentence_font.color[2], output_flag );
    writeStr( cursor_info[0].image_name, output_flag );
    writeStr( cursor_info[1].image_name, output_flag );

    writeInt( window_effect.effect, output_flag );
    writeInt( window_effect.duration, output_flag );
    writeStr( window_effect.anim.image_name, output_flag ); // probably
    
    writeInt( sentence_font.top_xy[0], output_flag );
    writeInt( sentence_font.top_xy[1], output_flag );
#ifndef RCA_SCALE
    writeInt( sentence_font.num_xy[0], output_flag );
    writeInt( sentence_font.num_xy[1], output_flag );
#else
    writeInt( sentence_font.num_xy[0] / scr_stretch_x + 0.5, output_flag );  // RCA Subject to rounding errors
    writeInt( sentence_font.num_xy[1] / scr_stretch_y + 0.5, output_flag );  // RCA Subject to rounding errors
#endif
    writeInt( sentence_font.font_size_xy[0], output_flag );
    writeInt( sentence_font.font_size_xy[1], output_flag );
    writeInt( sentence_font.pitch_xy[0], output_flag );
    writeInt( sentence_font.pitch_xy[1], output_flag );
    for ( i=0 ; i<3 ; i++ )
        writeChar( sentence_font.window_color[2-i], output_flag );
    writeChar( ( sentence_font.is_transparent )?0x00:0xff, output_flag ); 
    writeInt( sentence_font.wait_time, output_flag );
    writeInt( sentence_font_info.orig_pos.x, output_flag );
    writeInt( sentence_font_info.orig_pos.y, output_flag );
    writeInt( sentence_font_info.orig_pos.w + sentence_font_info.orig_pos.x - 1, output_flag );
    writeInt( sentence_font_info.orig_pos.h + sentence_font_info.orig_pos.y - 1, output_flag );
    writeStr( sentence_font_info.image_name, output_flag );

    writeInt( (cursor_info[0].abs_flag)?0:1, output_flag );
    writeInt( (cursor_info[1].abs_flag)?0:1, output_flag );
    writeInt( cursor_info[0].orig_pos.x, output_flag );
    writeInt( cursor_info[1].orig_pos.x, output_flag );
    writeInt( cursor_info[0].orig_pos.y, output_flag );
    writeInt( cursor_info[1].orig_pos.y, output_flag );
    
    writeStr( bg_info.file_name, output_flag );
    for ( i=0 ; i<3 ; i++ )
        writeStr( tachi_info[i].image_name, output_flag );

    for ( i=0 ; i<3 ; i++ )
        writeInt( tachi_info[i].orig_pos.x, output_flag );

    for ( i=0 ; i<3 ; i++ )
        writeInt( tachi_info[i].orig_pos.y, output_flag );

    writeInt( 0, output_flag );
    writeInt( 0, output_flag );
    writeInt( 0, output_flag );

    writeInt( -1, output_flag );
    writeInt( -1, output_flag );
    writeInt( -1, output_flag );
    
    for ( i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        writeStr( sprite_info[i].image_name, output_flag );
        writeInt( sprite_info[i].orig_pos.x, output_flag );
        writeInt( sprite_info[i].orig_pos.y, output_flag );
        writeInt( sprite_info[i].visible?1:0, output_flag );
        writeInt( sprite_info[i].current_cell, output_flag );
        if (sprite_info[i].trans == 256)
            writeInt( -1, output_flag );
        else
            writeInt( sprite_info[i].trans, output_flag );
    }

    writeVariables( 0, script_h.global_variable_border, output_flag );

    // nested info
    int num_nest = 0;
    NestInfo *info = root_nest_info.next;
    while( info ){
        if      (info->nest_mode == NestInfo::LABEL) num_nest++;
        else if (info->nest_mode == NestInfo::FOR)   num_nest+=4;
        info = info->next;
    }
    writeInt( num_nest, output_flag );
    info = root_nest_info.next;
    while( info ){
        if  (info->nest_mode == NestInfo::LABEL){
            writeInt( script_h.getOffset( info->next_script ), output_flag );
        }
        else if (info->nest_mode == NestInfo::FOR){
            writeInt( info->var_no, output_flag );
            writeInt( info->to, output_flag );
            writeInt( info->step, output_flag );
            writeInt( -script_h.getOffset( info->next_script ), output_flag );
        }
        info = info->next;
    }
    
    writeInt( (monocro_flag)?1:0, output_flag );
    for ( i=0 ; i<3 ; i++ )
        writeInt( monocro_color[2-i], output_flag );
    writeInt( nega_mode, output_flag );

    // sound
    writeStr( seqmusic_file_name, output_flag ); // MIDI file

    writeStr( wave_file_name, output_flag ); // wave, waveloop

    if ( current_cd_track >= 0 ) // play CD
        writeInt( current_cd_track, output_flag );
    else
        writeInt( -1, output_flag );

    writeInt( (seqmusic_play_loop_flag)?1:0, output_flag ); // play, playonce MIDI
    writeInt( (wave_play_loop_flag)?1:0, output_flag ); // wave, waveloop
    writeInt( (cd_play_loop_flag)?1:0, output_flag ); // play, playonce
    writeInt( (music_play_loop_flag)?1:0, output_flag ); // bgm, mp3, mp3loop
    writeInt( (mp3save_flag)?1:0, output_flag );
    if (mp3save_flag)
        writeStr( music_file_name, output_flag );
    else
        writeStr( NULL, output_flag );
    
    writeInt( (erase_text_window_mode>0)?1:0, output_flag );
    writeInt( 1, output_flag );
    
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        if ( bar_info[i] ){
            writeInt( bar_info[i]->param, output_flag );
            writeInt( bar_info[i]->orig_pos.x, output_flag );
            writeInt( bar_info[i]->orig_pos.y, output_flag );
            writeInt( bar_info[i]->max_width, output_flag );
            writeInt( bar_info[i]->orig_pos.h, output_flag );
            writeInt( bar_info[i]->max_param, output_flag );
            for ( j=0 ; j<3 ; j++ )
                writeChar( bar_info[i]->color[2-j], output_flag );
            writeChar( 0x00, output_flag );
        }
        else{
            writeInt( 0, output_flag );
            writeInt( -1, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
        }
    }
    
    for ( i=0 ; i<MAX_PARAM_NUM ; i++ ){
        if ( prnum_info[i] ){
            writeInt( prnum_info[i]->param, output_flag );
            writeInt( prnum_info[i]->orig_pos.x, output_flag );
            writeInt( prnum_info[i]->orig_pos.y, output_flag );
            writeInt( prnum_info[i]->font_size_xy[0], output_flag );
            writeInt( prnum_info[i]->font_size_xy[1], output_flag );
            for ( j=0 ; j<3 ; j++ )
                writeChar( prnum_info[i]->color_list[0][2-j], output_flag );
            writeChar( 0x00, output_flag );
        }
        else{
            writeInt( 0, output_flag );
            writeInt( -1, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
            writeInt( 0, output_flag );
        }
    }
    
    writeInt( 1, output_flag ); // unidentified (not 1) data in version 205
    writeInt( 0, output_flag );
    writeInt( 1, output_flag );
    writeStr( btndef_info.image_name, output_flag );

    writeArrayVariable(output_flag);
    
    writeInt( 0, output_flag );
    writeChar( (erase_text_window_mode==2)?1:0, output_flag );
    writeChar( 0, output_flag );
    writeChar( 0, output_flag );
    writeChar( 0, output_flag );
    writeStr( loop_bgm_name[0], output_flag );
    writeStr( loop_bgm_name[1], output_flag );

    writeInt( (rubyon_flag)?1:0, output_flag );
    writeInt( ruby_struct.font_size_xy[0], output_flag );
    writeInt( ruby_struct.font_size_xy[1], output_flag );
    writeStr( ruby_struct.font_name, output_flag );

    writeInt( 0, output_flag );
    
    for ( i=0 ; i<MAX_SPRITE2_NUM ; i++ ){
        writeStr( sprite2_info[i].image_name, output_flag );
        writeInt( sprite2_info[i].orig_pos.x, output_flag );
        writeInt( sprite2_info[i].orig_pos.y, output_flag );
        writeInt( sprite2_info[i].scale_x, output_flag );
        writeInt( sprite2_info[i].scale_y, output_flag );
        writeInt( sprite2_info[i].rot, output_flag );
        writeInt( sprite2_info[i].visible?1:0, output_flag );
        if (sprite2_info[i].trans == 256)
            writeInt( -1, output_flag );
        else
            writeInt( sprite2_info[i].trans, output_flag );
        writeInt( sprite2_info[i].blending_mode, output_flag );
    }

    writeInt( 0, output_flag );
    writeInt( 0, output_flag );
    writeInt( 1, output_flag ); // added in version 205
    writeInt( 0, output_flag );
    writeInt( 0, output_flag );
    writeInt( 0, output_flag );
    writeInt( 0, output_flag );
    writeChar( 0, output_flag ); // added in version 205

    writeInt(   0, output_flag ); // added in version 206
    for (i=0; i<3; i++)
        writeInt( humanpos[i], output_flag ); // added in version 206
    writeInt( underline_value, output_flag ); // added in version 206

    Page *page = current_page;
    int num_page = 0;
    while( page != start_page ){
        page = page->previous;
        num_page++;
    }

    writeInt( num_page, output_flag );
    for ( i=0 ; i<num_page ; i++ ){
        for ( j=0 ; j<page->text_count ; j++ )
            writeChar( page->text[j], output_flag );
        writeChar( 0, output_flag );
        page = page->next;
    }

    page = start_page;
    writeInt(num_page, output_flag);
    for (i=0 ; i<num_page ; i++){
        if (page->tag)
            for ( j=0 ; j<(int)strlen(page->tag) ; j++ )
                writeChar( page->tag[j], output_flag );
        writeChar( 0, output_flag );
        page = page->next;
    }
    
    writeInt( current_label_info.start_line + current_line, output_flag );
    char *buf = script_h.getAddressByLine( current_label_info.start_line + current_line );
    //printf("save %d:%d\n", current_label_info.start_line, current_line);

    i = 0;
    if (!script_h.isText()){
        while( buf != script_h.getCurrent() ){
            if ( *buf == ':' ) i++;
            buf++;
        }
    }
    writeInt( i, output_flag );
}
