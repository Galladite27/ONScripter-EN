/* -*- C++ -*-
 * 
 *  arcdec.cpp - SAR/NSA/NS2 archive decoding
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
 *  (of sardec.cpp, the source file used as a base for this file).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2009-2010 "Uncle" Mion Sonozaki
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

// Modified by Mion, December 2009, to consolidate SAR/NSA/ARC extract code

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

#ifdef SAR
#include "SarReader.h"
typedef SarReader reader;
#else
#include "NsaReader.h"
typedef NsaReader reader;
#endif

extern int errno;

int main( int argc, char **argv )
{
    DirPaths path;
    reader cSR(path);
    unsigned long length, buffer_length = 0;
    unsigned char *buffer = NULL;
    char file_name[512], dir_name[256], *outdir=NULL;
    unsigned int i, j, count;
#if defined(NS2)
    int archive_type = BaseReader::ARCHIVE_TYPE_NS2;
#elif defined(NSA)
    int archive_type = BaseReader::ARCHIVE_TYPE_NSA;
#endif
#ifndef SAR
    int nsa_offset = 0;
#endif
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
#ifndef SAR
        else if ( !strcmp( argv[0], "-offset" ) ){
            argc--;
            argv++;
            nsa_offset = atoi(argv[0]);
        }
#endif
        argc--;
        argv++;
    }
    if ( argc != 1 ){
#if defined(SAR)
        fprintf( stderr, "Usage: sardec [-o out_dir] arc_file\n");
#elif defined(NS2)
        fprintf( stderr, "Usage: ns2dec [-offset num] [-o out_dir] arc_file\n");
#else
        fprintf( stderr, "Usage: nsadec [-offset num] [-o out_dir] arc_file\n");
#endif
        exit(-1);
    }

#ifdef SAR
    if (cSR.openForConvert( argv[0] ) != 0){
#else
    if (cSR.openForConvert( argv[0], archive_type, nsa_offset ) != 0){
#endif
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
