/* -*- C++ -*-
 * 
 *  ONScripter_animation.cpp - Methods to manipulate AnimationInfo for ONScripter-EN
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

#include "ONScripterLabel.h"

int ONScripterLabel::proceedAnimation()
{
    int i, minimum_duration = -1;
    AnimationInfo *anim;
    
    for ( i=0 ; i<3 ; i++ ){
        anim = &tachi_info[i];
        if ( anim->visible && anim->is_animatable ){
            minimum_duration = estimateNextDuration( anim, anim->pos, minimum_duration );
        }
    }

    for ( i=MAX_SPRITE_NUM-1 ; i>=0 ; i-- ){
        anim = &sprite_info[i];
        if ( anim->visible && anim->is_animatable ){
            minimum_duration = estimateNextDuration( anim, anim->pos, minimum_duration );
        }
    }
//Mion - ogapee2009
#ifdef USE_LUA
    if (lua_handler.is_animatable && !script_h.isExternalScript()){
        if (lua_handler.remaining_time == 0){
            lua_handler.remaining_time = lua_handler.duration_time;
            if (minimum_duration == -1 || 
                minimum_duration > lua_handler.remaining_time)
                minimum_duration = lua_handler.remaining_time;
            int lua_event_mode = event_mode;
            int tmp_string_buffer_offset = string_buffer_offset;
            char *current = script_h.getCurrent();
            lua_handler.callback(LUAHandler::LUA_ANIMATION);
            script_h.setCurrent(current);
            readToken();
            string_buffer_offset = tmp_string_buffer_offset;
            event_mode = lua_event_mode;
        }
        else if ( (minimum_duration == -1) ||
                  (minimum_duration > lua_handler.remaining_time) ){
            minimum_duration = lua_handler.remaining_time;
        }
    }
#endif

    return minimum_duration;
}

int ONScripterLabel::proceedCursorAnimation()
{
    int minimum_duration = -1;
    AnimationInfo *anim = NULL;

    if ( !textgosub_label && draw_cursor_flag &&
         ( clickstr_state == CLICK_WAIT ||
           clickstr_state == CLICK_NEWPAGE ) ){
        
        if      ( clickstr_state == CLICK_WAIT )
            anim = &cursor_info[CURSOR_WAIT_NO];
        else if ( clickstr_state == CLICK_NEWPAGE )
            anim = &cursor_info[CURSOR_NEWPAGE_NO];

        if ( anim && anim->visible && anim->is_animatable ){
            SDL_Rect dst_rect = anim->pos;
            if ( !anim->abs_flag ){
                dst_rect.x += ExpandPos(sentence_font.x());
                dst_rect.y += ExpandPos(sentence_font.y());
            }

            minimum_duration = estimateNextDuration( anim, dst_rect, minimum_duration );
        }
    }

    return minimum_duration;
}

int ONScripterLabel::estimateNextDuration( AnimationInfo *anim, SDL_Rect &rect, int minimum )
{
    if ( anim->remaining_time == 0 ){

#ifndef NO_LAYER_EFFECTS
        if ( anim->trans_mode != AnimationInfo::TRANS_LAYER ) {
#endif
            if ( (minimum == -1) ||
                 (minimum > anim->duration_list[ anim->current_cell ]) )
                minimum = anim->duration_list[ anim->current_cell ];
            if ( anim->proceedAnimation() ) {
                dirty_rect.add(rect);
            }
#ifndef NO_LAYER_EFFECTS
        } else if (anim->layer_no >= 0) {
            LayerInfo *tmp = layer_info;
            while (tmp) {
                if ( tmp->num == anim->layer_no ) break;
                tmp = tmp->next;
            }
            if (tmp) {
                tmp->handler->update();
                dirty_rect.add(rect);
                anim->remaining_time = anim->duration_list[ anim->current_cell ];
                if ( (minimum == -1) ||
                     (minimum > anim->duration_list[ anim->current_cell ]) )
                    minimum = anim->duration_list[ anim->current_cell ];
            }
        }
#endif
    }
    else{
        if ( (minimum == -1) || (minimum > anim->remaining_time) )
            minimum = anim->remaining_time;
    }

    return minimum;
}

void ONScripterLabel::resetRemainingTime( int t )
{
    int i;
    AnimationInfo *anim;
    
    for ( i=0 ; i<3 ; i++ ){
        anim = &tachi_info[i];
        if ( anim->visible && anim->is_animatable){
            anim->remaining_time -= t;
            if (anim->remaining_time < 0)
                anim->remaining_time = 0;
        }
    }

    for ( i=MAX_SPRITE_NUM-1 ; i>=0 ; i-- ){
        anim = &sprite_info[i];
        if ( anim->visible && anim->is_animatable ){
            anim->remaining_time -= t;
            if (anim->remaining_time < 0)
                anim->remaining_time = 0;
        }
    }
//Mion - ogapee2009
#ifdef USE_LUA
    if (lua_handler.is_animatable && !script_h.isExternalScript())
        lua_handler.remaining_time -= t;
#endif
}

void ONScripterLabel::resetCursorTime( int t )
{
    AnimationInfo *anim = NULL;

    if ( !textgosub_label &&
         ((clickstr_state == CLICK_WAIT) ||
          (clickstr_state == CLICK_NEWPAGE)) ){
        if ( clickstr_state == CLICK_WAIT )
            anim = &cursor_info[CURSOR_WAIT_NO];
        else if ( clickstr_state == CLICK_NEWPAGE )
            anim = &cursor_info[CURSOR_NEWPAGE_NO];
        
        if ( anim->visible && anim->is_animatable ){
            anim->remaining_time -= t;
            if (anim->remaining_time < 0)
                anim->remaining_time = 0;
        }
    }
}

#ifdef RCA_SCALE
void ONScripterLabel::setupAnimationInfo( AnimationInfo *anim, Fontinfo *info,
                                          float stretch_x, float stretch_y )
#else
void ONScripterLabel::setupAnimationInfo( AnimationInfo *anim, Fontinfo *info )
#endif
{
    if (anim->image_surface && !anim->stale_image) return;

    anim->deleteImage();
    anim->abs_flag = true;

    if ( anim->trans_mode == AnimationInfo::TRANS_STRING ){
        Fontinfo f_info = sentence_font;
        if (info) f_info = *info;

        if ( anim->font_size_xy[0] >= 0 ){ // in case of Sprite, not rclick menu
            f_info.setTateyokoMode(0);
            f_info.top_xy[0] = anim->orig_pos.x;
            f_info.top_xy[1] = anim->orig_pos.y;
            if (anim->is_single_line)
                f_info.setLineArea( strlen(anim->file_name)/2+1 );
            f_info.clear();
            
            f_info.pitch_xy[0] = f_info.pitch_xy[0] - f_info.font_size_xy[0] + anim->font_size_xy[0];
            f_info.font_size_xy[0] = anim->font_size_xy[0];
            f_info.pitch_xy[1] = f_info.pitch_xy[1] - f_info.font_size_xy[1] + anim->font_size_xy[1];
            f_info.font_size_xy[1] = anim->font_size_xy[1];
            if ( anim->font_pitch >= 0 )
                f_info.pitch_xy[0] = anim->font_pitch;
            f_info.ttf_font = NULL;
        }

        SDL_Rect pos;
        if (anim->is_tight_region){
            drawString( anim->file_name, anim->color_list[ anim->current_cell ], &f_info, false, NULL, 0, &pos, NULL, anim->skip_whitespace );
        }
        else{
            int xy_bak[2];
            xy_bak[0] = f_info.xy[0];
            xy_bak[1] = f_info.xy[1];
            
            int xy[2] = {0, 0};
            f_info.setXY(f_info.num_xy[0]-1, f_info.num_xy[1]-1);
            pos = f_info.calcUpdatedArea(xy, screen_ratio1, screen_ratio2);

            f_info.xy[0] = xy_bak[0];
            f_info.xy[1] = xy_bak[1];
        }
        
        if (info != NULL){
            info->xy[0] = f_info.xy[0];
            info->xy[1] = f_info.xy[1];
        }
        
        anim->allocImage( pos.w*anim->num_of_cells, pos.h );
        anim->fill( 0, 0, 0, 0 );
        // need to fake an "original size" for this text sprite
        anim->orig_pos.w = ContractPos(pos.w);
        anim->orig_pos.h = ContractPos(pos.h);
        
        f_info.setRubyOnFlag(rubyon_flag && anim->is_ruby_drawable);
        f_info.top_xy[0] = f_info.top_xy[1] = 0;
        for ( int i=0 ; i<anim->num_of_cells ; i++ ){
            int offset = i * pos.w;
            f_info.clear();
            drawString( anim->file_name, anim->color_list[i], &f_info, false,
                        NULL, offset, NULL, anim, anim->skip_whitespace );
        }
    }
#ifndef NO_LAYER_EFFECTS
    else if (anim->trans_mode == AnimationInfo::TRANS_LAYER) {
        //pos w&h already screen-size
        anim->allocImage( anim->pos.w, anim->pos.h );
        anim->fill( 0, 0, 0, 0 );
    }
#endif //ndef NO_LAYER_EFFECTS
    else {
        bool has_alpha;
        SDL_Surface *surface = loadImage( anim->file_name, &has_alpha );

        SDL_Surface *surface_m = NULL;
        if (anim->trans_mode == AnimationInfo::TRANS_MASK)
            surface_m = loadImage( anim->mask_file_name );
        

        surface = anim->setupImageAlpha(surface, surface_m, has_alpha);

        bool did_resize = false;
#ifdef RCA_SCALE
        if (surface && ( (stretch_x > 1.0) || (stretch_y > 1.0) ||
                         ((screen_ratio2 != screen_ratio1) && !disable_rescale_flag) )){
            surface = anim->resize( surface, screen_ratio1, screen_ratio2, stretch_x, stretch_y );
#else
        if (surface && (screen_ratio2 != screen_ratio1) && !disable_rescale_flag){
            surface = anim->resize( surface, screen_ratio1, screen_ratio2 );
#endif //RCA_SCALE
            did_resize = true;
        }
        anim->setImage( surface );

        if ( surface_m ) SDL_FreeSurface(surface_m);

        if ((event_mode == IDLE_EVENT_MODE) &&
            ( did_resize || (trap_mode != TRAP_NONE) )) {
            // detect events from during image loading & resizing, but
            // without any image refresh (esp. if trapping)
            event_mode |= WAIT_NO_ANIM_MODE;
            waitEvent(0);
            event_mode &= ~WAIT_NO_ANIM_MODE;
        }
    }
    anim->stale_image = false;
}

bool ONScripterLabel::sameImageTag(const AnimationInfo &anim1, const AnimationInfo &anim2)
// returns true if the AnimationInfo tags would create identical images
{
    if (&anim1 == &anim2) return true;

    if ( (anim1.trans_mode != anim2.trans_mode) ||
#ifndef NO_LAYER_EFFECTS
        //assume layers aren't identical
         (anim1.trans_mode == AnimationInfo::TRANS_LAYER) ||
#endif
         (anim1.num_of_cells != anim2.num_of_cells) )
        return false;

    if ( !equalStrings(anim1.file_name, anim2.file_name) ||
         !equalStrings(anim1.mask_file_name, anim2.mask_file_name) )
        return false;

    if ( !equalColors(anim1.color, anim2.color) ||
         ((anim1.trans_mode == AnimationInfo::TRANS_DIRECT) &&
         !equalColors(anim1.direct_color, anim2.direct_color)) )
        return false;

    if (anim1.trans_mode == AnimationInfo::TRANS_STRING) {
        for ( int i=0 ; i<anim1.num_of_cells ; i++ ) {
            if (!equalColors(anim1.color_list[i], anim2.color_list[i]))
                return false;
        }
    }
    //by this point, they most likely create the same images
    return true;
}

void ONScripterLabel::parseTaggedString( AnimationInfo *anim, bool is_mask )
{
    if (anim->image_name == NULL) return;

    AnimationInfo acopy;
    if (!anim->stale_image && anim->image_surface)
        acopy.deepcopyTag(*anim); // a copy of the tag, for later comparison
    anim->removeTag();
    
    int i;
    char *buffer = anim->image_name;
    anim->num_of_cells = 1;
    anim->current_cell = 0;
    anim->trans_mode = trans_mode;
    //use COPY as default trans_mode for masks
    if (is_mask) anim->trans_mode = AnimationInfo::TRANS_COPY;

#ifndef NO_LAYER_EFFECTS
    if ( buffer[0] == '*' ){
        //Mion: it's a layer!
        LayerInfo *tmp = layer_info;
        anim->trans_mode = AnimationInfo::TRANS_LAYER;
        buffer++;
        anim->layer_no = getNumberFromBuffer( (const char**)&buffer );

        while (tmp) {
            if ( tmp->num == anim->layer_no ) break;
            tmp = tmp->next;
        }
        if (tmp) {
            anim->pos.x = anim->pos.y = 0;
            anim->pos.w = screen_width;
            anim->pos.h = screen_height;
            tmp->handler->setSpriteInfo(sprite_info, anim);
            anim->duration_list = new int[1];
            anim->duration_list[0] = tmp->interval;
            anim->is_animatable = true;
            printf("setup a sprite for layer %d\n", anim->layer_no);
        } else
            anim->layer_no = -1;
        return;
    }
#endif //ndef NO_LAYER_EFFECTS
    if ( buffer[0] == ':' ){
        while (*++buffer == ' ');
        
        if ( buffer[0] == 'a' ){
            anim->trans_mode = AnimationInfo::TRANS_ALPHA;
            buffer++;
        }
        else if ( buffer[0] == 'l' ){
            anim->trans_mode = AnimationInfo::TRANS_TOPLEFT;
            buffer++;
        }
        else if ( buffer[0] == 'r' ){
            anim->trans_mode = AnimationInfo::TRANS_TOPRIGHT;
            buffer++;
        }
        else if ( buffer[0] == 'c' ){
            anim->trans_mode = AnimationInfo::TRANS_COPY;
            buffer++;
        }
        else if ( buffer[0] == 's' ){
            anim->trans_mode = AnimationInfo::TRANS_STRING;
            buffer++;
            anim->num_of_cells = 0;
            if ( *buffer == '/' ){
                buffer++;
                script_h.getNext();
                
                script_h.pushCurrent( buffer );
                anim->font_size_xy[0] = script_h.readInt();
                anim->font_size_xy[1] = script_h.readInt();
                anim->font_pitch = script_h.readInt() + anim->font_size_xy[0];
                if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
                    script_h.readInt(); // 0 ... normal, 1 ... no anti-aliasing, 2 ... Fukuro
                }
                buffer = script_h.getNext();
                script_h.popCurrent();
            }
            else{
                anim->font_size_xy[0] = sentence_font.font_size_xy[0];
                anim->font_size_xy[1] = sentence_font.font_size_xy[1];
                anim->font_pitch = sentence_font.pitch_xy[0];
            }
            while(buffer[0] != '#' && buffer[0] != '\0') buffer++;
            i=0;
            while( buffer[i] == '#' ){
                anim->num_of_cells++;
                i += 7;
            }
            anim->color_list = new uchar3[ anim->num_of_cells ];
            for ( i=0 ; i<anim->num_of_cells ; i++ ){
                readColor( &anim->color_list[i], buffer );
                buffer += 7;
            }
        }
        else if ( buffer[0] == 'm' ){
            anim->trans_mode = AnimationInfo::TRANS_MASK;
            char *start = ++buffer;
            while(buffer[0] != ';' && buffer[0] != 0x0a && buffer[0] != '\0') buffer++;
            if (buffer[0] == ';')
                setStr( &anim->mask_file_name, start, buffer-start );
        }
        else if ( buffer[0] == '#' ){
            anim->trans_mode = AnimationInfo::TRANS_DIRECT;
            readColor( &anim->direct_color, buffer );
            buffer += 7;
        }
        else if ( buffer[0] == '!' ){
            anim->trans_mode = AnimationInfo::TRANS_PALETTE;
            buffer++;
            anim->palette_number = getNumberFromBuffer( (const char**)&buffer );
        }

        if (anim->trans_mode != AnimationInfo::TRANS_STRING)
            while(buffer[0] != '/' && buffer[0] != ';' && buffer[0] != '\0') buffer++;
    }

    if ( buffer[0] == '/' ){
        buffer++;
        anim->num_of_cells = getNumberFromBuffer( (const char**)&buffer );
        buffer++;
        if ( anim->num_of_cells == 0 ){
            fprintf( stderr, "ONScripterLabel::parseTaggedString  The number of cells is 0\n");
            return;
        }

        anim->duration_list = new int[ anim->num_of_cells ];

        if ( *buffer == '<' ){
            buffer++;
            for ( i=0 ; i<anim->num_of_cells ; i++ ){
                anim->duration_list[i] = getNumberFromBuffer( (const char**)&buffer );
                buffer++;
            }
            buffer++; // skip '>'
        }
        else{
            anim->duration_list[0] = getNumberFromBuffer( (const char**)&buffer );
            for ( i=1 ; i<anim->num_of_cells ; i++ )
                anim->duration_list[i] = anim->duration_list[0];
            buffer++;
        }
        
        anim->loop_mode = *buffer++ - '0'; // 3...no animation
        if ( anim->loop_mode != 3 ) anim->is_animatable = true;

        while(buffer[0] != ';' && buffer[0] != '\0') buffer++;
    }

    if ( buffer[0] == ';' ) buffer++;

    if ( anim->trans_mode == AnimationInfo::TRANS_STRING && buffer[0] == '$' ){
        script_h.pushCurrent( buffer );
        setStr( &anim->file_name, script_h.readStr() );
        script_h.popCurrent();
    }
    else{
        setStr( &anim->file_name, buffer );
    }

    anim->stale_image = ( anim->stale_image || (anim->image_surface == NULL) ||
                          !sameImageTag(*anim,acopy) );
}

void ONScripterLabel::drawTaggedSurface( SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect &clip )
{
#ifndef NO_LAYER_EFFECTS
    if ( anim->trans_mode != AnimationInfo::TRANS_LAYER ) {
#endif
        SDL_Rect poly_rect = anim->pos;

        if ( !anim->abs_flag ){
            poly_rect.x += ExpandPos(sentence_font.x());
            poly_rect.y += ExpandPos(sentence_font.y());
        }

        if (!anim->affine_flag)
            anim->blendOnSurface( dst_surface, poly_rect.x, poly_rect.y,
                                  clip, anim->trans );
        else
            anim->blendOnSurface2( dst_surface, poly_rect.x, poly_rect.y,
                                   clip, anim->trans );
#ifndef NO_LAYER_EFFECTS
    } else if (anim->layer_no >= 0) {
        LayerInfo *tmp = layer_info;
        while (tmp) {
            if ( tmp->num == anim->layer_no ) break;
            tmp = tmp->next;
        }
        if (tmp)
            tmp->handler->refresh( dst_surface, clip );
    }
#endif
}

void ONScripterLabel::stopCursorAnimation( int click )
{
    int no;

    if ( textgosub_label || !draw_cursor_flag ) return;

    if      ( click == CLICK_WAIT )    no = CURSOR_WAIT_NO;
    else if ( click == CLICK_NEWPAGE ) no = CURSOR_NEWPAGE_NO;
    else return;

    if (cursor_info[no].image_surface == NULL) return;
    
    SDL_Rect dst_rect = cursor_info[ no ].pos;

    if ( !cursor_info[ no ].abs_flag ){
        dst_rect.x += ExpandPos(sentence_font.x());
        dst_rect.y += ExpandPos(sentence_font.y());
    }

    flushDirect( dst_rect, refreshMode() );
}
