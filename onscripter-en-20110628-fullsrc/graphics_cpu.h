/* -*- C++ -*-
 * 
 *  graphics_cpu.h - ons_gfx namespace cpu-checking routines
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

#ifndef __GRAPHICS_CPU_H__
#define __GRAPHICS_CPU_H__

#include "graphics_common.h"

#if defined (USE_X86_GFX) && !defined(MACOSX)
#include <cpuid.h>
#endif

namespace ons_gfx {

    enum{
        CPUF_NONE           =  0,
        CPUF_X86_MMX        =  1,
        CPUF_X86_SSE        =  2,
        CPUF_X86_SSE2       =  4,
        CPUF_PPC_ALTIVEC    =  8
    };

    void setCpufuncs(unsigned int func);
    unsigned int getCpufuncs();

}

#endif // __GRAPHICS_CPU_H__
