//
//  ONSApplication.m
//  ONScripter
//
//  Created by Brian Mayton on 6/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ONSApplication.h"
#import "SDL.h"
#import "SDLMain.h"



@implementation ONSApplication

/*
- (ONSApplication*)init {
    self = [super init];
    if(!self) return nil;
    
    savePath = nil;
    
    return self;
}  */ 

/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}




@end
