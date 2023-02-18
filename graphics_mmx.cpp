/* -*- C++ -*-
 * 
 *  graphics_mmx.cpp - graphics routines using X86 MMX cpu functionality
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

// Based upon routines provided by Roto

#ifdef USE_X86_GFX

#include <mmintrin.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "graphics_sum.h"

namespace ons_gfx {

void imageFilterMean_MMX(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
    int n = length;

    // Compute first few values so we're on a 8-byte boundary in dst
    while( (((long)dst & 0x7) > 0) && (n > 0) ) {
        MEAN_PIXEL();
        --n; ++dst; ++src1; ++src2;
    }

    // Do bulk of processing using MMX (find the mean of 8 8-bit unsigned integers, with saturation)
    __m64 mask = _mm_set1_pi8(0x7F);
    while(n >= 8) {
        __m64 s1 = *((__m64*)src1);
        s1 = _mm_srli_pi16(s1, 1);
        s1 = _mm_and_si64(s1, mask);
        __m64 s2 = *((__m64*)src2);
        s2 = _mm_srli_pi16(s2, 1);
        s2 = _mm_and_si64(s2, mask);
        __m64* d = (__m64*)dst;
        *d = _mm_adds_pu8(s1, s2);

        n -= 8; src1 += 8; src2 += 8; dst += 8;
    }
    _mm_empty();

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_MEAN();
}


void imageFilterAddTo_MMX(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 8-byte boundary in dst
    while( (((long)dst & 0x7) > 0) && (n > 0) ) {
        ADDTO_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using MMX (add 8 8-bit unsigned integers, with saturation)
    while(n >= 8) {
        __m64* s = (__m64*)src;
        __m64* d = (__m64*)dst;
        *d = _mm_adds_pu8(*d, *s);

        n -= 8; src += 8; dst += 8;
    }
    _mm_empty();

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_ADDTO();
}


void imageFilterSubFrom_MMX(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 8-byte boundary in dst
    while( (((long)dst & 0x7) > 0) && (n > 0) ) {
        SUBFROM_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using MMX (sub 8 8-bit unsigned integers, with saturation)
    while(n >= 8) {
        __m64* s = (__m64*)src;
        __m64* d = (__m64*)dst;
        *d = _mm_subs_pu8(*d, *s);

        n -= 8; src += 8; dst += 8;
    }
    _mm_empty();

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_SUBFROM();
}

}//namespace ons_gfx

#endif //USE_X86_GFX


