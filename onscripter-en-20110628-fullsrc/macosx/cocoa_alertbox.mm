/* -*- Objective-C++ -*-
 * 
 *  cocoa_alertbox.mm - display modal alert box, generally indicating a fatal
 *  error followed by program exit
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

#include "cocoa_alertbox.h"
#include "cocoa_modal_alert.h"
#include "SDLMain.h"
#include "ONSCocoaUIController.h"
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

namespace ONSCocoa {
    
    static NSStringEncoding enctypeToNSStringEncoding(encoding_type str_encoding) {
        switch(str_encoding) {
            case ENC_SJIS: return NSShiftJISStringEncoding;
            case ENC_UTF8: return NSUTF8StringEncoding;
            default: return NSUTF8StringEncoding;
        }
    }

    /**
     * Display an alert message with only an OK button
     */
    void alertbox(const char *title, const char *msg) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        modal_alert([NSString stringWithUTF8String:title], [NSString stringWithUTF8String:msg]);
        [pool drain];
    }
    
    /**
     * Display a box offering the user a choice; returns true if the default 
     * button was selected.
     */
    bool choicebox(const char *title, const char *msg, const char *def_btn, const char *alt_btn, encoding_type str_encoding) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSStringEncoding enc = enctypeToNSStringEncoding(str_encoding);
        int choice = NSRunAlertPanel([NSString stringWithCString:title encoding:enc], [NSString stringWithCString:msg encoding:enc], [NSString stringWithCString:def_btn encoding:enc], [NSString stringWithCString:alt_btn encoding:enc], nil);
        [pool drain];
        return choice == NSAlertDefaultReturn;
    }
    
    scriptErrorBoxResult scriptErrorBox(const char *title, const char *msg, bool allowIgnore, encoding_type str_encoding) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSStringEncoding enc = enctypeToNSStringEncoding(str_encoding);
        SDLMain *onsMain = [NSApp delegate];
        scriptErrorBoxResult res = [[onsMain getUIController] showScriptError:[NSString stringWithCString:title encoding:enc] message:[NSString stringWithCString:msg encoding:enc] allowIgnore:allowIgnore];
        [pool drain];
        return res;
    }
    
}
