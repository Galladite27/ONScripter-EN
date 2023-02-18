/* -*- C++ -*-
 * 
 *  graphics_resize.h - ons_gfx namespace utility functions for resizing
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

#ifndef __GRAPHICS_RESIZE_H__
#define __GRAPHICS_RESIZE_H__

#include <SDL.h>
#include "graphics_common.h"

namespace ons_gfx {

    //Mion: for resizing (moved from ONScripterLabel)
    void resetResizeBuffer();
    int resizeSurface( SDL_Surface *src, SDL_Surface *dst, int num_cells=1 );

}

#endif // __GRAPHICS_RESIZE_H__
