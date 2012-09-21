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

@interface WebViewDelegate : NSObject <UIWebViewDelegate>
{
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

NSMutableString *g_currentTitle = nil;

bool g_loading = false;

@implementation WebViewDelegate

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
  switch (navigationType)
  {
    case UIWebViewNavigationTypeLinkClicked:  { 
      //NSLog (@"UIWebViewNavigationTypeLinkClicked"); 
      g_loading = true; 
      break; 
    }
    case UIWebViewNavigationTypeFormSubmitted:  { 
      //NSLog (@"UIWebViewNavigationTypeFormSubmitted"); 
      break; }
    case UIWebViewNavigationTypeBackForward:  { 
      //NSLog (@"UIWebViewNavigationTypeBackForward"); 
      break; }
    case UIWebViewNavigationTypeReload:  { 
      //NSLog (@"UIWebViewNavigationTypeReload"); 
      break; }
    case UIWebViewNavigationTypeFormResubmitted:  { 
      //NSLog (@"UIWebViewNavigationTypeFormResubmitted"); 
      break; }
    case UIWebViewNavigationTypeOther:  { 
      //NSLog (@"UIWebViewNavigationTypeOther"); 
      break; }
    default: { break; }
  }
  
  NSString *hostname = [[request URL] host];
  
  NSLog (@"Webview: Hostname: %@", hostname);
  
  if (g_loading)
  {
    NSLog (@"Webview: shouldStartLoadWithRequest: %@", request);
    
    [webView clearsContextBeforeDrawing];
  }
  
  return YES;
}

- (void) webViewDidStartLoad:(UIWebView *)webView
{
  if (g_loading)
  {
    NSLog (@"Webview: Starting load");
  }
}

- (void) webViewDidFinishLoad:(UIWebView *)webView
{
  NSString *docTitle = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
  
  if (docTitle.length > 0)
  {
    [g_currentTitle setString:docTitle];
  }
  
  if (g_loading)
  {
    NSLog (@"Webview: Finishing load");
    //g_loading = false;
  }
}

- (void) webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
  NSLog (@"Webview: Error: [%d] %@", [error code], [error description]);
  //NSLog (@"%@", error);
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
cx_texture *image;
browser_button_draw_style drawStyle;
} browser_button_t;

