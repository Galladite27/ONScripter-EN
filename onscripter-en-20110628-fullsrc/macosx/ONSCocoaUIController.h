/* -*- Objective-C++ -*-
 * 
 *  ONSCocoaUIController.h - Controller class for interacting with the
 *  user via a native Cocoa user interface
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

#import <Cocoa/Cocoa.h>
#import "cocoa_alertbox_result.h"

@interface ONSCocoaUIController : NSObject {
    IBOutlet NSWindow *scriptErrorWindow;
    IBOutlet NSTextField *scriptErrorTitleField;
    IBOutlet NSTextField *scriptErrorDescriptionField;
    IBOutlet NSButton *scriptErrorIgnoreButton;
    
    NSURL *savePath;
    
    scriptErrorBoxResult userAction;
}

- (IBAction)dismissScriptError:(id)sender;
- (IBAction)dismissScriptErrorAndExit:(id)sender;
- (scriptErrorBoxResult)showScriptError:(NSString*)title message:(NSString*)message allowIgnore:(BOOL)allowIgnore;

- (IBAction)showKeyboardShortcuts:(id)sender;
- (IBAction)showONScripterCorner:(id)sender;
- (IBAction)showSaveFiles:(id)sender;
- (void)setSavePath:(NSURL *)path;

@end
