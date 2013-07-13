//
//  ViewController.m
//  earthnews
//
//  Created by Ubaka Onyechi on 01/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import "ViewController.h"
#import "AppDelegate.h"
#import "../app.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG_TOUCHES 0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface ViewController ()
{
  bool _isOrientationLandscape;
  UIInterfaceOrientation _toOrientation;
}
@property (strong, nonatomic) EAGLContext *context;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation ViewController

@synthesize context = _context;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) dealloc
{
  [_context release];
  [super dealloc];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) viewDidLoad
{
  [super viewDidLoad];
  
  _isOrientationLandscape = false;
  _toOrientation = UIInterfaceOrientationPortrait;
  
  self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];
  
  if (!self.context)
  {
    CX_LOG_CONSOLE (1, "Failed ot create ES context");
  }

  GLKView *view = (GLKView *) self.view;
  
  view.context = self.context;
  view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat16;
  view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
  view.drawableMultisample = GLKViewDrawableMultisample4X;
  
  UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
                                            initWithTarget:self action:@selector(handlePinchGesture:)];
  [self.view addGestureRecognizer:pinchGesture];
  [pinchGesture release];

  [self setPreferredFramesPerSecond:60];
  
  [EAGLContext setCurrentContext:self.context];
  
  /*
  NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
  NSString *appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
  NSString *appVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
  */
  
  //[[UINavigationBar appearanceWhenContainedIn:[UINavigationController class], [UIPopoverController class], nil] setTintColor:[UIColor colorWithWhite:0.0f alpha:0.5f]];
  
  //[[UINavigationBar appearanceWhenContainedIn:[UIViewController class], nil] setTintColor:[UIColor colorWithWhite:0.0f alpha:0.5f]];
  
  //[[UIBarButtonItem appearanceWhenContainedIn:[UINavigationBar class], nil] setTintColor:[UIColor colorWithWhite:0.1f alpha:1.0f]];
  
  //[[UINavigationBar appearance] setBarStyle:UIBarStyleBlack];
  
  //[[UINavigationBar appearance] setTranslucent:YES];
  
  //[[UINavigationBar appearance] setBarStyle:UIBarStyleBlackTranslucent];
  
  //[[UITableViewCell appearanceWhenContainedIn:[UITableView class], [UIViewController class], [UIPopoverController class], nil] setBackgroundColor:[UIColor colorWithWhite:0.0f alpha:0.4f]];
  
  //[[UITableViewCell appearanceWhenContainedIn:[UIPopoverController class], nil] setBackgroundView:nil];
  
  //[[UITableViewCell appearanceWhenContainedIn:[UIPopoverController class], nil] setAlpha:0.3f];
  
  /*
  UIImage *navBarImage = [[UIImage imageNamed:@"data/images/ui/system/navbar.png"]
                          resizableImageWithCapInsets:UIEdgeInsetsMake(0, 0, 0, 0)];
  
  [[UINavigationBar appearance] setBackgroundImage:navBarImage forBarMetrics:UIBarMetricsDefault];
  */
  
  //[[UIBarButtonItem appearance] setTintColor:[UIColor blackColor]];
  
  //[[UINavigationBar appearance] setBarStyle:UIBarStyleBlackTranslucent];
  
  //[[UINavigationBar appearance] setTintColor:[UIColor redColor]];
  
  //[[UINavigationBar appearance] setTranslucent:YES];
  
  //[[UINavigationBar appearance] setAlpha:0.7f];
  
  
#if 0
  AppDelegate* appDelegate = (((AppDelegate*) [UIApplication sharedApplication].delegate));
  UIViewController *rootViewController = [appDelegate.window rootViewController];
  
  UIWindow *appWindow = [[UIApplication sharedApplication] keyWindow];
  UIViewController *rvc = appWindow.rootViewController;
  (void) rvc;
#endif
  
  [[NSNotificationCenter defaultCenter]
   addObserver:self
   selector:@selector(deviceOrientationDidChangeNotification:)
   name:UIDeviceOrientationDidChangeNotification
   object:nil];
  
  int w = (int) self.view.bounds.size.width;
  int h = (int) self.view.bounds.size.height;
  
  app_init (self, self.context, w, h);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) viewDidUnload
{
  [super viewDidUnload];
  
  [EAGLContext setCurrentContext:self.context];
  
  // application deinitialise
  app_deinit ();
  
  if ([EAGLContext currentContext] == self.context)
  {
    [EAGLContext setCurrentContext:nil];
  }
  
  self.context = nil;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
  if ((toInterfaceOrientation == UIInterfaceOrientationPortrait) ||
      (toInterfaceOrientation == UIInterfaceOrientationPortraitUpsideDown))
  {
    return NO;
  }
  
  return YES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)deviceOrientationDidChangeNotification:(NSNotification*)notification
{
  UIDeviceOrientation deviceOrientation = [[UIDevice currentDevice] orientation];
  
  if (!_isOrientationLandscape && UIDeviceOrientationIsLandscape (deviceOrientation))
  {
    [EAGLContext setCurrentContext:self.context];
  
    int w = (int) self.view.bounds.size.width;
    int h = (int) self.view.bounds.size.height;
  
    app_view_resize (w, h);
    
    _isOrientationLandscape = true;
  }
}

- (void) willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
  _toOrientation = toInterfaceOrientation;
}

