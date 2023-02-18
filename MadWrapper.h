/* -*- C++ -*-
 * 
 *  MadWrapper.h - SMPEG compatible wrapper functions for MAD: Mpeg Audio Decoder
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __MAD_WRAPPER_H__
#define __MAD_WRAPPER_H__

#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>

typedef struct _MAD_WRAPPER MAD_WRAPPER;

MAD_WRAPPER* MAD_WRAPPER_new( const char *file, void* info, int sdl_audio );
MAD_WRAPPER* MAD_WRAPPER_new_rwops( SDL_RWops *src, void* info, int sdl_audio );
int MAD_WRAPPER_playAudio( void *userdata, Uint8 *stream, int len );
void MAD_WRAPPER_stop( MAD_WRAPPER *mad );
void MAD_WRAPPER_play( MAD_WRAPPER *mad );
void MAD_WRAPPER_delete( MAD_WRAPPER *mad );
void MAD_WRAPPER_setvolume( MAD_WRAPPER *mad, int volume );
const char* MAD_WRAPPER_error( MAD_WRAPPER *mad );

#define SMPEG_new MAD_WRAPPER_new
#define SMPEG_new_rwops MAD_WRAPPER_new_rwops
#define SMPEG_playAudio MAD_WRAPPER_playAudio
#define SMPEG_play MAD_WRAPPER_play
#define SMPEG_stop MAD_WRAPPER_stop
#define SMPEG_delete MAD_WRAPPER_delete
#define SMPEG_setvolume MAD_WRAPPER_setvolume
#define SMPEG MAD_WRAPPER
#define SMPEG_error MAD_WRAPPER_error

#endif // __MAD_WRAPPER_H__

