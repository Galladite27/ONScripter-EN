/*
 *  PathProvider.h - an abstract class for handling DirPaths
 */

#ifndef __PATHPROVIDER_H__
#define __PATHPROVIDER_H__

#include <cstdlib>


class PathProvider
{
public:
    virtual ~PathProvider() {};
    virtual const char *get_path( int n ) const = 0;
    virtual const char *get_all_paths() const = 0;
    virtual int get_num_paths() const = 0;
    virtual size_t max_path_len() const = 0;
};

#endif // __PATHPROVIDER_H__
