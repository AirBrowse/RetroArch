/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2013-2014 - Jason Fetters
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __COCOA_COMMON_H
#define __COCOA_COMMON_H

#include <Foundation/Foundation.h>

#include "../../core_info.h"
#include "../../playlist.h"
#include "../../settings.h"
#include "../../menu/menu.h"

#if defined(HAVE_COCOATOUCH)
#include <UIKit/UIKit.h>

#include <CoreLocation/CoreLocation.h>
#import <AVFoundation/AVCaptureOutput.h>



/*********************************************/
/* RAMenuBase                                */
/* A menu class that displays RAMenuItemBase */
/* objects.                                  */
/*********************************************/
@interface RAMenuBase : UITableViewController
@property (nonatomic) NSMutableArray* sections;
@property (nonatomic) BOOL hidesHeaders;

- (id)initWithStyle:(UITableViewStyle)style;
- (id)itemForIndexPath:(NSIndexPath*)indexPath;
- (void)menuRefresh;

@end

@interface RAFoldersList : RAMenuBase
- (id) initWithFilePath:(NSString*)path;
@end

typedef struct
{
   char orientations[32];
   unsigned orientation_flags;
   char bluetooth_mode[64];
} apple_frontend_settings_t;
extern apple_frontend_settings_t apple_frontend_settings;

@interface CocoaView : UIViewController<CLLocationManagerDelegate, AVCaptureAudioDataOutputSampleBufferDelegate>
+ (CocoaView*)get;
@end

@interface RetroArch_iOS : UINavigationController<UIApplicationDelegate, UINavigationControllerDelegate>

@property (nonatomic) UIWindow* window;
@property (nonatomic) NSString* documentsDirectory;
@property (nonatomic) RAMenuBase* mainmenu;
                      
+ (RetroArch_iOS*)get;

- (void)showGameView;
- (void)toggleUI;

- (void)refreshSystemConfig;
- (void)mainMenuRefresh;
@end

void get_ios_version(int *major, int *minor);

#elif defined(HAVE_COCOA)
#include <AppKit/AppKit.h>
#ifdef HAVE_CORELOCATION
#include <CoreLocation/CoreLocation.h>
#endif


@interface CocoaView : NSView
#ifdef HAVE_CORELOCATION
<CLLocationManagerDelegate>
#endif

+ (CocoaView*)get;
#if !defined(HAVE_COCOA)
- (void)display;
#endif

@end

@interface RetroArch_OSX : NSObject
{
   NSWindow* _window;
   NSWindowController* _settingsWindow;
}

@property (nonatomic, retain) NSWindow IBOutlet* window;

@end

#endif

extern void apple_display_alert(const char *message, const char *title);



#define BOXSTRING(x) [NSString stringWithUTF8String:x]
#define BOXINT(x)    [NSNumber numberWithInt:x]
#define BOXUINT(x)   [NSNumber numberWithUnsignedInt:x]
#define BOXFLOAT(x)  [NSNumber numberWithDouble:x]

#endif
