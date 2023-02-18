/* -*- C++ -*-
 * 
 *  batchconv.cpp - Batch rescale of image files
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
#ifdef WIN32
#include <windows.h>
#endif

#include <regex.h>

#ifdef WIN32
#define DELIMITER '\\'
#else
#define DELIMITER '/'
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

struct dirnode {
    DIR *dir;
    int namelen;
    dirnode *next;
    dirnode *prev;
    dirnode() : dir(NULL), namelen(0), next(NULL), prev(NULL) {}
};
static dirnode dirn;
static int quality;
static int num_of_cells;
static bool in_place;

static unsigned long buffer_length = 0;
static unsigned char *buffer = NULL, *rescaled_buffer = NULL;

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

int processFile(char *fullname, char *name, char *outname )
{
    char dir_name[256];
    FILE *fp = NULL, *outfp = NULL;
    struct stat file_stat;
    unsigned char *out_buffer = NULL;
    unsigned long length, new_length;
    unsigned int i;
    int num_cells = num_of_cells;

    if ( (fp = fopen( fullname, "rb" ) ) == NULL ){
        fprintf( stderr, "can't open file %s, skipping\n", fullname );
        return -1;
    }

    for ( i=0 ; i<strlen(fullname) ; i++ ){
        if ( (fullname[i] == '\\') || (fullname[i] == '/') )
            fullname[i] = DELIMITER;
    }

    for ( i=0 ; i<strlen(outname) ; i++ ){
        if ( (outname[i] == '\\') || (outname[i] == '/') ){
            outname[i] = DELIMITER;
            strncpy( dir_name, outname, i );
            dir_name[i] = '\0';

            /* If the directory doesn't exist, create it */
            if ( stat ( dir_name, &file_stat ) == -1 && errno == ENOENT )
#ifdef WIN32
                CreateDirectory(dir_name, 0);
#else
                mkdir( dir_name, 00755 );
#endif
        }
    }

    if ( (outfp = fopen( outname, "ab" ) ) == NULL ){
        fprintf( stderr, "can't open %s for writing, skipping\n", outname );
        return -1;
    }
    fclose(outfp);

    fseek( fp, 0, SEEK_END );
    length = ftell( fp );
    if ( length == 0 ){
        fprintf( stderr, "file %s is empty (not converting)\n", fullname );
        outfp = fopen( outname, "wb" );
        fclose(outfp);
        return -1;
    }
    new_length = length;
    fseek( fp, 0, SEEK_SET );

    for (i=0; i<num_of_rules; i++){
        if (match(name, rules[i].pattern) == 0){
            num_cells = rules[i].num_cells;
            break;
        }
    }

    bool is_image = false;
    if ( ((strlen( name ) > 4) &&
          ( !strcmp( name + strlen( name ) - 4, ".JPG") ||
            !strcmp( name + strlen( name ) - 4, ".jpg") )) ||
         ((strlen( name ) > 5) &&
          ( !strcmp( name + strlen( name ) - 5, ".JPEG") ||
            !strcmp( name + strlen( name ) - 5, ".jpeg") )) ){
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if (fread( buffer, 1, length, fp ) != length){
            fprintf( stderr, "file %s can't be retrieved %ld\n", fullname, length );
            return -1;
        }
        is_image = true;
        new_length = rescaleJPEG( buffer, length, &rescaled_buffer, quality,
                                  num_cells );
        out_buffer = rescaled_buffer;
    }
    else if ((strlen( name ) > 4) &&
             ( !strcmp( name + strlen( name ) - 4, ".PNG") ||
               !strcmp( name + strlen( name ) - 4, ".png") )){
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if (fread( buffer, 1, length, fp ) != length){
            fprintf( stderr, "file %s can't be retrieved %ld\n", fullname, length );
            return -1;
        }
        is_image = true;
        new_length = rescalePNG( buffer, length, &rescaled_buffer, num_cells );
        out_buffer = rescaled_buffer;
    }
    else if ((strlen( name ) > 4) &&
             ( !strcmp( name + strlen( name ) - 4, ".BMP") ||
               !strcmp( name + strlen( name ) - 4, ".bmp") )){
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if (fread( buffer, 1, length, fp ) != length){
            fprintf( stderr, "file %s can't be retrieved %ld\n", fullname, length );
            return -1;
        }
        is_image = true;
        new_length = rescaleBMP( buffer, &rescaled_buffer, false, false,
                                 quality, num_cells );
        out_buffer = rescaled_buffer;
    }
    else if (!in_place){
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if (fread( buffer, 1, length, fp ) != length){
            fprintf( stderr, "file %s can't be retrieved %ld\n", fullname, length );
            return -1;
        }
        out_buffer = buffer;
    }
    else {
        printf( ": %s [%ld] unchanged\n", fullname, length );
        return 0;
    }
    fclose(fp);

    printf( ": %s [%ld] -> %s [%ld] (%ld%%)", fullname, length,
            outname, new_length, new_length * 100 / length );
    if (is_image)
        printf(", %d cell%s\n", num_cells,
               num_cells == 1 ? "" : "s");
    else
        printf("\n");
    fflush(stdout);

    outfp = fopen( outname, "wb" );
    fwrite( out_buffer, 1, new_length, outfp );
    fclose(outfp);

    return 0;
}

