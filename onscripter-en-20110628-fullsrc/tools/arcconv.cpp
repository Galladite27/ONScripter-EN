/* -*- C++ -*-
 * 
 *  arcconv.cpp - Images in SAR/NSA/NS2 archive are re-scaled to 320x240 size
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

// Modified by Mion, December 2009, to consolidate SAR/NSA/ARC conversion code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <regex.h>

#ifdef SAR
#include "SarReader.h"
typedef SarReader reader;
#else
#include "NsaReader.h"
typedef NsaReader reader;
#endif

extern int scale_ratio_upper;
extern int scale_ratio_lower;

extern size_t rescaleJPEG( unsigned char *original_buffer, size_t length,
                           unsigned char **rescaled_buffer,
                           int quality, int num_of_cells );
extern size_t rescalePNG( unsigned char *original_buffer, size_t length,
                          unsigned char **rescaled_buffer, int num_of_cells );
extern size_t rescaleBMP( unsigned char *original_buffer,
                          unsigned char **rescaled_buffer,
                          bool output_png_flag, bool output_jpeg_flag, int quality, int num_of_cells );


#ifdef main
#undef main
#endif

struct ruleset {
    int num_cells;
    char *pattern;
    ruleset() : num_cells(1), pattern(NULL) {}
};
static ruleset *rules;
static unsigned int num_of_rules;

int match(const char *string, const char *pattern) {
    int status;
    regex_t re;

    if ((status = regcomp(&re, pattern, REG_EXTENDED|REG_ICASE|REG_NOSUB)) != 0)
        return status;

    status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);

    return status;
}

int parseRulesFile(const char *rulesfile)
{
    if (rules) delete[] rules;
    rules = NULL;
    num_of_rules = 0;
    FILE *fp;
    char buf[512], *tmp = NULL;
    unsigned int i, count;

    if ( (fp = fopen( rulesfile, "r" ) ) == NULL ){
        fprintf( stderr, "can't open rules file %s, ignoring\n", rulesfile );
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0){
        fprintf( stderr, "empty rules file %s, ignoring\n", rulesfile );
        return -1;
    }
    fseek(fp, 0, SEEK_SET);

    count = 0;
    char ch = fgetc(fp);
    while (!feof(fp)) {
        if (ch == '\n')
            count++;
        ch = fgetc(fp);
    }
    rules = new ruleset[count+1];

    fseek(fp, 0, SEEK_SET);
    i = 0;
    while (!feof(fp) && (i<count)) {
        if (fgets(buf, 512, fp) == NULL) break;
        buf[511] = '\0';
        tmp = strchr(buf, '\n');
        if (tmp) *tmp = '\0';
        if ((buf[0] < '0') || (buf[0] > '9')) continue;

        //have a line that starts with a digit; parse it
        rules[i].num_cells = 0;
        tmp = (char*)&buf;
        while ((*tmp >= '0') && (*tmp <= '9')){
            //read the positive integer
            rules[i].num_cells *= 10;
            rules[i].num_cells += *tmp++ - '0';
        }
        if (rules[i].num_cells == 0) continue;
        while ((*tmp == ' ') || (*tmp == '\t')) tmp++; //skip whitespace
        rules[i].pattern = new char[strlen(tmp)+1];
        strcpy(rules[i].pattern, tmp);
        i++;
    }
    num_of_rules = i;

    printf( "using rules file %s\n", rulesfile );
    for (i=0; i<num_of_rules; i++)
        printf("rule #%d: %d cells for pattern '%s'\n",
               i+1, rules[i].num_cells, rules[i].pattern);

    return 0;
}

void help()
{
#if defined(SAR)
    fprintf(stderr, "Usage: sarconv [-p] [-j] [-q quality] [-r rules_file] [-n num_cells]");
#elif defined (NS2)
    fprintf(stderr, "Usage: ns2conv [-offset num] [-p] [-j] [-q quality] [-r rules_file] [-n num_cells]");
#else
    fprintf(stderr, "Usage: nsaconv [-offset num] [-e] [-p] [-j] [-q quality] [-r rules_file] [-n num_cells]");
#endif
    fprintf(stderr, " src_width dst_width src_archive_file dst_archive_file\n");
#ifndef SAR
    fprintf(stderr, "           offset    ... 0 or more (rarely needed)\n");
#endif
    fprintf(stderr, "           quality    ... 0 to 100 (default 75)\n");
    fprintf(stderr, "           rules_file ... file with num_cell/filepattern pairings\n");
    fprintf(stderr, "           num_cells  ... number of components (cells and alphas)\n");
    fprintf(stderr, "           src_width  ... 640 or 800\n");
    fprintf(stderr, "           dst_width  ... 176, 220, 320, 360, 384, 640, etc.\n");
    exit(-1);
}

int main( int argc, char **argv )
{
    DirPaths path;
    reader cSR(path);
    unsigned long length, offset = 0, buffer_length = 0;
    unsigned char *buffer = NULL, *rescaled_buffer = NULL;
    unsigned int i, j, count;
#if defined(NS2)
    int archive_type = BaseReader::ARCHIVE_TYPE_NS2;
#elif defined(NSA)
    int archive_type = BaseReader::ARCHIVE_TYPE_NSA;
#endif
#ifndef SAR
    int nsa_offset = 0;
    bool enhanced_flag = false;
#endif
    bool bmp2jpeg_flag = false, bmp2png_flag = false;
    int quality = 75;
    int num_of_cells = 1;
    FILE *fp;

    argc--; // skip command name
    argv++;
    while (argc > 4){
        if      ( !strcmp( argv[0], "-j" ) )    bmp2jpeg_flag = true;
        else if ( !strcmp( argv[0], "-p" ) )    bmp2png_flag = true;
#ifdef NSA
        else if ( !strcmp( argv[0], "-e" ) )    enhanced_flag = true;
#endif
#ifndef SAR
        else if ( !strcmp( argv[0], "-offset" ) ){
            argc--;
            argv++;
            nsa_offset = atoi(argv[0]);
        }
#endif
        else if ( !strcmp( argv[0], "-q" ) ){
            argc--;
            argv++;
            quality = atoi(argv[0]);
        }
        else if ( !strcmp( argv[0], "-r" ) ){
            argc--;
            argv++;
            parseRulesFile(argv[0]);
        }
        else if ( !strcmp( argv[0], "-n" ) ){
            argc--;
            argv++;
            num_of_cells = atoi(argv[0]);
        }
        argc--;
        argv++;
    }
    if (argc != 4) help();

    scale_ratio_lower = atoi(argv[0]); // src width
    if (scale_ratio_lower!=640 && scale_ratio_lower!=800) help();
    
    scale_ratio_upper = atoi(argv[1]); // dst width

    if ( (fp = fopen( argv[3], "wb" ) ) == NULL ){
        fprintf( stderr, "can't open file %s for writing.\n", argv[3] );
        exit(-1);
    }
#ifdef SAR
    if (cSR.openForConvert( argv[2] ) != 0){
#else
    if (cSR.openForConvert( argv[2], archive_type, nsa_offset ) != 0){
#endif
        fprintf( stderr, "can't open file %s\n", argv[2] );
        exit(-1);
    }
    count = cSR.getNumFiles();

#if defined(SAR)
    printf("converting SAR archive %s (%d) to %s (%d)\n", argv[2], scale_ratio_lower,
           argv[3], scale_ratio_upper );
#elif defined(NS2)
    printf("converting NS2 archive %s (%d) to %s (%d)\n", argv[2], scale_ratio_lower,
           argv[3], scale_ratio_upper );
#else
    printf("converting NSA archive %s (%d) to %s (%d)\n", argv[2], scale_ratio_lower,
           argv[3], scale_ratio_upper );
#endif
    printf("conversion using %d cell%s for default (including alpha)\n",
           num_of_cells, num_of_cells == 1 ? "" : "s" );

    reader::FileInfo sFI;
    
    for ( i=0 ; i<count ; i++ ){
        sFI = cSR.getFileByIndex( i );
        if ( i==0 ) offset = sFI.offset;
        int num_cells = num_of_cells;

        for (j=0; j<num_of_rules; j++){
            if (match(sFI.name, rules[j].pattern) == 0){
                num_cells = rules[j].num_cells;
                break;
            }
        }
        printf( "converting %d of %d (%s)", i+1, count, sFI.name );
        fflush(stdout);

        length = cSR.getFileLength( sFI.name );
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }

        bool is_image = false;
        sFI.offset = offset;
        size_t orig_len = sFI.length, new_len = sFI.length;
        if ( ((strlen( sFI.name ) > 4) && !strcmp( sFI.name + strlen( sFI.name ) - 4, ".JPG")) ||
             ((strlen( sFI.name ) > 5) && !strcmp( sFI.name + strlen( sFI.name ) - 5, ".JPEG")) ){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            is_image = true;
            sFI.length = rescaleJPEG( buffer, length, &rescaled_buffer, quality, num_cells );
#ifdef SAR
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, true, rescaled_buffer );
#else
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, sFI.compression_type, true, rescaled_buffer );
#endif
        }
        else if ((strlen( sFI.name ) > 4) && !strcmp( sFI.name + strlen( sFI.name ) - 4, ".PNG")){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            is_image = true;
            sFI.length = rescalePNG( buffer, length, &rescaled_buffer, num_cells );
#ifdef SAR
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, true, rescaled_buffer );
#else
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, sFI.compression_type, true, rescaled_buffer );
#endif
        }
        else if ((strlen( sFI.name ) > 4) && !strcmp( sFI.name + strlen( sFI.name ) - 4, ".BMP")){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            is_image = true;
            sFI.length = rescaleBMP( buffer, &rescaled_buffer, bmp2png_flag, bmp2jpeg_flag, quality, num_cells );
#ifdef SAR
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, true, rescaled_buffer );
#else
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length,
                                   (enhanced_flag && !bmp2jpeg_flag && !bmp2png_flag)?BaseReader::NBZ_COMPRESSION:sFI.compression_type,
                                   true, rescaled_buffer );
#endif
        }
#ifdef NSA
        else if ( enhanced_flag && (strlen( sFI.name ) > 3) &&
                  !strcmp( sFI.name + strlen( sFI.name ) - 3, "WAV") ){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length, length,
                                   BaseReader::NBZ_COMPRESSION, true, buffer );
        }
#endif
        else{
#ifdef SAR
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length,
                                   sFI.original_length, false, buffer );
#else
            new_len = cSR.putFile( fp, i, sFI.offset, sFI.length,
                                   sFI.original_length, sFI.compression_type, false, buffer );
#endif
        }
        if (is_image)
            printf(", %d cell%s\n", num_cells,
                   num_cells == 1 ? "" : "s");
        else
            printf("\n");
        fflush(stdout);
        offset += new_len;
        if (orig_len != new_len){
            printf( "    %d -> %d (%d%%)\n", (int)orig_len, (int)new_len,
                    (int)(new_len * 100 / orig_len) );
        }
    }
#ifdef SAR
    cSR.writeHeader( fp );
#else
    cSR.writeHeader( fp, archive_type );
#endif

    fclose(fp);

    if ( rescaled_buffer ) delete[] rescaled_buffer;
    if ( buffer ) delete[] buffer;
    if ( rules ) delete[] rules;
    
    return 0;
}
