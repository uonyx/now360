//
//  webview.m
//  earthnews
//
//  Created by Ubaka Onyechi on 30/03/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "webview.h"
#import "util.h"
#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>
#import "SVModalWebViewController.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WEBVIEW_DEBUG_CX_VIEWCTRL       1
#define WEBVIEW_DEBUG_LOG_ENABLED       1
#define WEBVIEW_UI_SIZE_WIDTH          (778.0f)
#define WEBVIEW_UI_SIZE_HEIGHT         (620.0f)
#define WEBVIEW_SCREEN_FADE_OPACITY    (0.6f)
#define WEBVIEW_SCREEN_FADE_DURATION   (0.5f)
#define WEBVIEW_TEXT_LABEL_WIDTH       (490.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface WebViewControllerDelegate : NSObject<SVModalWebViewControllerDelegate>
@end

#if 0
@interface PopOverControllerDelegate : NSObject<UIPopoverControllerDelegate>
@end
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXWebViewController : UIViewController <UIWebViewDelegate, UIActionSheetDelegate, MFMailComposeViewControllerDelegate>
- (id)initWithAddress:(NSString*)urlString title:(NSString *)title;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;
static bool s_active = false;
static UIViewController *s_rootViewCtrlr = nil;
static UIPopoverController *s_popover = nil;
#if WEBVIEW_DEBUG_CX_VIEWCTRL
static CXWebViewController *s_cxWebViewCtrlr = nil;
#else
static SVModalWebViewController *s_webViewCtrlr = nil;
static WebViewControllerDelegate *s_webViewDelegate = nil;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool webview_init (const void *rootvc)
{
  CX_ASSERT (!s_initialised);
  CX_ASSERT (rootvc);
  
  s_rootViewCtrlr = (UIViewController *) rootvc;
  
#if WEBVIEW_DEBUG_CX_VIEWCTRL
  s_cxWebViewCtrlr = [[CXWebViewController alloc] initWithAddress:@"" title:nil];
  
  s_popover = [[UIPopoverController alloc] initWithContentViewController:[s_cxWebViewCtrlr navigationController]];
#else
  s_webViewDelegate = [[WebViewControllerDelegate alloc] init];
  
  s_webViewCtrlr = [[SVModalWebViewController alloc] initWithAddress:@""];
  
  s_popover = [[UIPopoverController alloc] initWithContentViewController:s_webViewCtrlr];
#endif
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void webview_deinit (void)
{
  CX_ASSERT (s_initialised);
  
  [s_popover release];
  
#if WEBVIEW_DEBUG_CX_VIEWCTRL
  [s_cxWebViewCtrlr release];
#else
  [s_webViewDelegate release];
  [s_webViewCtrlr release];
#endif
  s_rootViewCtrlr = nil;
  
  s_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void webview_show (const char *url, const char *title)
{
  if (!s_active)
  {
    UIView *parentView = s_rootViewCtrlr.view;

    float viewPosX = parentView.bounds.origin.x;
    float viewPosY = parentView.bounds.origin.y;
    float viewWidth  = parentView.bounds.size.width;
    float viewHeight = parentView.bounds.size.height;

    float width = WEBVIEW_UI_SIZE_WIDTH;
    float height = WEBVIEW_UI_SIZE_HEIGHT;
    float posX = viewPosX + ((viewWidth - width) * 0.5f);
    float posY = viewPosY + ((viewHeight - height) * 0.5f);

#if WEBVIEW_DEBUG_CX_VIEWCTRL
    if (s_cxWebViewCtrlr)
    {
      [s_cxWebViewCtrlr release];
      s_cxWebViewCtrlr = nil;
    }
    
    NSString *objcURL = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    NSString *objcTitle = title ? [NSString stringWithCString:title encoding:NSASCIIStringEncoding] : nil;
    
    s_cxWebViewCtrlr = [[CXWebViewController alloc] initWithAddress:objcURL title:objcTitle];
    [s_cxWebViewCtrlr setContentSizeForViewInPopover:CGSizeMake (width, height)];
    
    [s_popover setContentViewController:[s_cxWebViewCtrlr navigationController]];
    [s_popover setPopoverContentSize:CGSizeMake(width, height)];
    [s_popover setPassthroughViews:[NSArray arrayWithObject:parentView]];
    [s_popover presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
#else
    if (s_webViewCtrlr)
    {
      [s_webViewCtrlr release];
      s_webViewCtrlr = nil;
    }

    NSString *objcURL = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    NSString *objcTitle = title ? [NSString stringWithCString:title encoding:NSASCIIStringEncoding] : nil;
    
    s_webViewCtrlr = [[SVModalWebViewController alloc] initWithAddress:objcURL title:objcTitle];
    [s_webViewCtrlr setSvdelegate:s_webViewDelegate];
    [s_webViewCtrlr setContentSizeForViewInPopover:CGSizeMake (width, height)];
    [s_webViewCtrlr setAvailableActions:(SVWebViewControllerAvailableActionsOpenInSafari |
                                         SVWebViewControllerAvailableActionsCopyLink |
                                         SVWebViewControllerAvailableActionsMailLink)];

    [s_popover setContentViewController:s_webViewCtrlr];
    [s_popover setPopoverContentSize:CGSizeMake(width, height)];
    [s_popover setPassthroughViews:[NSArray arrayWithObject:parentView]];
    [s_popover presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
#endif
    util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, WEBVIEW_SCREEN_FADE_OPACITY, WEBVIEW_SCREEN_FADE_DURATION, NULL, NULL);
    
    s_active = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
    
void webview_hide (void)
{
  if (s_active)
  {
    [s_popover dismissPopoverAnimated:YES];
    
    util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, WEBVIEW_SCREEN_FADE_OPACITY, WEBVIEW_SCREEN_FADE_DURATION, NULL, NULL);
    
    s_active = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool webview_active (void)
{
  return s_active;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation WebViewControllerDelegate

- (void) doneButtonClicked:(SVModalWebViewController *)webViewController
{
  webview_hide ();
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
@implementation PopOverControllerDelegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
  if (s_webViewCtrlr)
  {
    [s_webViewCtrlr release];
    s_webViewCtrlr = nil;
  }
}

@end
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXWebViewController ()
{
  UINavigationController *_navCtrlr;
  UIWebView *_webView;
  NSURL *_url;
  UIBarButtonItem *_doneButton;
  UIBarButtonItem *_forwardButton;
  UIBarButtonItem *_backButton;
  UIBarButtonItem *_stopButton;
  UIBarButtonItem *_reloadButton;
  UIBarButtonItem *_actionButton;
  UIBarButtonItem *_fixedSpace60;
  UIBarButtonItem *_fixedSpace5;
  UILabel *_titleLabel;
  UIActionSheet *_actionSheet;
  BOOL _browserIsChrome;
}
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation CXWebViewController

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (id)initWithAddress:(NSString*)urlString title:(NSString *)title
{
  self = [super init];
  
  if (self)
  {
    _webView = nil;
    
    _url = [NSURL URLWithString:urlString];
    
    _navCtrlr = [[UINavigationController alloc] initWithRootViewController:self];
  
    _doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                                target:self
                                                                action:@selector(doneButtonClicked:)];
    
    _reloadButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                  target:self
                                                                  action:@selector(reloadButtonClicked:)];

    _actionButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAction
                                                                  target:self
                                                                  action:@selector(actionButtonClicked:)];
    
    _stopButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemStop
                                                                  target:self
                                                                  action:@selector(stopButtonClicked:)];
    
    _backButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"data/images/webview/back"]
                                                   style:UIBarButtonItemStylePlain
                                                  target:self
                                                  action:@selector(backButtonClicked:)];
    _backButton.imageInsets = UIEdgeInsetsMake(2.0f, 0.0f, -2.0f, 0.0f);
		_backButton.width = 18.0f;
    
    _forwardButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"data/images/webview/forward"]
                                                      style:UIBarButtonItemStylePlain
                                                     target:self
                                                     action:@selector(forwardButtonClicked:)];
    _forwardButton.imageInsets = UIEdgeInsetsMake(2.0f, 0.0f, -2.0f, 0.0f);
		_forwardButton.width = 18.0f;
    
    _fixedSpace60 = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                                                                  target:nil
                                                                  action:nil];
    _fixedSpace60.width = 60.0f;
    
    _fixedSpace5 = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                                                                 target:nil
                                                                 action:nil];
    _fixedSpace5.width = 5.0f;
    
    _titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 400.0f, 44.0f)];
    _titleLabel.backgroundColor = [UIColor clearColor];
    _titleLabel.font = [UIFont boldSystemFontOfSize:14.0];
    _titleLabel.textColor = [UIColor whiteColor];
    _titleLabel.textAlignment = UITextAlignmentLeft;
    [self setTitleLabelText:title]; //_titleLabel.text = title;
    
    _actionSheet = [[UIActionSheet alloc] initWithTitle:_url.absoluteString
                                               delegate:self
                                      cancelButtonTitle:nil
                                 destructiveButtonTitle:nil
                                      otherButtonTitles:nil];
    
    [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_COPY_LINK", nil)];
    
    if ([MFMailComposeViewController canSendMail])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_MAIL_LINK", nil)];
    }

    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:@"googlechrome://"]])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_OPEN_CHROME", nil)];
      _browserIsChrome = TRUE;
    }
    else
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_OPEN_SAFARI", nil)];
    }
    
    [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_CANCEL", nil)];
    
    [_actionSheet setCancelButtonIndex:[_actionSheet numberOfButtons] - 1];
    
    self.navigationItem.titleView = _titleLabel;
    self.navigationItem.rightBarButtonItems = [NSArray arrayWithObjects:
                                               _fixedSpace5,
                                               _doneButton,
                                               _actionButton,
                                               _forwardButton,
                                               _backButton,
                                               _stopButton,
                                               _fixedSpace60,
                                               nil];
  }
  
  return self;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)dealloc
{
  [_doneButton release];
  [_reloadButton release];
  [_actionButton release];
  [_stopButton release];
  [_backButton release];
  [_forwardButton release];
  [_fixedSpace5 release];
  [_fixedSpace60 release];
  [_titleLabel release];
  [_actionSheet release];
  [_navCtrlr release];
  [_webView release];
  
  [super dealloc];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)loadView
{
  CX_ASSERT (_webView == nil);
  
  _webView = [[UIWebView alloc] init];
  [_webView setDelegate:self];
  [_webView setScalesPageToFit:YES];
  [_webView loadRequest:[NSURLRequest requestWithURL:_url
                                         cachePolicy:NSURLRequestReturnCacheDataElseLoad
                                     timeoutInterval:30]];
  self.view = _webView;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)viewDidLoad
{
  [super viewDidLoad];
  // set up view
  
  //self.navigationController.navigationBar.tintColor = [UIColor blackColor];
  //self.navigationController.navigationBar.translucent = YES;
  //self.navigationController.navigationBar.barStyle = UIBarStyleBlackTranslucent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)viewDidUnload
{
  [_webView release];
  _webView = nil;
  
  [super viewDidUnload];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)viewWillAppear:(BOOL)animated
{
  //[self setContentSizeForViewInPopover:CGSizeMake (SETTINGS_UI_SIZE_WIDTH, SETTINGS_UI_SIZE_HEIGHT)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)viewWillDisappear:(BOOL)animated
{
  [_webView release];
  _webView = nil;
  self.view = nil;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (NSInteger)supportedInterfaceOrientations
{
  return UIInterfaceOrientationMaskLandscape;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

-(void)setTitleLabelText:(NSString *)title
{
  // finest hackery for text alignment. works? ship it!
  
  NSMutableString *mtitle = nil;
  
  if (title)
  {
    mtitle = [NSMutableString stringWithFormat:@"  %@", title];
    
    CGSize textSize = [mtitle sizeWithFont:[_titleLabel font]];
    
    float w = textSize.width;
    
    while (w < WEBVIEW_TEXT_LABEL_WIDTH)
    {
      [mtitle appendString:@"  "];
      
      w = [mtitle sizeWithFont:[_titleLabel font]].width;
    }
  }
  
  _titleLabel.text = mtitle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)updateButtons
{  
  UIBarButtonItem *reloadORStop = _webView.isLoading ? _stopButton : _reloadButton;
  
  self.navigationItem.rightBarButtonItems = [NSArray arrayWithObjects:
                                             _fixedSpace5,
                                             _doneButton,
                                             _actionButton,
                                             _forwardButton,
                                             _backButton,
                                             reloadORStop,
                                             _fixedSpace60,
                                             nil];
  _backButton.enabled = _webView.canGoBack;
  _forwardButton.enabled = _webView.canGoForward;
  _actionButton.enabled = !_webView.isLoading;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)forwardButtonClicked:(id)sender
{
  [_webView goForward];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)backButtonClicked:(id)sender
{
  [_webView goBack];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)stopButtonClicked:(id)sender
{
  [_webView stopLoading];
  
  [self updateButtons];
  
  util_activity_indicator_set_active (false); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)reloadButtonClicked:(id)sender
{
  [_webView reload];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)actionButtonClicked:(id)sender
{
  [_actionButton setTitle:_webView.request.URL.absoluteString];
  [_actionSheet showFromBarButtonItem:_actionButton animated:YES];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)doneButtonClicked:(id)sender
{
  [_webView stopLoading];
  
  util_activity_indicator_set_active (false);
  
#if 0
  [webViewController dismissViewControllerAnimated:YES completion:NULL];
#else
  webview_hide ();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
  switch (navigationType)
  {
    case UIWebViewNavigationTypeLinkClicked:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeLinkClicked)");
      //[webView stringByEvaluatingJavaScriptFromString:@"document.body.innerHTML = \"\";"];
      //[webView loadRequest:request];
      break;
    }
    case UIWebViewNavigationTypeFormSubmitted:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeFormSubmitted)");
      break;
    }
    case UIWebViewNavigationTypeBackForward:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeBackForward)");
      break;
    }
    case UIWebViewNavigationTypeReload:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeReload)");
      break;
    }
    case UIWebViewNavigationTypeFormResubmitted:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeFormResubmitted)");
      break;
    }
    case UIWebViewNavigationTypeOther:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: UIWebViewNavigationTypeOther)");
      break;
    }
    default:
    {
      CX_DEBUGLOG_CONSOLE (WEBVIEW_DEBUG_LOG_ENABLED, "Webview: navigationType: Unknown!)");
      break;
    }
  }
  
