//
//  webview.m
//  earthnews
//
//  Created by Ubaka Onyechi on 08/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import "browser.h"
#import "render.h"
#import "../engine/cx_engine.h"

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BROWSER_DEBUG_LOG_ENABLED   1
#define BROWSER_CACHE_ENABLED       1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface WebViewDelegate : NSObject <UIWebViewDelegate>
{
}
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

NSMutableString *g_browserTitle = nil;

@implementation WebViewDelegate

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
  switch (navigationType)
  {
    case UIWebViewNavigationTypeLinkClicked:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeLinkClicked)");
      break; 
    }
    case UIWebViewNavigationTypeFormSubmitted:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeFormSubmitted)");
      break; 
    }
    case UIWebViewNavigationTypeBackForward:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeBackForward)");
      break; 
    }
    case UIWebViewNavigationTypeReload:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeReload)");
      break; 
    }
    case UIWebViewNavigationTypeFormResubmitted:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeFormResubmitted)");
      break; 
    }
    case UIWebViewNavigationTypeOther:  
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeOther)");
      break; 
    }
    default: 
    { 
      CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: navigationType: Unknown!)");
      break; 
    }
  }
  
#if BROWSER_DEBUG_LOG_ENABLED
  NSString *hostname = [[request URL] host];
  NSLog (@"Webview: Hostname: %@", hostname);
  NSLog (@"Webview: request: %@", request);
#endif
  
#if BROWSER_CACHE_ENABLED
  if ([request isKindOfClass:[NSMutableURLRequest class]])
  {
    NSMutableURLRequest *mutableRequest = (NSMutableURLRequest *) request;
    [mutableRequest setCachePolicy:NSURLRequestReturnCacheDataElseLoad];
  }
#endif
  
  return YES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webViewDidStartLoad:(UIWebView *)webView
{
  CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: Starting load");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webViewDidFinishLoad:(UIWebView *)webView
{
  NSString *docTitle = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
  
  if (docTitle.length > 0)
  {
    [g_browserTitle setString:docTitle];
  }
  
  CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "Webview: Finishing load");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
#if BROWSER_DEBUG_LOG_ENABLED
  NSLog (@"Webview: Error: [%d] %@", [error code], [error description]);
#endif
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  BROWSER_BUTTON_ID_INVALID = -1,
  BROWSER_BUTTON_ID_REFRESH,
  BROWSER_BUTTON_ID_FORWARD,
  BROWSER_BUTTON_ID_BACK,
  NUM_BROWSER_BUTTON_IDS
} browser_button_id;

typedef enum
{
  BROWSER_BUTTON_DRAW_STYLE_NONE,
  BROWSER_BUTTON_DRAW_STYLE_FLIP_HORIZONTAL,
  BROWSER_BUTTON_DRAW_STYLE_FLIP_VERTICAL,
} browser_button_draw_style;

typedef struct browser_button_t
{
  cx_texture *image, *altImage;
  cx_colour colour, altColour;
  browser_button_draw_style drawStyle;
  bool alt;
} browser_button_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool               s_animating = false;
static bool               s_touch [NUM_BROWSER_BUTTON_IDS];
static browser_button_t   s_buttons [NUM_BROWSER_BUTTON_IDS];
static const float        s_toolbarHeight = 32.0f; 
static const float        s_toolbarMargin = 16.0f;
static const float        s_buttonSpacing = 12.0f;
static UIWebView         *s_webview = nil;
static WebViewDelegate   *s_webviewDelegate = nil;
static UIViewController  *s_rootViewController = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_set_hidden (bool hidden);
bool browser_get_hidden (void);
void browser_clear_cache (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_init (void *container)
{  
  CX_ASSERT (container);
  
  s_rootViewController = (UIViewController *) container;
  
  s_webview = nil;
  s_webviewDelegate = [[WebViewDelegate alloc] init];
  
  //cx_texture *dirButton = cx_texture_create_from_file ("data/browser_icons/icon066.png");
  //cx_texture *refreshButton = cx_texture_create_from_file ("data/browser_icons/icon072.png");

  cx_texture *dirButton = cx_texture_create_from_file ("data/browser_icons/bicon_arrow_16.png");
  cx_texture *refreshButton = cx_texture_create_from_file ("data/browser_icons/bicon_refr_16.png");
  cx_texture *stopButton = cx_texture_create_from_file("data/browser_icons/bicon_x_16.png");
  
  CX_ASSERT (dirButton);
  CX_ASSERT (refreshButton);
  
  memset (s_touch, false, sizeof (s_touch));
  memset (s_buttons, 0, sizeof (s_buttons));
  
  cx_colour colour1, colour2;
  
  cx_colour_set (&colour1, 0.3f, 0.3f, 0.3f, 1.0f);
  cx_colour_set (&colour2, 0.6f, 0.6f, 0.6f, 1.0f);
  
  s_buttons [BROWSER_BUTTON_ID_BACK].image = dirButton;
  s_buttons [BROWSER_BUTTON_ID_BACK].altImage = dirButton;
  s_buttons [BROWSER_BUTTON_ID_BACK].colour = colour1;
  s_buttons [BROWSER_BUTTON_ID_BACK].altColour = colour2;
  s_buttons [BROWSER_BUTTON_ID_BACK].drawStyle = BROWSER_BUTTON_DRAW_STYLE_FLIP_HORIZONTAL;
  
  s_buttons [BROWSER_BUTTON_ID_FORWARD].image = dirButton;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].altImage = dirButton;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].colour = colour1;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].altColour = colour2;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].drawStyle = BROWSER_BUTTON_DRAW_STYLE_NONE;
  
  s_buttons [BROWSER_BUTTON_ID_REFRESH].image = refreshButton;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].altImage = stopButton;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].colour = colour1;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].altColour = colour1;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].drawStyle = BROWSER_BUTTON_DRAW_STYLE_NONE;
  