- (void) didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
  if (!_isOrientationLandscape && UIInterfaceOrientationIsLandscape (_toOrientation))
  {
    [EAGLContext setCurrentContext:self.context];
    
    int w = (int) self.view.bounds.size.width;
    int h = (int) self.view.bounds.size.height;
    
    app_view_resize (w, h);
    
    _isOrientationLandscape = true;
  }
}

- (NSInteger) preferredInterfaceOrientationForPresentation
{
  /*
  UIDeviceOrientation deviceOrientation = [[UIDevice currentDevice] orientation];
  
  if (deviceOrientation == UIDeviceOrientationLandscapeLeft)
  {
    return UIInterfaceOrientationLandscapeLeft;
  }
  else
  {
    return UIInterfaceOrientationLandscapeRight;
  }
  */
  
  return [[UIApplication sharedApplication] statusBarOrientation];
}

- (NSInteger) supportedInterfaceOrientations
{
  return UIInterfaceOrientationMaskLandscape;
}

- (BOOL) shouldAutorotate
{
  return YES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) update
{
  app_update ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  app_render ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES && 1, "touchesBegan [%d]", [[event allTouches] count]);
  
  if ([[event allTouches] count] > 1)
  {
    return;
  }
  
  UITouch *touch = [[[event allTouches] allObjects] objectAtIndex:0];
  CGPoint currTouchPoint = [touch locationInView:self.view];
  CGPoint prevTouchPoint = [touch previousLocationInView:self.view];
  
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES, "touch: x = %.1f, y = %.1f", currTouchPoint.x, currTouchPoint.y);

  float screen_width = self.view.bounds.size.width;
  float screen_height = self.view.bounds.size.height;
  
  float normalised_x = currTouchPoint.x / screen_width;
  float normalised_y = currTouchPoint.y / screen_height;
  float normalised_prev_x = prevTouchPoint.x / screen_width;
  float normalised_prev_y = prevTouchPoint.y / screen_height;
  
  _input_cache_touch_event (INPUT_TOUCH_TYPE_BEGIN, normalised_x, normalised_y, normalised_prev_x, normalised_prev_y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES && 1, "touchesEnded [%d]", [[event allTouches] count]);
  
  if ([[event allTouches] count] > 1)
  {
    return;
  }
  
  UITouch *touch = [[[event allTouches] allObjects] objectAtIndex:0];
  CGPoint currTouchPoint = [touch locationInView:self.view];
  CGPoint prevTouchPoint = [touch previousLocationInView:self.view];
  
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES, "touch: x = %.1f, y = %.1f", currTouchPoint.x, currTouchPoint.y);
  
  float screen_width = self.view.bounds.size.width;
  float screen_height = self.view.bounds.size.height;
  
  float normalised_x = currTouchPoint.x / screen_width;
  float normalised_y = currTouchPoint.y / screen_height;
  float normalised_prev_x = prevTouchPoint.x / screen_width;
  float normalised_prev_y = prevTouchPoint.y / screen_height;
  
  _input_cache_touch_event (INPUT_TOUCH_TYPE_END, normalised_x, normalised_y, normalised_prev_x, normalised_prev_y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES && 1, "touchesMoved [%d]", [[event allTouches] count]);
  
  if ([[event allTouches] count] > 1)
  {
    return;
  }
  
  UITouch *touch = [[[event allTouches] allObjects] objectAtIndex:0];
  CGPoint currTouchPoint = [touch locationInView:self.view];
  CGPoint prevTouchPoint = [touch previousLocationInView:self.view];
  
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES, " prev: x = %.1f, y = %.1f", prevTouchPoint.x, prevTouchPoint.y);
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES, " curr: x = %.1f, y = %.1f", currTouchPoint.x, currTouchPoint.y);
  
  float screen_width = self.view.bounds.size.width;
  float screen_height = self.view.bounds.size.height;
  
  float normalised_x = currTouchPoint.x / screen_width;
  float normalised_y = currTouchPoint.y / screen_height;
  float normalised_prev_x = prevTouchPoint.x / screen_width;
  float normalised_prev_y = prevTouchPoint.y / screen_height;
  
  _input_cache_touch_event (INPUT_TOUCH_TYPE_MOVE, normalised_x, normalised_y, normalised_prev_x, normalised_prev_y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES && 1, "touchesCancelled [%d]", [[event allTouches] count]);
  
  UITouch *touch = [[[event allTouches] allObjects] objectAtIndex:0];
  CGPoint currTouchPoint = [touch locationInView:self.view];
  CGPoint prevTouchPoint = [touch previousLocationInView:self.view];

  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES, "touch: x = %.1f, y = %.1f", currTouchPoint.x, currTouchPoint.y);
  
  float screen_width = self.view.bounds.size.width;
  float screen_height = self.view.bounds.size.height;
  
  float normalised_x = currTouchPoint.x / screen_width;
  float normalised_y = currTouchPoint.y / screen_height;
  float normalised_prev_x = prevTouchPoint.x / screen_width;
  float normalised_prev_y = prevTouchPoint.y / screen_height;
  
  _input_cache_touch_event (INPUT_TOUCH_TYPE_END, normalised_x, normalised_y, normalised_prev_x, normalised_prev_y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)handlePinchGesture:(UIGestureRecognizer *)sender 
{
  CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
  
  CX_LOG_CONSOLE (DEBUG_LOG_TOUCHES && 0, "Pinch Gesture: factor [%.2f]", factor);
  //self.view.transform = CGAffineTransformMakeScale (factor, factor);
  
  _input_cache_gesture_event (INPUT_GESTURE_TYPE_PINCH, (void *) &factor);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
