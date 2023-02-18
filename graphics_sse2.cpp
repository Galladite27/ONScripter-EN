/* -*- C++ -*-
 * 
 *  graphics_sse2.cpp - graphics routines using X86 SSE2 cpu functionality
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

#include <emmintrin.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "graphics_sum.h"
#include "graphics_blend.h"

namespace ons_gfx {

int imageFilterMean_SSE2(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
        MEAN_PIXEL();
        --n; ++dst; ++src1; ++src2;
    }

    // Do bulk of processing using SSE2 (find the mean of 16 8-bit unsigned integers, with saturation)
    __m128i mask = _mm_set1_epi8(0x7F);
    while(n >= 16) {
        __m128i s1 = _mm_loadu_si128((__m128i*)src1);
        s1 = _mm_srli_epi16(s1, 1); // shift right 1
        s1 = _mm_and_si128(s1, mask); // apply byte-mask
        __m128i s2 = _mm_loadu_si128((__m128i*)src2);
        s2 = _mm_srli_epi16(s2, 1); // shift right 1
        s2 = _mm_and_si128(s2, mask); // apply byte-mask
        __m128i r = _mm_adds_epu8(s1, s2);
        _mm_store_si128((__m128i*)dst, r);

        n -= 16; src1 += 16; src2 += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_MEAN();

    return length - n;
}


int imageFilterAddTo_SSE2(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
        ADDTO_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using SSE2 (add 16 8-bit unsigned integers, with saturation)
    while(n >= 16) {
        __m128i s = _mm_loadu_si128((__m128i*)src);
        __m128i d = _mm_load_si128((__m128i*)dst);
        __m128i r = _mm_adds_epu8(s, d);
        _mm_store_si128((__m128i*)dst, r);

        n -= 16; src += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_ADDTO();

    return length - n;
}


void imageFilterSubFrom_SSE2(unsigned char *dst, unsigned char *src, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst
    while( (((long)dst & 0xF) > 0) && (n > 0) ) {
        SUBFROM_PIXEL();
        --n; ++dst; ++src;
    }

    // Do bulk of processing using SSE2 (sub 16 8-bit unsigned integers, with saturation)
    while(n >= 16) {
        __m128i s = _mm_loadu_si128((__m128i*)src);
        __m128i d = _mm_load_si128((__m128i*)dst);
        __m128i r = _mm_subs_epu8(d, s);
        _mm_store_si128((__m128i*)dst, r);

        n -= 16; src += 16; dst += 16;
    }

    // If any bytes are left over, deal with them individually
    ++n;
    BASIC_SUBFROM();
}

// basic bitmasks 0x00FF00FF, 0x000000FF
static const __m128i rbmask = _mm_set1_epi32(0x00FF00FF);
static const __m128i bmask = _mm_srli_epi32(rbmask, 16);

static inline __m128i alphaBlendCore_SSE2(__m128i src1, __m128i src2, __m128i d_a)
{
    // rb = (src2_argb & rbmask) * alpha1
    __m128i rb = _mm_and_si128(src2, rbmask);
    rb = _mm_mullo_epi16(d_a, rb);
    // g = ((src2_argb >> 8) & bmask) * alpha1
    src2 = _mm_srli_epi32(src2, 8);
    __m128i g = _mm_and_si128(src2, bmask);
    g = _mm_mullo_epi16(d_a, g);
    // alpha2 = alpha1 ^ rbmask
    d_a = _mm_xor_si128(d_a, rbmask);
    // rb += (src1_argb & rbmask) * alpha2
    __m128i tmp = _mm_and_si128(src1, rbmask);
    tmp = _mm_mullo_epi16(d_a, tmp);
    rb = _mm_add_epi32(rb, tmp);
    // rb = (rb >> 8) & rbmask
    rb = _mm_srli_epi32(rb, 8);
    rb = _mm_and_si128(rb, rbmask);
    // g += ((src1_argb >> 8) & bmask) * alpha2
    src1 = _mm_srli_epi32(src1, 8);
    tmp = _mm_and_si128(src1, bmask);
    tmp = _mm_mullo_epi16(d_a, tmp);
    g = _mm_add_epi32(g, tmp);
    // g = g & (bmask << 8)
    tmp =_mm_slli_epi32(bmask, 8);
    g = _mm_and_si128(g, tmp);
    // dst_argb = rb | g
    return _mm_or_si128(rb, g);
}

int imageFilterBlend_SSE2(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst_buffer
    while( (((long)dst_buffer & 0xF) > 0) && (n > 0) ) {
        BLEND_PIXEL();
        --n; ++dst_buffer; ++src_buffer;
    }

    // Do bulk of processing using SSE2 (process 4 32bit (BGRA) pixels)
    while(n >= 4) {
        // alpha1 = ((src_argb >> 24) * alpha) >> 8
        __m128i a = _mm_set1_epi32(alpha);
        __m128i buf = _mm_loadu_si128((__m128i*)src_buffer);
        __m128i tmp = _mm_srli_epi32(buf, 24);
        a = _mm_mullo_epi16(a, tmp);
        a = _mm_srli_epi32(a, 8);
        // double-up alpha1 (0x000000vv -> 0x00vv00vv)
        tmp = _mm_slli_epi32(a, 16);
        a = _mm_or_si128(a, tmp);

        tmp = _mm_load_si128((__m128i*)dst_buffer);
        __m128i dst = alphaBlendCore_SSE2(tmp, buf, a);
        _mm_store_si128((__m128i*)dst_buffer, dst);

        n -= 4; src_buffer += 4; dst_buffer += 4; alphap += 16;
    }

    // If any pixels are left over, deal with them individually
    ++n;
    BASIC_BLEND();

    return length - n;
}

int imageFilterEffectBlend_SSE2(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 mask2, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst_buffer
    while( (((long)dst_buffer & 0xF) > 0) && (n > 0) ) {
        BLEND_EFFECT_PIXEL();
        --n; ++dst_buffer; ++src1_buffer; ++src2_buffer;
    }

    // Do bulk of processing using SSE2 (process 4 32bit (BGRA) pixels)
    // load alpha1, double-up (0x000000vv -> 0x00vv00vv)
    __m128i a = _mm_set1_epi32(mask2);
    __m128i tmp = _mm_slli_epi32(a, 16);
    a = _mm_or_si128(a, tmp);
    while(n >= 4) {

        tmp = _mm_loadu_si128((__m128i*)src1_buffer);
        __m128i buf = _mm_loadu_si128((__m128i*)src2_buffer);
        __m128i dst = alphaBlendCore_SSE2(tmp, buf, a);
        _mm_store_si128((__m128i*)dst_buffer, dst);

        n -= 4; dst_buffer += 4; src1_buffer += 4; src2_buffer += 4;
    }

    // If any pixels are left over, deal with them individually
    ++n;
    while(--n > 0) {
        BLEND_EFFECT_PIXEL();
        ++dst_buffer, ++src1_buffer, ++src2_buffer;
    }

    return length - n;
}

int imageFilterEffectMaskBlend_SSE2(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 *mask_buffer, Uint32 overflow_mask, Uint32 mask_value, int length)
{
    int n = length;

    // Compute first few values so we're on a 16-byte boundary in dst_buffer
    while( (((long)dst_buffer & 0xF) > 0) && (n > 0) ) {
        BLEND_EFFECT_MASK_PIXEL();
        --n; ++dst_buffer; ++src1_buffer; ++src2_buffer; ++mask_buffer;
    }

    // Do bulk of processing using SSE2 (process 4 32bit (BGRA) pixels)
    __m128i over = bmask;
    if (overflow_mask == 0xFFFFFFFF)
        over = _mm_xor_si128(bmask, bmask);
    while(n >= 4) {
        //if (mask_value > (mask & BMASK))
        //    alpha1 = subs(mask_value,(mask & BMASK)
        //    if (alpha1 > over) alpha1 = BMASK
        //else alpha1 = 0
        __m128i a = _mm_set1_epi32(mask_value);
        __m128i buf = _mm_loadu_si128((__m128i*)mask_buffer);
        buf = _mm_and_si128(buf, bmask);
        __m128i tmp = _mm_cmpgt_epi32(a, buf);
        tmp = _mm_and_si128(tmp, bmask);
        a = _mm_subs_epu16(a, buf);
        buf = _mm_cmpgt_epi32(a, over);
        a = _mm_or_si128(a, buf);
        a = _mm_and_si128(a, tmp);

        // double-up alpha1 (0x000000vv -> 0x00vv00vv)
        tmp = _mm_slli_epi32(a, 16);
        a = _mm_or_si128(a, tmp);

        tmp = _mm_loadu_si128((__m128i*)src1_buffer);
        buf = _mm_loadu_si128((__m128i*)src2_buffer);
        __m128i dst = alphaBlendCore_SSE2(tmp, buf, a);
        _mm_store_si128((__m128i*)dst_buffer, dst);

        n -= 4; dst_buffer += 4; src1_buffer += 4; src2_buffer += 4; mask_buffer += 4;
    }

    // If any pixels are left over, deal with them individually
    ++n;
    while(--n > 0) {
        BLEND_EFFECT_MASK_PIXEL();
        ++dst_buffer, ++src1_buffer, ++src2_buffer; ++mask_buffer;
    }

    return length - n;
}

}//namespace ons_gfx

#endif //USE_X86_GFX
