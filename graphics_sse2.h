/* -*- C++ -*-
 * 
 *  graphics_sse2.h - graphics routines using X86 SSE2 cpu functionality
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

#ifndef __GRAPHICS_SSE2_H__
#define __GRAPHICS_SSE2_H__

#ifdef USE_X86_GFX
namespace ons_gfx {

int imageFilterMean_SSE2(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length);
int imageFilterAddTo_SSE2(unsigned char *dst, unsigned char *src, int length);
int imageFilterSubFrom_SSE2(unsigned char *dst, unsigned char *src, int length);
int imageFilterBlend_SSE2(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length);
int imageFilterEffectBlend_SSE2(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 mask2, int length);
int imageFilterEffectMaskBlend_SSE2(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 *mask_buffer, Uint32 is_crossfade, Uint32 mask_value, int length);

}
#endif //USE_X86_GFX

#endif // __GRAPHICS_SSE2_H__