#if 0
  NSString *hostname = [[request URL] host];
  NSLog (@"Webview: Hostname: %@", hostname);
  NSLog (@"Webview: request: %@", request);
#endif
  
  if ([request isKindOfClass:[NSMutableURLRequest class]])
  {
    NSMutableURLRequest *mutableRequest = (NSMutableURLRequest *) request;
    [mutableRequest setCachePolicy:NSURLRequestReturnCacheDataElseLoad];
  }
  
  return YES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webViewDidStartLoad:(UIWebView *)webView
{
  util_activity_indicator_set_active (true); //[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES];
  
  [self updateButtons];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webViewDidFinishLoad:(UIWebView *)webView
{
  NSString *docTitle = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
  
  if (docTitle && docTitle.length > 0)
  {
    [self setTitleLabelText:docTitle];
  }
  
  util_activity_indicator_set_active (false); //[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:NO];
  
  [self updateButtons];
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
  const int ACTION_ITEM_INDEX_COPY_LINK = 0;
  const int ACTION_ITEM_INDEX_MAIL_LINK = 1;
  const int ACTION_ITEM_INDEX_OPEN_BROWSER = 2;
  const int ACTION_ITEM_INDEX_CANCEL = 3;
  
  switch (buttonIndex)
  {
    case ACTION_ITEM_INDEX_COPY_LINK:
    {
      UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
      pasteboard.string = _webView.request.URL.absoluteString;
      break;
    }
      
    case ACTION_ITEM_INDEX_MAIL_LINK:
    {
      MFMailComposeViewController *mailViewController = [[MFMailComposeViewController alloc] init];
      
      mailViewController.mailComposeDelegate = self;
      [mailViewController setSubject:[_webView stringByEvaluatingJavaScriptFromString:@"document.title"]];
      [mailViewController setMessageBody:_webView.request.URL.absoluteString isHTML:NO];
      mailViewController.modalPresentationStyle = UIModalPresentationFormSheet;
      
      [self presentViewController:mailViewController animated:YES completion:NULL];
      break;
    }
      
    case ACTION_ITEM_INDEX_OPEN_BROWSER:
    {
      if (_browserIsChrome)
      {
        NSURL *inputURL = _webView.request.URL;
        
        NSString *chromeScheme = [inputURL.scheme isEqualToString:@"https"] ? @"googlechromes" : @"googlechrome";
        NSString *absoluteString = [inputURL absoluteString];
        
        NSRange rangeForScheme = [absoluteString rangeOfString:@":"];
        NSString *urlNoScheme = [absoluteString substringFromIndex:rangeForScheme.location];
        
        NSString *chromeURLString = [chromeScheme stringByAppendingString:urlNoScheme];
        
        [[UIApplication sharedApplication] openURL:[NSURL URLWithString:chromeURLString]];
      }
      else
      {
        [[UIApplication sharedApplication] openURL:_webView.request.URL];
      }
      
      break;
    }
      
    case ACTION_ITEM_INDEX_CANCEL:
    {
      break;
    }
      
    default:
    {
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)mailComposeController:(MFMailComposeViewController *)controller
          didFinishWithResult:(MFMailComposeResult)result
                        error:(NSError *)error
{
  [self dismissViewControllerAnimated:YES completion:NULL];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////