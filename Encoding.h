/* -*- C++ -*-
 * 
 *  Encoding.h - Character encoding handler
 *
 *  Copyright (c) 2019-2020 Ogapee. All rights reserved.
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

#ifndef __ENCODING_H__
#define __ENCODING_H__

class Encoding
{
public:
    enum { CODE_CP932 = 0,
           CODE_UTF8 = 1
    };
    
    Encoding();
    ~Encoding();

    void setEncoding(int code);
    int getEncoding(){ return code; };
    
    char getTextMarker();

    int getBytes(unsigned char ch, int code = -1);
    int getNum(const unsigned char *buf);
    
    unsigned short getUTF16(const char *text, int code = -1);

private:
    int code;
};

#endif // __ENCODING_H__ 

