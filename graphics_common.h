/* -*- C++ -*-
 * 
 *  graphics_common.h - basic defines used in 16bpp & 32bpp graphics operations
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

#ifndef __GRAPHICS_COMMON_H__
#define __GRAPHICS_COMMON_H__

#if !defined(USE_CPU_GFX) && (defined(USE_X86_GFX) || defined(USE_PPC_GFX))
#define USE_CPU_GFX
#endif

#ifdef BPP16
#define BPP 16
#define BLENDMASK  0x07e0f81f
#define BLENDSHIFT 5
#define RMASKHI 0xf8000000
#define GMASKHI 0x07e00000
#define BMASKHI 0x001f0000
#define RBMASK 0xf81f
#define GBMASK 0x07ff
#define RMASK 0xf800
#define GMASK 0x07e0
#define BMASK 0x001f
#define AMASK 0
#define RSHIFT 11
#define GSHIFT 5
#define BSHIFT 0
#define ASHIFT 0
#define RLOSS  3
#define GLOSS  2
#define BLOSS  3
#define ALOSS  0

#else

#define BPP 32
// the mask is the same as the one used in TTF_RenderGlyph_Blended
#define RBMASK 0x00ff00ff
#define GBMASK 0x0000ffff
#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000
#define RSHIFT 16
#define GSHIFT 8
#define BSHIFT 0
#define ASHIFT 24
#define RLOSS  0
#define GLOSS  0
#define BLOSS  0
#define ALOSS  0

#endif

#define RGBMASK 0x00ffffff
#define MEDGRAY 0x88888888

#endif // __GRAPHICS_COMMON_H__
