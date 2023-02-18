/* -*- C++ -*-
 *
 *  SarReader.cpp - Reader from a SAR archive for ONScripter-EN
 *
 *  Copyright (c) 2001-2008 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2007-2010 "Uncle" Mion Sonozaki
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

// Modified by Mion, December 2009, to add NS2 archive support.

#ifndef __SAR_READER_H__
#define __SAR_READER_H__

#include "DirectReader.h"

class SarReader : public DirectReader
{
public:
    SarReader( DirPaths &path, const unsigned char *key_table=NULL );
    ~SarReader();

    int open( const char *name=NULL );
    int close();
    const char *getArchiveName() const;
    int getNumFiles();
    
    size_t getFileLength( const char *file_name );
    size_t getFile( const char *file_name, unsigned char *buf, int *location=NULL );
    struct FileInfo getFileByIndex( unsigned int index );

#ifdef TOOLS_BUILD
    int openForConvert( const char *name );
    ArchiveInfo* openForCreate( const char *name );
    int writeHeader( FILE *fp );
    size_t addFile( ArchiveInfo *ai, FILE *newfp, int no, size_t offset, unsigned char *buffer );
    size_t putFile( FILE *fp, int no, size_t offset, size_t length, size_t original_length, bool modified_flag, unsigned char *buffer );
#endif
protected:
    struct ArchiveInfo archive_info;
    struct ArchiveInfo *root_archive_info, *last_archive_info;
    int num_of_sar_archives;

    int readArchive( ArchiveInfo *ai, int archive_type = ARCHIVE_TYPE_SAR, int offset = 0 );
    int getIndexFromFile( ArchiveInfo *ai, const char *file_name );
    size_t getFileSub( ArchiveInfo *ai, const char *file_name, unsigned char *buf );

#ifdef TOOLS_BUILD
    int writeHeaderSub( ArchiveInfo *ai, FILE *fp, int archive_type = ARCHIVE_TYPE_SAR, int offset = 0 );
    size_t putFileSub( ArchiveInfo *ai, FILE *fp, int no, size_t offset, size_t length, size_t original_length, int compression_type, bool modified_flag, unsigned char *buffer );
#endif
};

#endif // __SAR_READER_H__