#if BROWSER_CACHE_ENABLED
#if 0
  const unsigned int cacheMemory  = 1024 * 1024 * 2;
  const unsigned int cacheStorage = 1024 * 1024 * 8;
  NSString *cacheLocation = @"browser";
  
  NSURLCache *sharedCache = [[NSURLCache alloc] initWithMemoryCapacity:cacheMemory 
                                                          diskCapacity:cacheStorage 
                                                              diskPath:cacheLocation];
  [NSURLCache setSharedURLCache:sharedCache];
#else
  const unsigned int cacheMemory = [[NSURLCache sharedURLCache] memoryCapacity];
  const unsigned int cacheStorage = [[NSURLCache sharedURLCache] diskCapacity];
  
  CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "cacheMemory: %d", cacheMemory);
  CX_DEBUGLOG_CONSOLE (BROWSER_DEBUG_LOG_ENABLED, "cacheStorage: %d", cacheStorage);
  
  CX_REFERENCE_UNUSED_VARIABLE (cacheMemory);
  CX_REFERENCE_UNUSED_VARIABLE (cacheStorage);
  
  const unsigned int newCacheMemory = MAX (cacheMemory, (1024 * 1024 * 2));
  [[NSURLCache sharedURLCache] setMemoryCapacity:newCacheMemory];
#endif
  browser_clear_cache ();
