/* -*- C++ -*-
 * 
 *  nbzdec.cpp - NBZ archive decoding
 *
 *  One user was having issues extracting an NSA archive using NBZ
 *  compression, only receiving the compressed files and not the
 *  original uncompressed files.
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
 *  ogapee@aqua.dti2.ne.jp
 *  Copyright (c) 2007-2010 "Uncle" Mion Sonozaki
 *  UncleMion@gmail.com
 *  (of DirectReader.cpp, code from which is used here).
 *
 *  Copyright (c) 2024 Galladite
 *
 *  galladite@yandex.com
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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef WIN32
#include <direct.h>
#include <windows.h>
#endif

#include "SarReader.h"
typedef SarReader reader;
#include "DirectReader.h" // For decode_NBZ

extern int errno;

int main( int argc, char **argv )
{
    DirPaths path;
    reader cSR(path);
    unsigned long length, buffer_length = 0;
    unsigned char *buffer = NULL;
    char file_name[512], dir_name[256], *outdir=NULL;
    unsigned int i, j, count;
    FILE *fp;
    struct stat file_stat;

    argc--; // skip command name
    argv++;
    while ( argc > 1 ){
        if ( !strcmp( argv[0], "-o" ) ){
            argc--;
            argv++;
            outdir = argv[0];
        }
        argc--;
        argv++;
    }
    if ( argc != 1 ){
        fprintf( stderr, "Usage: nbzdec [-o out_dir] nbz_file\n");
        exit(-1);
    }

    if (cSR.openForConvert( argv[0] ) != 0){
        fprintf( stderr, "can't open file %s\n", argv[0] );
        exit(-1);
    }
    count = cSR.getNumFiles();

    reader::FileInfo sFI;

    for ( i=0 ; i<count ; i++ ){
        sFI = cSR.getFileByIndex( i );
        
        length = cSR.getFileLength( sFI.name );

        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if (length > 0){
            unsigned int len;
            if ( (len = cSR.getFile( sFI.name, buffer )) != length ){
                fprintf( stderr, "file %s is not fully retrieved %d %lu\n", sFI.name, len, length );
                length = sFI.length;
                continue;
            }
        } else
            fprintf( stderr, "file %s is empty\n", sFI.name );
        printf( "extracting %d of %d, %lu bytes (%s)\n", i+1, count, length, sFI.name );
        
        if ( outdir )
            sprintf( file_name, "%s\\%s", outdir, sFI.name );
        else
            sprintf( file_name, "%s", sFI.name );
        for ( j=0 ; j<strlen(file_name) ; j++ ){
            file_name[j] = tolower(file_name[j]); //Mion: easier on the eyes
            if ( file_name[j] == '\\' ){
                file_name[j] = '/';
                strncpy( dir_name, file_name, j );
                dir_name[j] = '\0';

                /* If the directory doesn't exist, create it */
                if ( stat ( dir_name, &file_stat ) == -1 && errno == ENOENT )
                    mkdir(dir_name
#ifndef WIN32
                          , 0755
#endif
                         );
            }
        }
    
        if ( (fp = fopen( file_name, "wb" ) )){
            printf("    opening %s\n", file_name );
            fwrite( buffer, 1, length, fp );
            fclose(fp);
        }
        else{
            printf("    opening %s ... failed\n", file_name );
        }
    }
    
    if ( buffer ) delete[] buffer;
    
    exit(0);
}
