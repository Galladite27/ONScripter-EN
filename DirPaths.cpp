/* -*- C++ -*-
 * 
 *  DirPaths.cpp - class that provides multiple directory paths for ONScripter-EN
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

#include "DirPaths.h"

DirPaths::DirPaths( const char *new_paths )
: num_paths(0), paths(NULL), all_paths(NULL)
{
    //printf("DirPaths cons\n");
    add(new_paths);
}

DirPaths::DirPaths( const DirPaths& dp )
{
    //printf("DirPaths copy cons\n");
    set(dp);
}

DirPaths& DirPaths::operator =( const DirPaths &dp )
{
    if (this != &dp){
        //printf("DirPaths =op\n");
        set(dp);
    }
    return *this;
}

void DirPaths::set( const DirPaths &dp )
{
    memcpy(this, &dp, sizeof(DirPaths));
    if (paths != NULL) {
        char **old_paths = paths;
        paths = new char*[num_paths + 1];
        for (int i=0; i<=num_paths; i++) {
            if (old_paths[i] != NULL) {
                paths[i] = new char[strlen(old_paths[i]) + 1];
                strcpy(paths[i], old_paths[i]);
            } else
                paths[i] = NULL;
        }
    }
    if (all_paths != NULL) {
        char *old_all_paths = all_paths;
        all_paths = new char[strlen(old_all_paths) + 1];
        strcpy(all_paths, old_all_paths);
    }
}

DirPaths::~DirPaths()
{
    if (paths != NULL) {
        char **ptr = paths;
        for (int i=0; i<num_paths; i++) {
            if (*ptr != NULL) {
                delete[] *ptr;
            }
            ptr++;
        }
        delete[] paths;
    }
    if (all_paths != NULL) {
        delete[] all_paths;
    }
}

void DirPaths::add( const DirPaths &dp )
{
    add(dp.get_all_paths());
}

void DirPaths::add( const char *new_paths )
{
    //don't add null paths
    if (new_paths == NULL) return;
    
    if (*new_paths == '\0') {
        //don't add empty string paths, unless there are none
        if (num_paths == 0) num_paths = 1;
        return;
    }

    fprintf(stderr, "Adding path: %s\n", new_paths);

    if (all_paths != NULL) delete[] all_paths;
    all_paths = NULL;

    if (paths == NULL) num_paths = 0;

    int cur_num = num_paths;
    num_paths++;
    const char *ptr1 = new_paths;
    while (*ptr1 != '\0') {
        char tmp = *ptr1++;
        if ((tmp == PATH_DELIMITER) && (*ptr1 != '\0') &&
            (*ptr1 != PATH_DELIMITER)) {
            // ignore empty string paths
            num_paths++;
        }
    }

    if (paths != NULL) {
        // allocate a new "paths", copy over any existing ones
        //printf("DirPaths::add(\"%s\")\n", new_paths);
        char **old_paths = paths;
        paths = new char*[num_paths + 1];
        for (int i=0; i<cur_num; i++) {
            paths[i] = old_paths[i];
        }
        delete[] old_paths;
    } else {
        //printf("DirPaths(\"%s\")\n", new_paths);
        paths = new char*[num_paths + 1];
        if (cur_num == 1) {
            //was an "empty path"
            //keep the "" as the first of the paths by making it a "."
            paths[0] = new char[3];
            snprintf(paths[0], 2, "%s%c", ".", DELIMITER);
        }
    }
    const char *ptr2 = ptr1 = new_paths;
    do {
        while ((*ptr2 != '\0') && (*ptr2 != PATH_DELIMITER)) ptr2++;
        if (ptr2 == ptr1) {
            if (*ptr2 == '\0') break;
            ptr1++;
            ptr2++;
            continue;
        } else {
            paths[cur_num] = new char[ptr2 - ptr1 + 2];
            char *dptr = paths[cur_num];
            if (ptr1 != ptr2) {
                while (ptr1 != ptr2) *dptr++ = *ptr1++;
                if (*(dptr-1) != DELIMITER) {
                    // put a slash on the end if there isn't one already
                    *dptr++ = DELIMITER;
                }
            }
            *dptr = '\0';
        }
        if (*ptr2 != '\0') {
            ptr1++;
            ptr2++;
        }
        //printf("added path: \"%s\"\n", paths[cur_num]);
        cur_num++;
    } while (*ptr2 != '\0');

    num_paths = cur_num;
    paths[num_paths] = NULL;

    // construct all_paths
    size_t len = 0;
    for (cur_num=0; cur_num<num_paths; cur_num++) {
        len += strlen(paths[cur_num]) + 1;
    }

    if (all_paths != NULL) {
        delete[] all_paths;
    }
    all_paths = new char[len];

    char *dptr = all_paths;
    for (cur_num=0; cur_num<(num_paths-1); cur_num++) {
        size_t curlen = strlen(paths[cur_num]) + 1;
        snprintf(dptr, len, "%s%c", paths[cur_num], PATH_DELIMITER);
        dptr += curlen;
        len -= curlen;
    }
    snprintf(dptr, len, "%s", paths[cur_num]);
}

const char* DirPaths::get_path( int n ) const
{
    if ((n == 0) || ((n > 0) && (n < num_paths))) {
        if (paths)
            return paths[n];
        else
            return "";
    }
    return NULL;
}

// Returns a delimited string containing all paths
const char* DirPaths::get_all_paths() const
{
    if (all_paths != NULL)
        return all_paths;
    else
        return "";
}

int DirPaths::get_num_paths() const
{
    return num_paths;
}

// Returns the length of the longest path
size_t DirPaths::max_path_len() const
{
    size_t len = 0;
    if (paths != NULL) {
        for (int i=0; i<num_paths;i++) {
            if (strlen(paths[i]) > len)
                len = strlen(paths[i]);
        }
    }
    return len;
}

