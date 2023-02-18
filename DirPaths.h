/* -*- C++ -*-
 * 
 *  DirPaths.h - class that provides multiple directory paths for ONScripter-EN
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

#ifndef __DIR_PATHS__
#define __DIR_PATHS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define DELIMITER '\\'
#define PATH_DELIMITER ';'
#else
#define DELIMITER '/'
#define PATH_DELIMITER ':'
#endif


class DirPaths
{
public:
    DirPaths( const char *new_paths=NULL );
    DirPaths( const DirPaths &dp );
    DirPaths& operator =( const DirPaths &dp );
    ~DirPaths();
    
    void add( const char *new_paths );
    void add( char *new_paths ) { add( (const char*)new_paths ); }
    void add( const DirPaths &dp );
    const char *get_path( int n ) const; //paths numbered from 0 to num_paths-1
    const char *get_all_paths() const;
    int get_num_paths() const;
    size_t max_path_len() const;

private:
    void set( const DirPaths &dp ); //called by copy cons & =op

    int num_paths;
    char **paths;
    char *all_paths;
};

#endif // __DIR_PATHS__
