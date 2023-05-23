/* -*- C++ -*-
 * 
 *  Encoding.cpp - Character encoding handler
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

#include "Encoding.h"

extern unsigned short convSJIS2UTF16(unsigned short in);
extern unsigned short convUTF8ToUTF16(const char **src);

Encoding::Encoding()
{
    code = CODE_CP932;
}

Encoding::~Encoding()
{
}

void Encoding::setEncoding(int code)
{
    this->code = code;
}

int Encoding::getBytes(unsigned char ch, int code)
{
    if (code == -1) code = this->code;
    
    if (code == CODE_CP932){
        if ((ch & 0xe0) == 0xe0 || (ch & 0xe0) == 0x80) return 2;
    }
    else{
        if (0    <= ch && ch < 0x80) return 1;
        if (0xc0 <= ch && ch < 0xe0) return 2;
        if (0xe0 <= ch && ch < 0xf0) return 3;
        if (0xf0 <= ch && ch < 0xf8) return 4;
    }

    return 1;
}

int Encoding::getNum(const unsigned char *buf)
{
    int n = 0;
    
    while(buf[0] != 0){
        int n2 = getBytes(buf[0]);
        n++;
        if (n2 > 1) n++;
        buf += n2;
    }

    return n;
}

char Encoding::getTextMarker()
{
    if (code == CODE_UTF8)
        return '^';
    
    return '`';
}

unsigned short Encoding::getUTF16(const char *text, int code)
{
    unsigned short unicode = 0;

    if (code == -1) code = this->code;
    
    if (code == CODE_CP932){
        if ((text[0] & 0xe0) == 0xe0 || (text[0] & 0xe0) == 0x80){
            unsigned index = ((unsigned char*)text)[0];
            index = index << 8 | ((unsigned char*)text)[1];
            unicode = convSJIS2UTF16(index);
        }
        else{
            if ((text[0] & 0xe0) == 0xa0 || (text[0] & 0xe0) == 0xc0)
                unicode = ((unsigned char*)text)[0] - 0xa0 + 0xff60;
            else
                unicode = text[0];
        }
    }
    else{
        unicode = convUTF8ToUTF16(&text);
    }

    return unicode;
}
