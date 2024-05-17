//
//  AppDelegate.h
//  Emu
//
//  Created by Chris Marrin on 5/15/24.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property(readwrite) CFRunLoopObserverRef observerRef;

@end

