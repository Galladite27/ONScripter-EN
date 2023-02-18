/* -*- Objective-C++ -*-
 * 
 *  cocoa_directories.mm - functions for getting and manipulating system
 *  directories
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

#include "cocoa_directories.h"
#include "cocoa_modal_alert.h"
#include "SDLMain.h"

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <stdlib.h>


namespace ONSCocoa {
    
    /**
     * Gets the path to the game's directory in the user's Application Support directory.
     * If it doesn't exist, it is created.
     *
     * @param path double pointer to C string; memory for the result will be allocated with new[]
     *  and the string will be stored here.  You must delete[] the result if you don't want to
     *  keep it.
     * @param gameid used as directory name
     */
    void getGameAppSupportPath(char **path, const char *gameid) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        
        NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([dirs count] < 1) {
            modal_alert(@"Application Support Folder Missing", @"Your Application Support folder can't be found.  This shouldn't happen; contact the ONScripter-EN developers for assistance.");
            exit(1);
        }
        NSString *gameAppSuportFolder = [[dirs objectAtIndex:0] stringByAppendingPathComponent:[NSString stringWithUTF8String:gameid]];
        
        NSFileManager *fileManager = [[NSFileManager alloc] init];
        // DEPRECATION: createDirectoryAtPath:attributes: is deprecated as of 10.5, no 10.4 SDK
        // compatible alternatives are available
        [fileManager createDirectoryAtPath:gameAppSuportFolder attributes:nil];
        [fileManager release];
        
        SDLMain *onsMain = [NSApp delegate];
        [[onsMain getUIController] setSavePath:[NSURL fileURLWithPath:gameAppSuportFolder]];
                
        const char *path_str = [gameAppSuportFolder UTF8String];
        *path = new char[strlen(path_str) + 1];
        strcpy(*path, path_str);
        
        [pool drain];
    }
    
}
