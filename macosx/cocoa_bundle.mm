/* -*- Objective-C++ -*-
 * 
 *  cocoa_bundle.mm - functions for working with bundles
 *
 *  Copyright (c) 2010 Roto. All rights reserved.
 *
 *  roto@roto1.net
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

#include "cocoa_bundle.h"
#include <Foundation/Foundation.h>

namespace ONSCocoa {
    
    /**
     * Gets bundle path information.
     */
    void getBundleInfo(char **resources_path, char **bundle_path, char **bundle_name) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSBundle *bundle = [NSBundle mainBundle];
        
        const char *resources_str = [[bundle resourcePath] UTF8String];
        *resources_path = new char[strlen(resources_str) + 1];
        strcpy(*resources_path, resources_str);
        
        const char *bundle_str = [[[bundle bundlePath] stringByDeletingLastPathComponent] UTF8String];
        *bundle_path = new char[strlen(bundle_str) + 1];
        strcpy(*bundle_path, bundle_str);
    
        const char *bundlename_str = [[[[bundle bundlePath] lastPathComponent] stringByDeletingPathExtension] UTF8String];
        *bundle_name = new char[strlen(bundlename_str) + 1];
        strcpy(*bundle_name, bundlename_str);
        
        [pool drain];
    }
    
}