void help()
{
    fprintf(stderr, "Usage: batchconv [-r rules_file] [-n num_cells] [-o out_dir]");
    fprintf(stderr, " src_width dst_width -d in_dir\n");
    fprintf(stderr, "       batchconv [-r rules_file] [-n num_cells] [-o out_dir]");
    fprintf(stderr, " src_width dst_width in_file(s)\n");
    fprintf(stderr, "           rules_file ... file with num_cell/filepattern pairings\n");
    fprintf(stderr, "           num_cells  ... number of components (cells and alphas)\n");
    fprintf(stderr, "           src_width  ... 640 or 800\n");
    fprintf(stderr, "           dst_width  ... 176, 220, 320, 360, 384, 640, etc.\n");
    exit(-1);
}

int main( int argc, char **argv )
{
    char file_name[512], out_file_name[512], *indir = NULL, *outdir=NULL;
    char *fnptr = (char *)&file_name, *outfnptr = (char *)&out_file_name;
    unsigned int count;

    quality = 75;
    num_of_cells = 1;
    in_place = false;

    argc--; // skip command name
    argv++;
    while (argc > 2){
        if      ( !strcmp( argv[0], "-r" ) ){
            argc--;
            argv++;
            parseRulesFile(argv[0]);
        }
        else if ( !strcmp( argv[0], "-n" ) ){
            argc--;
            argv++;
            num_of_cells = atoi(argv[0]);
        }
        else if ( !strcmp( argv[0], "-o" ) ){
            argc--;
            argv++;
            outdir = argv[0];
        }
        else break;
        argc--;
        argv++;
    }
    if (argc < 3) help();

    scale_ratio_lower = atoi(argv[0]); // src width
    if (scale_ratio_lower!=640 && scale_ratio_lower!=800) help();

    scale_ratio_upper = atoi(argv[1]); // dst width
    argc -= 2;
    argv += 2;
    
    if ( (argc > 1) && !strcmp( argv[0], "-d" ) ){
        indir = argv[1];
        argc = 0;
    }
    if ( !indir && (argc < 1) ) help();

    if (indir)
        printf("using directory %s\n", indir);
    if (!indir && !outdir){
        in_place = true;
        printf("converting in-place\n");
    }
    printf("conversion using %d cell%s for default (including alpha)\n",
           num_of_cells, num_of_cells == 1 ? "" : "s" );

    // recurse through directories
    if (outdir){
        sprintf(outfnptr, "%s\\", outdir);
        outfnptr += strlen(outdir) + 1;
    }
    dirnode *cur = &dirn;
    int j = 0;
    count = 0;
    if (indir && (argc == 0)) j = -1;
    cur->dir = NULL;
    cur->next = cur->prev = NULL;
    cur->namelen = 0;

    if (indir){
        cur->dir = opendir(indir);
        if (! cur->dir){
            if (errno == ENOTDIR)
                fprintf(stderr, "'%s' is not a directory\n", indir);
            else
                fprintf(stderr, "can't open directory '%s'\n", indir);
            exit(-1);
        }
        sprintf(fnptr, "%s\\", indir);
        fnptr += strlen(indir) + 1;
    }
    while (j < argc) {
        if (j >= 0){
            if (strlen(argv[j]) > 255){
                fprintf( stderr, "filename too long: %s\n", argv[j] );
                continue;
            }
            sprintf(fnptr, "%s", argv[j]);
            cur->dir = opendir(file_name);
            if (! cur->dir){
                if (errno == ENOTDIR){
                    sprintf(outfnptr, "%s", argv[j]);
                    processFile((char*)&file_name, fnptr, (char*)&out_file_name);
                    count++;
                }
                else {
                    fprintf(stderr, "can't open directory '%s'\n", file_name);
                    exit(-1);
                }
                j++;
                continue;
            } else {
                cur->namelen = strlen(fnptr);
                fnptr[cur->namelen] = '\\';
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
                        fnptr[cur->namelen] = '\\';
                        cur->namelen++;
                    } else {
                        sprintf(outfnptr, "%s", fnptr);
                        processFile((char*)&file_name, fnptr, (char*)&out_file_name);
                        count++;
                    }
                }
            }
        }
        j++;
    }

    if ( rescaled_buffer ) delete[] rescaled_buffer;
    if ( buffer ) delete[] buffer;
    if ( rules ) delete[] rules;
    
    return 0;
}