static bool               s_animating = false;
static bool               s_touch [NUM_BROWSER_BUTTON_IDS];
static browser_button_t   s_buttons [NUM_BROWSER_BUTTON_IDS];
static const float        s_toolbarHeight = 32.0f; 
static const float        s_toolbarMargin = 16.0f;
static const float        s_buttonSpacing = 12.0f;
static UIWebView         *s_webview = nil;
static WebViewDelegate   *s_webviewDelegate = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_init (void *container)
{  
  CX_ASSERT (container);
  
  g_currentTitle = [[NSMutableString alloc] init];
  
  UIViewController *rootViewController = (UIViewController *) container;
  
  s_webview = [[UIWebView alloc] init];
  s_webviewDelegate = [[WebViewDelegate alloc] init];
  
  [s_webview setDelegate:s_webviewDelegate];
  [s_webview setScalesPageToFit:YES];
  [s_webview setHidden:YES];
  [s_webview setAlpha:0.0f];
  [s_webview setClearsContextBeforeDrawing:YES];
  [s_webview setBackgroundColor:[UIColor whiteColor]];
  
  [[rootViewController view] addSubview:s_webview];

  cx_texture *dirButton = cx_texture_create_from_file ("data/browser_icons/icon066.png");
  cx_texture *refreshButton = cx_texture_create_from_file ("data/browser_icons/icon072.png");
  
  CX_ASSERT (dirButton);
  CX_ASSERT (refreshButton);
  
  s_buttons [BROWSER_BUTTON_ID_BACK].image = dirButton;
  s_buttons [BROWSER_BUTTON_ID_BACK].drawStyle = BROWSER_BUTTON_DRAW_STYLE_FLIP_HORIZONTAL;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].image = dirButton;
  s_buttons [BROWSER_BUTTON_ID_FORWARD].drawStyle = BROWSER_BUTTON_DRAW_STYLE_NONE;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].image = refreshButton;
  s_buttons [BROWSER_BUTTON_ID_REFRESH].drawStyle = BROWSER_BUTTON_DRAW_STYLE_NONE;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_deinit (void)
{
#if 0
  // remove all cached responses
  [[NSURLCache sharedURLCache] removeAllCachedResponses];
  
  // set an empty cache
  NSURLCache *sharedCache = [[NSURLCache alloc] initWithMemoryCapacity:0 diskCapacity:0 diskPath:nil];
  [NSURLCache setSharedURLCache:sharedCache];
  
  // remove the cache for a particular request
  //[[NSURLCache sharedURLCache] removeCachedResponseForRequest:request];
#endif
  
  [s_webview release];
  [s_webviewDelegate release];
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_launch_url (const char *url)
{
  CX_ASSERT (url);
  CX_ASSERT (s_webview);

  NSString *nsurl = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
  
  [s_webview stopLoading];
  
  [s_webview loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"about:blank"]]];
  
  [s_webview loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:nsurl]]];

  [g_currentTitle setString:nsurl];
  
  browser_hide (false);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_hide (bool hide)
{
#if 0
  g_uiwebview.hidden = hide;
#else
  if (!s_animating)
  {
    [UIWebView animateWithDuration:0.35f 
                             delay:0.0f 
                           options:UIViewAnimationOptionAllowAnimatedContent|UIViewAnimationCurveEaseInOut|UIViewAnimationOptionTransitionNone
                        animations:^{ 
                                      s_animating = true; 
                                      if (hide) 
                                      { 
                                        s_webview.alpha = 0.0f;
                                      } 
                                      else 
                                      { 
                                        s_webview.alpha = 1.0f; 
                                        s_webview.hidden = NO; 
                                      }; 
                        } 
                        completion:^(BOOL finished) { 
                                      if (finished && hide) 
                                      { 
                                        s_webview.hidden = YES; 
                                      } 
                                      s_animating = false; 
                        }
     ];
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void browser_render (const browser_def_t *browserDef, float opacity)
{
  CX_ASSERT (browserDef);
  
  if (!s_webview.isHidden)
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
    
    cx_draw_quad (bdx1, bdy1, bdx2, bdy2, 0.0f, &bdcolour, NULL);
    
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
    
    cx_draw_quad (tbx1, tby1, tbx2, tby2, 0.0f, &tbcolour, NULL);
    
    //////////////////////////
    // loading icon && title
    //////////////////////////
    
    const cx_font *font = render_get_ui_font (UI_FONT_SIZE_12);
    
    if (g_currentTitle)
    {
      //const char *t = [g_currentTitle cStringUsingEncoding:NSASCIIStringEncoding];
      const char *t = [g_currentTitle cStringUsingEncoding:NSUTF8StringEncoding];
      
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
    
    cx_colour buttonColour;
    //cx_colour_set (&buttonColour, 0.90f, 0.90f, 0.98f, alpha); // lavender
    cx_colour_set (&buttonColour, 1.0f, 1.0f, 1.0f, alpha);
    
    float tbEndX = tbx2 - toolbarMargin;
    
    for (int i = 0, c = NUM_BROWSER_BUTTON_IDS; i < c; ++i)
    {
      cx_texture *button = s_buttons [i].image;
      browser_button_draw_style drawStyle = s_buttons [i].drawStyle;
      
      float buttonWidth = (float) button->width;
      float buttonHeight = (float) button->height;
      
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
    
      cx_draw_quad2 (bx1, by1, bx2, by2, 0.0f, u1, v1, u2, v2, &buttonColour, button);
      
      tbEndX = bx1 - s_buttonSpacing;
    }
     
    // back button
    if (s_webview.canGoBack)
    {
    }
    else
    {
    }
    
    // forward button
    if (s_webview.canGoForward)
    {
    }
    else
    {
    }
    
    // stop/cancel button
    if (s_webview.isLoading)
    {
    }
    else 
    {
    }
    
    CGRect frame = CGRectMake (viewPosX, viewPosY, viewWidth, viewHeight);
    [s_webview setFrame:frame];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_input_update (const browser_def_t *browserDef, float touchX, float touchY)
{
  CX_ASSERT (browserDef);
  
  bool handled = false;
  
  // test for toolbar buttons
  
  if (!s_webview.hidden)
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
      float buttonHeight = (float) button->height;
      
      float bx1 = tbEndX - buttonWidth;
      float by1 = tby1 + ((toolbarHeight - buttonHeight) * 0.5f);
      float bx2 = bx1 + buttonWidth;
      float by2 = by1 + buttonHeight;
      
      if ((touchX >= bx1) && (touchX <= bx2))
      {
        if ((touchY >= by1) && (touchY <= by2))
        {
          s_touch [i] = true;
        }
      }
      
      tbEndX = bx1 - s_buttonSpacing;
    }
    
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
      [s_webview reload];
      handled = true;
    }
  }
  
  return handled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool browser_is_active (void)
{
  return (!s_animating && !s_webview.hidden);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
