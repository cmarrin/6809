//
//  AppDelegate.m
//  Emu
//
//  Created by Chris Marrin on 5/15/24.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    CFRunLoopAddObserver(CFRunLoopGetMain(), self.observerRef, kCFRunLoopCommonModes);
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
