/* -*- C++ -*-
 * 
 *  graphics_altivec.cpp - graphics routines using PPC Altivec cpu functionality
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

#ifdef USE_PPC_GFX

#include <altivec.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "graphics_sum.h"

namespace ons_gfx {

void imageFilterMean_Altivec(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
         MEAN_PIXEL();
       --n; ++dst; ++src1; ++src2;
    }

    // Do bulk of processing using Altivec (find the mean of 16 8-bit unsigned integers, with saturation)
    vector unsigned char rshft = vec_splat_u8(0x1);
    while(n >= 16) {
        vector unsigned char s1 = vec_ld(0,src1);
        s1 = vec_sr(s1, rshft); // shift right 1
        vector unsigned char s2 = vec_ld(0,src2);
        s2 = vec_sr(s2, rshft); // shift right 1
        vector unsigned char r = vec_adds(s1, s2);
        vec_st(r,0,dst);

        n -= 16; src1 += 16; src2 += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_MEAN();
}


void imageFilterAddTo_Altivec(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
        ADDTO_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using Altivec (add 16 8-bit unsigned integers, with saturation)
    while(n >= 16) {
        vector unsigned char s = vec_ld(0,src);
        vector unsigned char d = vec_ld(0,dst);
        vector unsigned char r = vec_adds(d, s);
        vec_st(r,0,dst);

        n -= 16; src += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_ADDTO();
}


void imageFilterSubFrom_Altivec(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
        SUBFROM_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using Altivec (sub 16 8-bit unsigned integers, with saturation)
    while(n >= 16) {
        vector unsigned char s = vec_ld(0,src);
        vector unsigned char d = vec_ld(0,dst);
        vector unsigned char r = vec_subs(d, s);
        vec_st(r,0,dst);

        n -= 16; src += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_SUBFROM();
}

}//namespace ons_gfx

#endif //USE_PPC_GFX