#endif
  
  g_browserTitle = [[NSMutableString alloc] init];
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_deinit (void)
{
  [g_browserTitle release];
  [s_webview release];
  [s_webviewDelegate release];
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_clear_cache (void)
{
  [[NSURLCache sharedURLCache] removeAllCachedResponses];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_open (const char *url)
{
  CX_ASSERT (url);
  
  bool success = false;

  if (browser_get_hidden ())
  {
    ///////////////////////
    // create view
    ///////////////////////
    
    if (s_webview)
    {
      [s_webview removeFromSuperview];
      [s_webview release];
      s_webview = nil;
    }
    
    s_webview = [[UIWebView alloc] init]; 
    
    CX_ASSERT (s_webview);
    
    [s_webview setDelegate:s_webviewDelegate];
    [s_webview setHidden:YES];
    [s_webview setAlpha:0.0f];
    [s_webview setScalesPageToFit:YES];
    [s_webview setClearsContextBeforeDrawing:YES];
    [s_webview setBackgroundColor:[UIColor whiteColor]];
    
    [[s_rootViewController view] addSubview:s_webview];
    
    ///////////////////////
    // launch view
    ///////////////////////
    
    NSString *nsurlStirng = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    NSURL *nsurl = [NSURL URLWithString:nsurlStirng];
    
#if BROWSER_CACHE_ENABLED
    const int requestTimeoutSecs = 30;
    NSURLRequest *request = [NSURLRequest requestWithURL:nsurl 
                                             cachePolicy:NSURLRequestReturnCacheDataElseLoad 
                                         timeoutInterval:requestTimeoutSecs];
#else
    NSURLRequest *request = [NSURLRequest requestWithURL:nsurl];
#endif
    
    [s_webview loadRequest:request];
    
    ///////////////////////
    // begin animation
    ///////////////////////
    
    browser_set_hidden (false);
    
    [g_browserTitle setString:nsurlStirng];
    
    success = true;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_close (void)
{
  bool success = false;
  
  if (!browser_get_hidden ())
  {
    browser_set_hidden (true);
    
    success = true;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_is_open (void)
{
  return !browser_get_hidden ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_set_hidden (bool hidden)
{
  if (!s_animating)
  {
    // NB: block-based animations only available in iOS 4 and later
    [UIWebView animateWithDuration:0.5f 
                             delay:0.0f 
                           options:UIViewAnimationOptionAllowAnimatedContent|UIViewAnimationCurveEaseInOut|UIViewAnimationOptionTransitionNone
                        animations:^{ 
                                      s_animating = true; 
                          
                                      if (hidden) 
                                      {
                                        [s_webview stopLoading];
                                        [s_webview setAlpha:0.0f];
                                      } 
                                      else 
                                      { 
                                        [s_webview setAlpha:1.0f];
                                        [s_webview setHidden:NO];
                                      }; 
                        } 
                        completion:^(BOOL finished) { 
                                      if (finished && hidden) 
                                      { 
                                        [s_webview setHidden:YES];
                                      } 
                          
                                      s_animating = false; 
                        }
     ];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_get_hidden (void)
{
  bool hidden;
  
  if (s_animating)
  {
    hidden = true;
  }
  if (s_webview)
  {
    hidden = s_webview.hidden;
  }
  else
  {
    hidden = true;
  }
  
  if (s_webview)
  {
    hidden = s_webview.hidden && !s_animating;
  }
  else
  {
    
  }
  
  
  return hidden;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_render (const browser_def_t *browserDef, float opacity)
{
  CX_ASSERT (browserDef);
  
  //if (s_webview && !s_webview.isHidden)
  if (s_webview)
  {
    float toolbarHeight = s_toolbarHeight; 
    float toolbarMargin = s_toolbarMargin;
    float viewPosX      = browserDef->posX;
    float viewPosY      = browserDef->posY + toolbarHeight;
    float viewWidth     = browserDef->width;
    float viewHeight    = browserDef->height - toolbarHeight;
    float alpha         = opacity * [[[s_webview layer] presentationLayer] opacity];
  
    ///////////////////////
    // backdrop
    ///////////////////////
    
    float bdx1 = 0.0f;
    float bdy1 = 0.0f;
    float bdx2 = bdx1 + cx_gdi_get_screen_width ();
    float bdy2 = bdy1 + cx_gdi_get_screen_height ();
    
    cx_colour bdcolour;
    cx_colour_set (&bdcolour, 0.0f, 0.0f, 0.0f, alpha * 0.6f);
    
    cx_draw_quad (bdx1, bdy1, bdx2, bdy2, 0.0f, 0.0f, &bdcolour, NULL);
    
    ///////////////////////
    // toolbar
    ///////////////////////
    
    float tbx1 = browserDef->posX;
    float tby1 = browserDef->posY;
    float tbx2 = tbx1 + browserDef->width;
    float tby2 = tby1 + toolbarHeight;
    
    cx_colour tbcolour;
    //cx_colour_set (&tbcolour, 0.3f, 0.3f, 0.3f, alpha); // grey
    cx_colour_set (&tbcolour, 0.7f, 0.7f, 0.7f, alpha); // grey
    
    cx_draw_quad (tbx1, tby1, tbx2, tby2, 0.0f, 0.0f, &tbcolour, NULL);
    
    //////////////////////////
    // loading icon && title
    //////////////////////////
    
    const cx_font *font = render_get_ui_font (UI_FONT_SIZE_12);
    
    if (g_browserTitle)
    {
      //const char *t = [g_currentTitle cStringUsingEncoding:NSASCIIStringEncoding];
      const char *t = [g_browserTitle cStringUsingEncoding:NSUTF8StringEncoding];
      
      if (t)
      {
        float tx = tbx1 + toolbarMargin;
        float ty = tby1 + (toolbarHeight * 0.5f);
        
        cx_colour colour;
        cx_colour_set (&colour, 0.2f, 0.2f, 0.2f, alpha);
        
        cx_font_render (font, t, tx, ty, 0.0f, CX_FONT_ALIGNMENT_CENTRE_Y, &colour);
      }
    }
    
    ///////////////////////
    // buttons
    ///////////////////////
    
    s_buttons [BROWSER_BUTTON_ID_BACK].alt    = !s_webview.canGoBack;
    s_buttons [BROWSER_BUTTON_ID_FORWARD].alt = !s_webview.canGoForward;
    s_buttons [BROWSER_BUTTON_ID_REFRESH].alt = s_webview.isLoading;

    float tbEndX = tbx2 - toolbarMargin;
    
    for (int i = 0, c = NUM_BROWSER_BUTTON_IDS; i < c; ++i)
    {

      cx_texture *buttonImage = NULL;
      cx_colour *buttonColour = NULL;
    
      bool buttonHeld    = s_touch [i];
      bool buttonEnabled = !s_buttons [i].alt;
      browser_button_draw_style drawStyle = s_buttons [i].drawStyle;
      
      if (buttonEnabled)
      {
        buttonImage = s_buttons [i].image;
        buttonColour = &s_buttons [i].colour;
      }
      else
      {
        buttonImage = s_buttons [i].altImage;
        buttonColour = &s_buttons [i].altColour;
      }
      
      buttonColour->a = alpha;
      
      float buttonWidth = (float) buttonImage->width;
      float buttonHeight = (float) buttonImage->height;
      
      float bx1 = tbEndX - buttonWidth;
      float by1 = tby1 + ((toolbarHeight - buttonHeight) * 0.5f);
      float bx2 = bx1 + buttonWidth;
      float by2 = by1 + buttonHeight;
      
      float u1, v1, u2, v2;
      
      switch (drawStyle) 
      {
        case BROWSER_BUTTON_DRAW_STYLE_FLIP_HORIZONTAL:
        {
          u1 = 1.0f;
          v1 = 0.0f;
          u2 = 0.0f;
          v2 = 1.0f;
          break;
        }
          
        case BROWSER_BUTTON_DRAW_STYLE_FLIP_VERTICAL:
        {
          u1 = 0.0f;
          v1 = 1.0f;
          u2 = 1.0f;
          v2 = 0.0f;
          break;
        }
          
        default:
        {
          u1 = 0.0f;
          v1 = 0.0f;
          u2 = 1.0f;
          v2 = 1.0f;
          break;
        }
      }
    
      if (buttonHeld)
      {
        cx_colour touchColour;
        cx_colour_set (&touchColour, 0.2f, 0.8f, 0.2f, alpha);
        
        float bhx1 = tbEndX - buttonWidth - s_buttonSpacing;
        float bhy1 = tby1;
        float bhx2 = bhx1 + buttonWidth + (s_buttonSpacing + s_buttonSpacing);
        float bhy2 = bhy1 + toolbarHeight;
        
        cx_draw_quad (bhx1, bhy1, bhx2, bhy2, 0.0f, 0.0f, &touchColour, NULL);
      }
      
      cx_draw_quad_uv (bx1, by1, bx2, by2, 0.0f, 0.0f, u1, v1, u2, v2, buttonColour, buttonImage);
      
      tbEndX = bx1 - s_buttonSpacing;
    }
    
    CGRect frame = CGRectMake (viewPosX, viewPosY, viewWidth, viewHeight);
    [s_webview setFrame:frame];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_handle_input (const browser_def_t *browserDef, browser_input input, float touchX, float touchY)
{
  CX_ASSERT (browserDef);
  
  bool handled = false;
  
  if (!browser_get_hidden ())
  {
    float toolbarHeight = s_toolbarHeight; 
    float toolbarMargin = s_toolbarMargin;
    
    float tbx1 = browserDef->posX;
    float tby1 = browserDef->posY;
    float tbx2 = tbx1 + browserDef->width;
    float tby2 = tby1 + toolbarHeight;
    
    if ((touchX >= tbx1) && (touchX <= tbx2))
    {
      if ((touchY >= tby1) && (touchY <= tby2))
      {
        handled = true;
      }
    }    
    
    memset (s_touch, false, sizeof (s_touch));
    
    float tbEndX = tbx2 - toolbarMargin;
    
    for (int i = 0, c = NUM_BROWSER_BUTTON_IDS; i < c; ++i)
    {
      cx_texture *button = s_buttons [i].image;
      
      float buttonWidth = (float) button->width;
      //float buttonHeight = (float) button->height;
      
      float bx1 = tbEndX - buttonWidth;
      float by1 = tby1; // + ((toolbarHeight - buttonHeight) * 0.5f);
      float bx2 = bx1 + buttonWidth;
      float by2 = tby2; //by1 + buttonHeight;
      
      if ((touchX >= bx1) && (touchX <= bx2))
      {
        if ((touchY >= by1) && (touchY <= by2))
        {
          s_touch [i] = true;
        }
      }
      
      tbEndX = bx1 - s_buttonSpacing;
    }
    
    if (input == BROWSER_INPUT_TOUCH_END)
    {      
      bool goBack = s_touch [BROWSER_BUTTON_ID_BACK];
      bool goForward = s_touch [BROWSER_BUTTON_ID_FORWARD];
      bool refresh = s_touch [BROWSER_BUTTON_ID_REFRESH];
      
      if (goBack)
      {
        [s_webview goBack];
        handled = true;
      }
      else if (goForward)
      {
        [s_webview goForward];
        handled = true;
      }
      else if (refresh)
      {
        if ([s_webview isLoading])
        {
          [s_webview stopLoading];
        }
        else
        {
          [s_webview reload];
        }
        
        handled = true;
      }
      
      memset (s_touch, false, sizeof (s_touch));
    }
  }
  
  return handled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

