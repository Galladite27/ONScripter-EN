/* -*- C++ -*-
 * 
 *  arcmake.cpp - Create SAR/NSA/NS2 archive from a directory or files
 *
 *  Copyright (c) 2001-2006 Ogapee. All rights reserved.
 *  (of sarconv.cpp, the source file used as a base for this file).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2009 "Uncle" Mion Sonozaki
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#ifdef SAR
#include "SarReader.h"
typedef SarReader reader;
#else
#include "NsaReader.h"
typedef NsaReader reader;
#endif

#ifdef main
#undef main
#endif

struct dirnode {
    DIR *dir;
    int namelen;
    dirnode *next;
    dirnode *prev;
    dirnode() : dir(NULL), namelen(0), next(NULL), prev(NULL) {}
};
static dirnode dirn;

int processFile(reader::ArchiveInfo *ai, reader::FileInfo *fi,
                char *fullname, char *name, unsigned long &offset,
                bool enhanced_flag )
{
    FILE *fp = NULL;
    char magic[5];
    char fullpath[512];

    strcpy(fi->name, name);
    for (unsigned int j=0; j<strlen(fi->name); j++){
        if (fi->name[j] == '/')
            fi->name[j] = '\\';
    }
    strcpy(fullpath, fullname);
    for (unsigned int j=0; j<strlen(fullpath); j++){
        if ( (fullpath[j] == '/') || (fullpath[j] == '\\') )
            fullpath[j] = DELIMITER;
    }
    if ( (fp = fopen( fullpath, "rb" ) ) == NULL ){
        fprintf( stderr, "can't open file %s, skipping\n", fullpath );
        return -1;
    }
    fseek( fp, 0, SEEK_END );
    fi->length = ftell( fp );
    fseek( fp, 0, SEEK_SET );
    magic[0] = 0;
    int len = fread(magic, 1, 4, fp);
    magic[len] = 0;
    fclose(fp);

    if ( (strstr( fi->name, ".nbz" ) != NULL) ||
         (strstr( fi->name, ".NBZ" ) != NULL) )
        fi->compression_type = BaseReader::NBZ_COMPRESSION;
#ifdef NSA
    else if (enhanced_flag &&
             ( (( (strstr( fi->name, ".bmp" ) != NULL) ||
                  (strstr( fi->name, ".BMP" ) != NULL) ) &&
                (magic[0] == 'B') && (magic[1] == 'M')) ||
               (( (strstr( fi->name, ".wav" ) != NULL) ||
                  (strstr( fi->name, ".WAV" ) != NULL) ) &&
                (magic[0] == 'R') && (magic[1] == 'I') &&
                (magic[2] == 'F') && (magic[3] == 'F')) )){
        // If enhanced, use NBZ compression on (true) BMP & WAV files in NSA archive
        fi->compression_type = BaseReader::NBZ_COMPRESSION;
    }
#endif
    else
        fi->compression_type = BaseReader::NO_COMPRESSION;

    fi->original_length = fi->length;
    fi->offset = offset;
    offset += fi->length;
#if defined(SAR)
    ai->base_offset += strlen(fi->name) + 9;  //'\0', offset, length
#elif defined(NSA)
    ai->base_offset += strlen(fi->name) + 14; //'\0', compress, offset, length, orig length
#else
    ai->base_offset += strlen(fi->name) + 6;  //""s, length
#endif

    ai->num_of_files++;
    return 0;
}


int main( int argc, char **argv )
{
    DirPaths path;
    reader cSR(path);
    unsigned long length, offset = 0, buffer_length = 0;
    unsigned char *buffer = NULL;
    char file_name[512], file_path[512], *indir = NULL, *arcname = NULL;
    unsigned int i, count, total;
#if defined(NS2)
    int archive_type = BaseReader::ARCHIVE_TYPE_NS2;
#elif defined(NSA)
    int archive_type = BaseReader::ARCHIVE_TYPE_NSA;
#endif
    bool enhanced_flag = false;
    FILE *fp = NULL;
    char *fnptr = (char *)&file_name;

    argc--; // skip command name
    argv++;
    if (argc > 0){
        arcname = argv[0];
        argc--;
        argv++;
    }
#ifdef NSA
    if ( (argc > 0) && !strcmp( argv[0], "-e" ) ){
        enhanced_flag = true;
        argc--;
        argv++;
    }
#endif
    if ( (argc > 1) && !strcmp( argv[0], "-d" ) ){
        indir = argv[1];
        argc = 0;
    }
    if ( !indir && (argc < 1) ){
#if defined(SAR)
        fprintf( stderr, "Usage: sarmake arc_file -d in_dir\n");
        fprintf( stderr, "       sarmake arc_file in_file(s)\n");
#elif defined(NS2)
        fprintf( stderr, "Usage: ns2make arc_file -d in_dir\n");
        fprintf( stderr, "       ns2make arc_file in_file(s)\n");
#else
        fprintf( stderr, "Usage: nsamake arc_file [-e] -d in_dir\n");
        fprintf( stderr, "       nsamake arc_file [-e] in_file(s)\n");
#endif
        exit(-1);
    }

    reader::ArchiveInfo *sAI;
#ifdef SAR
    if ( (sAI = cSR.openForCreate( arcname ) ) == NULL ){
#else
    if ( (sAI = cSR.openForCreate( arcname, archive_type ) ) == NULL ){
#endif
        fprintf( stderr, "can't open file %s for writing.\n", arcname );
        exit(-1);
    }

    if (indir)
        printf("using directory %s\n", indir);

    total = 0;
    reader::FileInfo *sFI = NULL;
    
    for (int t=0; t<2; t++){
        // recurse through directories
        // first pass - count, second pass - process file info
        fnptr = (char *)&file_name;
        dirnode *cur = &dirn;
        int j = 0;
        count = 0;
        if (indir) j = -1;
        cur->dir = NULL;
        cur->next = cur->prev = NULL;
        cur->namelen = 0;

        while (j < argc) {
            if (indir){
                cur->dir = opendir(indir);
                if (! cur->dir){
                    if (errno == ENOTDIR)
                        fprintf(stderr, "'%s' is not a directory\n", indir);
                    else
                        fprintf(stderr, "can't open directory '%s'\n", indir);
                    exit(-1);
                }
                sprintf(fnptr, "%s%c", indir, DELIMITER);
                fnptr += strlen(indir) + 1;
            } else {
                if (strlen(argv[j]) > 255){
                    fprintf( stderr, "filename too long: %s\n", argv[j] );
                    continue;
                }
                sprintf(fnptr, "%s", argv[j]);
                cur->dir = opendir(file_name);
                if (! cur->dir){
                    if (errno == ENOTDIR){
                        if (t){
                            processFile(sAI, sFI, (char*)&file_name,
                                        fnptr, offset, enhanced_flag);
                            sFI++;
                        }
                        count++;
                    }
                    else {
                        fprintf(stderr, "can't open directory '%s'\n", fnptr);
                        exit(-1);
                    }
                    j++;
                    continue;
                } else {
                    cur->namelen = strlen(fnptr);
                    fnptr[cur->namelen] = DELIMITER;
                    cur->namelen++;
                }
            }
            struct dirent *drnt = NULL;
            while (1){
                if ((drnt = readdir(cur->dir)) == NULL){
                    closedir(cur->dir);
                    if (cur->prev == NULL) break;
                    cur = cur->prev;
                    delete cur->next;
                    cur->next = NULL;
                } else {
                    if (drnt->d_name[0] != '.'){ //don't process dotted files/dirs
                        sprintf(fnptr + cur->namelen, "%s", drnt->d_name);
                        DIR *dir = opendir(file_name);
                        if (dir){
                            cur->next = new dirnode;
                            cur->next->prev = cur;
                            cur->next->namelen = cur->namelen + strlen(drnt->d_name);
                            cur = cur->next;
                            cur->dir = dir;
                            fnptr[cur->namelen] = DELIMITER;
                            cur->namelen++;
                        } else {
                            if (t){
                                processFile(sAI, sFI, (char*)&file_name,
                                            fnptr, offset, enhanced_flag);
                                sFI++;
                            }
                            count++;
                        }
                    }
                }
            }
            j++;
        }
        total = count;
        if (!t){
            sAI->num_of_files = 0;
#ifdef NS2
            sAI->base_offset = 5;
#else
            sAI->base_offset = 6;
#endif
            offset = 0;
            sAI->fi_list = new reader::FileInfo[ count ];
            sFI = sAI->fi_list;
        }
    }
#if defined(SAR)
    printf("creating SAR archive '%s', %d files total\n", arcname, total);
#elif defined(NS2)
    printf("creating NS2 archive '%s', %d files total\n", arcname, total);
#else
    printf("creating NSA archive '%s', %d files total\n", arcname, total);
#endif
    fflush(stdout);

    sFI = sAI->fi_list;
    for ( i=0 ; i<sAI->num_of_files ; i++, sFI++ )
        sFI->offset += sAI->base_offset;
#ifdef SAR
    cSR.writeHeader( sAI->file_handle );
#else
    cSR.writeHeader( sAI->file_handle, archive_type );
#endif
    sFI = sAI->fi_list;
#ifdef NSA
    unsigned long offset_sub = 0;
#endif
    for ( i=0 ; i<sAI->num_of_files ; i++, sFI++ ){
        //now add the file
        printf( "adding %d of %d (%s), length=%d\n", i+1, sAI->num_of_files, sFI->name, (int)sFI->original_length );
        fflush(stdout);
        length = sFI->original_length;
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        sprintf(fnptr, "%s", sFI->name);
        sprintf(file_path, "%s", file_name);
        for (unsigned int j=0; j<strlen(file_path); j++) {
            if ( (file_path[j] == '\\') || (file_path[j] == '/') )
                file_path[j] = DELIMITER;
        }
        if ( (fp = fopen( file_path, "rb" ) ) == NULL ){
            fprintf( stderr, "can't open file %s, exiting\n", file_path );
            exit(-1);
        }
#ifdef NSA
        if (!enhanced_flag)
            sFI->compression_type = BaseReader::NO_COMPRESSION;
        sFI->offset -= offset_sub;
        cSR.addFile( sAI, fp, i, sFI->offset, buffer );
        if (sFI->original_length != sFI->length){
            offset_sub += sFI->original_length - sFI->length;
            printf( "    NBZ compressed: %d -> %d (%d%%)\n",
                    (int)sFI->original_length, (int)sFI->length,
                    (int)(sFI->length * 100 / sFI->original_length) );
        }
#else
        cSR.addFile( sAI, fp, i, sFI->offset, buffer );
#endif
        fclose(fp);
    }
#ifdef NSA
    cSR.writeHeader( sAI->file_handle, archive_type );
#endif
    fclose(sAI->file_handle);

    if ( buffer ) delete[] buffer;
    
    return 0;
}
