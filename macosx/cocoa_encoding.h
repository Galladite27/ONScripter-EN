/* -*- Objective-C++ -*-
 * 
 *  cocoa_encoding.h - functions for converting text encodings
 *
 *  Copyright (c) 2010 Roto. All rights reserved.
 *
 *  roto@roto1.net
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

#ifndef __cocoa_encoding_h_
#define __cocoa_encoding_h_

namespace ONSCocoa {
    
    /**
     * Encoding type constants
     */
    typedef enum {
        ENC_SJIS,
        ENC_UTF8
    } encoding_type;
    
    /**
     * Converts a ShiftJIS string to UTF8.
     */
    void sjis_to_utf8(char *utf8_str, const char *sjis_str);
    
}

#endif // __cocoa_encoding_h_
