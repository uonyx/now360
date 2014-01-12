//
//  AppDelegate.m
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#import "AppDelegate.h"
#import "../app.h"

@implementation AppDelegate

@synthesize window = _window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  //self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  // Override point for customization after application launch.
  //self.window.backgroundColor = [UIColor whiteColor];
  //[self.window makeKeyAndVisible];

  [[UIBarButtonItem appearance] setTintColor:[UIColor blackColor]];
  [[UINavigationBar appearance] setTintColor:[UIColor blackColor]];
  [[UINavigationBar appearanceWhenContainedIn:[UIViewController class], [UIPopoverController class], nil] setTintColor:[UIColor blackColor]];
  
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
  app_on_background ();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
  app_on_foreground ();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
  app_on_terminate ();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
  // release any cached data, images, etc that aren't in use
  
  app_on_memory_warning ();
}

- (void)application:(UIApplication *)application didChangeStatusBarOrientation:(UIInterfaceOrientation)oldStatusBarOrientation
{
}

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
{
  //return UIInterfaceOrientationMaskLandscape;
  return UIInterfaceOrientationMaskAll;
}

@end
