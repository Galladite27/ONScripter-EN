/* -*- Objective-C++ -*-
 * 
 *  ONSCocoaUIController.mm - Controller class for interacting with the
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

#import "ONSCocoaUIController.h"


@implementation ONSCocoaUIController

- (ONSCocoaUIController*)init {
    self = [super init];
    if(self != nil) {
        savePath = nil;
    }
    return self;
}

- (scriptErrorBoxResult)showScriptError:(NSString*)title message:(NSString*)message allowIgnore:(BOOL)allowIgnore
{
    [scriptErrorTitleField setStringValue:[NSString stringWithFormat:@"A script error has occurred: %@", title]];
    [scriptErrorDescriptionField setStringValue:message];
    [scriptErrorIgnoreButton setEnabled:allowIgnore];
    //[scriptErrorWindow makeKeyAndOrderFront:self];
    //[NSApp runModalForWindow:scriptErrorWindow];
    [NSApp beginSheet:scriptErrorWindow modalForWindow:[NSApp mainWindow] modalDelegate:self didEndSelector:@selector(errorSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
    [NSApp runModalForWindow:scriptErrorWindow];
    [NSApp endSheet:scriptErrorWindow];
    [scriptErrorWindow orderOut:self];
    return userAction;
}

- (void)errorSheetDidEnd:(NSWindow*)sheet returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
    
}
                                                                                                  
                                                                                                  
- (IBAction)dismissScriptError:(id)sender {
    [scriptErrorWindow close];
    userAction = SCRIPTERROR_IGNORE;
    [NSApp stopModal];
}

- (IBAction)dismissScriptErrorAndExit:(id)sender {
    [scriptErrorWindow close];
    userAction = SCRIPTERROR_QUIT;
    [NSApp stopModal];
}

- (IBAction)showKeyboardShortcuts:(id)sender
{
    NSString *locBookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"keyboard"  inBook:locBookName];
}


- (IBAction)showONScripterCorner:(id)sender
{
    NSURL *urlToOpen = [NSURL URLWithString:@"http://onscripter.unclemion.com"];
    [[NSWorkspace sharedWorkspace] openURL:urlToOpen];
}



- (void)setSavePath:(NSURL *)path
{
    if(savePath != nil) [savePath release];
    savePath = path;
    [savePath retain];
}



- (IBAction)showSaveFiles:(id)sender
{
    if(savePath != nil) {
        NSLog(@"open save path: %p", [savePath absoluteString]);
        [[NSWorkspace sharedWorkspace] openURL:savePath];
    }
}

@end
