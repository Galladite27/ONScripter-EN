/* -*- C++ -*-
 * 
 *  graphics_sum.h - graphics macros and ons_gfx namespace utility functions
 *                   for simple add/sub/mean operations
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

#ifndef __GRAPHICS_SUM_H__
#define __GRAPHICS_SUM_H__

#include "graphics_common.h"

#define MEAN_PIXEL(){\
    int result = ((int)(*src1) + (int)(*src2)) / 2;  \
    (*dst) = result; \
}

#define BASIC_MEAN(){\
    while (--n > 0) {  \
        MEAN_PIXEL();  \
        ++dst; ++src1; ++src2;  \
    }  \
}

#define ADDTO_PIXEL(){\
    int result = (*dst) + (*src);  \
    (*dst) = (result < 255) ? result : 255; \
}

#define BASIC_ADDTO(){\
    while (--n > 0) {  \
        ADDTO_PIXEL();  \
        ++dst, ++src;  \
    }  \
}

#define SUBFROM_PIXEL(){\
    int result = (*dst) - (*src);  \
    (*dst) = (result > 0) ? result : 0;  \
}

#define BASIC_SUBFROM(){\
    while(--n > 0) {  \
        SUBFROM_PIXEL();  \
        ++dst, ++src;  \
    } \
}


namespace ons_gfx {

#ifndef BPP16 // currently none of the fast CPU routines support 16bpp
    void imageFilterMean(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length);
    void imageFilterAddTo(unsigned char *dst, unsigned char *src, int length);
    void imageFilterSubFrom(unsigned char *dst, unsigned char *src, int length);
#endif //!BPP16

}

#endif // __GRAPHICS_SUM_H__
