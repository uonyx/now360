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
#import "metrics.h"
#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>
#import <Social/Social.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WEBVIEW_DEBUG_LOG_ENABLED      (CX_DEBUG && 1)
#define WEBVIEW_UI_SIZE_WIDTH          (778.0f)
#define WEBVIEW_UI_SIZE_HEIGHT         (620.0f)
#define WEBVIEW_SCREEN_FADE_OPACITY    (0.6f)
#define WEBVIEW_SCREEN_FADE_DURATION   (0.5f)
#define WEBVIEW_TEXT_LABEL_WIDTH       (490.0f)
#define WEBVIEW_USE_ACTION_SHEET        0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXWebViewController : UIViewController <UIWebViewDelegate, UIActionSheetDelegate, MFMailComposeViewControllerDelegate>
- (id)initWithAddress:(NSString*)urlString title:(NSString *)title;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXWebViewPopoverBackground : UIPopoverBackgroundView
@property (nonatomic, readwrite) UIPopoverArrowDirection arrowDirection;
@property (nonatomic, readwrite) CGFloat arrowOffset;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool g_initialised = false;
static bool g_active = false;
static UIViewController *g_rootViewCtrlr = nil;
static UIPopoverController *g_popover = nil;
static CXWebViewController *g_webviewCtrlr = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool webview_init (const void *rootvc)
{
  CX_ASSERT (!g_initialised);
  CX_ASSERT (rootvc);
  
  g_rootViewCtrlr = (UIViewController *) rootvc;
  
  //g_webviewCtrlr = [[CXWebViewController alloc] initWithAddress:@"" title:nil];
  
  return g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void webview_deinit (void)
{
  CX_ASSERT (g_initialised);
  
  [g_webviewCtrlr release];

  [g_popover release];

  g_rootViewCtrlr = nil;
  
  g_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void webview_show (const char *url, const char *title)
{
  if (!g_active)
  {
    UIView *parentView = g_rootViewCtrlr.view;

    float viewPosX = parentView.bounds.origin.x;
    float viewPosY = parentView.bounds.origin.y;
    float viewWidth  = parentView.bounds.size.width;
    float viewHeight = parentView.bounds.size.height;

    float width = WEBVIEW_UI_SIZE_WIDTH;
    float height = WEBVIEW_UI_SIZE_HEIGHT;
    float posX = viewPosX + ((viewWidth - width) * 0.5f);
    float posY = viewPosY + ((viewHeight - height) * 0.5f);

    if (g_webviewCtrlr)
    {
      [g_webviewCtrlr release];
      g_webviewCtrlr = nil;
    }
    
    NSString *objcURL = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    NSString *objcTitle = title ? [NSString stringWithCString:title encoding:NSASCIIStringEncoding] : nil;
    
    g_webviewCtrlr = [[CXWebViewController alloc] initWithAddress:objcURL title:objcTitle];
    [g_webviewCtrlr setContentSizeForViewInPopover:CGSizeMake (width, height)];
    
    if (!g_popover)
    {
      g_popover = [[UIPopoverController alloc] initWithContentViewController:[g_webviewCtrlr navigationController]];
      
      [g_popover setPopoverBackgroundViewClass:[CXWebViewPopoverBackground class]];
    }
    else
    {
      [g_popover setContentViewController:[g_webviewCtrlr navigationController]];
    }
    
    [g_popover setPopoverContentSize:CGSizeMake(width, height)];
    [g_popover setPassthroughViews:[NSArray arrayWithObject:parentView]];
    [g_popover presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
    
    util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, WEBVIEW_SCREEN_FADE_OPACITY, WEBVIEW_SCREEN_FADE_DURATION, NULL, NULL);
    
    g_active = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
    
void webview_hide (void)
{
  if (g_active)
  {
    [g_popover dismissPopoverAnimated:YES];
    
    util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, WEBVIEW_SCREEN_FADE_OPACITY, WEBVIEW_SCREEN_FADE_DURATION, NULL, NULL);
    
    g_active = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool webview_active (void)
{
  return g_active;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Webview Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if WEBVIEW_USE_ACTION_SHEET
typedef enum
{
  CX_WEBVIEW_ACTION_ITEM_INVALID = -1,
  CX_WEBVIEW_ACTION_ITEM_COPY_LINK,
  CX_WEBVIEW_ACTION_ITEM_MAIL_LINK,
  CX_WEBVIEW_ACTION_ITEM_OPEN_SAFARI,
  CX_WEBVIEW_ACTION_ITEM_OPEN_CHROME,
  CX_WEBVIEW_ACTION_ITEM_POST_TWITTER,
  CX_WEBVIEW_ACTION_ITEM_POST_FACEBOOK,
  CX_WEBVIEW_ACTION_ITEM_CANCEL,
  CX_WEBVIEW_NUM_ACTION_ITEMS
} CXWebViewActionItem;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXWebViewActivityItemText : UIActivityItemProvider
{
}
@property (copy) NSString *strTwitter;
@end


@implementation CXWebViewActivityItemText
@synthesize strTwitter;

- (id)initWithStrings:(NSString *)defaultText twitterText:(NSString *)twitterText
{
  self = [super init];
  
  if (self)
  {
    [self initWithPlaceholderItem:defaultText];
    
    [self setStrTwitter:twitterText];
  }
  
  return self;
}

- (id)item
{
  CX_ASSERT ([self.placeholderItem isKindOfClass:[NSString class]]);
  
  if ([self.activityType isEqualToString:UIActivityTypePostToTwitter])
  {
    return strTwitter;
  }
  else if ([self.activityType isEqualToString:UIActivityTypeCopyToPasteboard])
  {
    return nil;
  }

  return self.placeholderItem;
}
@end

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
  
#if WEBVIEW_USE_ACTION_SHEET
  UIActionSheet *_actionSheet;
  int _actionItemIndices [CX_WEBVIEW_NUM_ACTION_ITEMS];
#endif
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
    
    //_navCtrlr.navigationBar.tintColor = [UIColor redColor];
    //_navCtrlr.navigationBar.translucent = YES;
    //_navCtrlr.navigationBar.barStyle = UIBarStyleBlackTranslucent;
  
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
    
#if WEBVIEW_USE_ACTION_SHEET
    _actionSheet = [[UIActionSheet alloc] initWithTitle:_url.absoluteString
                                               delegate:self
                                      cancelButtonTitle:nil
                                 destructiveButtonTitle:nil
                                      otherButtonTitles:nil];
    
    memset (_actionItemIndices, -1, sizeof (_actionItemIndices));
    
    int currIndex = 0;
    
    [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_COPY_LINK", nil)];
    _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_COPY_LINK] = currIndex++;
    
    if ([MFMailComposeViewController canSendMail])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_MAIL_LINK", nil)];
      _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_MAIL_LINK] = currIndex++;
    }

    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:@"googlechrome://"]])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_OPEN_CHROME", nil)];
      _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_OPEN_CHROME] = currIndex++;
    }
    else
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_OPEN_SAFARI", nil)];
      _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_OPEN_SAFARI] = currIndex++;
    }
    
    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:@"twitter://"]])
      //if ([SLComposeViewController isAvailableForServiceType:SLServiceTypeTwitter])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_POST_TWITTER", nil)];
      _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_POST_TWITTER] = currIndex++;
    }

    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:@"fb://profile"]])
      //if ([SLComposeViewController isAvailableForServiceType:SLServiceTypeFacebook])
    {
      [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_POST_FACEBOOK", nil)];
      _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_POST_FACEBOOK] = currIndex++;
    }
    
    [_actionSheet addButtonWithTitle:NSLocalizedString (@"TXT_CANCEL", nil)];
    _actionItemIndices [CX_WEBVIEW_ACTION_ITEM_CANCEL] = currIndex++;
    
    [_actionSheet setCancelButtonIndex:_actionItemIndices [CX_WEBVIEW_ACTION_ITEM_CANCEL]];
#endif
    
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
#if WEBVIEW_USE_ACTION_SHEET
  [_actionSheet release];
#endif
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
  
  NSString *docTitle = [_webView stringByEvaluatingJavaScriptFromString:@"document.title"];
  
  if (docTitle && docTitle.length > 0)
  {
    [_actionButton setEnabled:YES];
  }
  
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

-(void)actionButtonClicked:(id)sender
{
#if WEBVIEW_USE_ACTION_SHEET
  
  //[_actionButton setTitle:_webView.request.URL.absoluteString];
  [_actionSheet showFromBarButtonItem:_actionButton animated:YES];
  
#else
  
  NSURL *link = _webView.request.URL;
  NSString *title = [_webView stringByEvaluatingJavaScriptFromString:@"document.title"];
  NSString *sigDefault = @"(via uonyechi.com/now360)";
  NSString *sigTwitter = @"(via @now360_app)";
  
  NSString *postDefault = [NSString stringWithFormat:@"%@\n", title];
  NSString *postDefaultSig = [NSString stringWithFormat:@"\n %@", sigDefault];
  NSString *postTwitter = [NSString stringWithFormat:@"%@ %@", title, sigTwitter];
  NSString *postTwitterSig = nil;
  
#if WEBVIEW_DEBUG_LOG_ENABLED
  NSLog (@"post: %d", [postTwitter length]);
  NSLog (@"titl: %d", [title length]);
  NSLog (@"link: %d", [[link absoluteString] length]);
#endif
  
  const int twitterMaxTextLen = 140;
  
  if (postTwitter.length > twitterMaxTextLen)
  {
    if (title.length > twitterMaxTextLen)
    {
      if (link.absoluteString.length > twitterMaxTextLen)
      {
        postTwitter = sigTwitter;
      }
      else
      {
        if ((link.absoluteString.length + sigTwitter.length + 1) > twitterMaxTextLen)
        {
          postTwitter = [NSString stringWithFormat:@"%@ %@", link.absoluteString, sigTwitter];
        }
        else
        {
          postTwitter = link.absoluteString;
        }
      }
    }
    else
    {
      postTwitter = title;
    }
  }
  
  CXWebViewActivityItemText *postMsg = [[CXWebViewActivityItemText alloc] initWithStrings:postDefault
                                                                              twitterText:postTwitter];
  
  CXWebViewActivityItemText *postSig = [[CXWebViewActivityItemText alloc] initWithStrings:postDefaultSig
                                                                              twitterText:postTwitterSig];
  
  NSArray *activityItems = @[postMsg, link, postSig];
  
  UIActivityViewController *activityViewCtrlr = [[UIActivityViewController alloc] initWithActivityItems:activityItems
                                                                                  applicationActivities:nil];
  
  activityViewCtrlr.completionHandler = ^(NSString *activityType, BOOL completed)
  {
#if WEBVIEW_DEBUG_LOG_ENABLED
    NSLog (@" activityType: %@", activityType);
    NSLog (@" completed: %i", completed);
#endif
    
    [postMsg release];
    [postSig release];
    [activityViewCtrlr release];
  };
  
  [self presentViewController:activityViewCtrlr animated:YES completion:nil];
  
#endif
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
  
#if (WEBVIEW_DEBUG_LOG_ENABLED && 0)
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
  util_activity_indicator_set_active (true);
  
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
  
  util_activity_indicator_set_active (false);
  
  [self updateButtons];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void) webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
#if WEBVIEW_DEBUG_LOG_ENABLED
  NSLog (@"Webview: Error: [%d] %@", [error code], [error description]);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if WEBVIEW_USE_ACTION_SHEET
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
  CXWebViewActionItem actionItem = CX_WEBVIEW_ACTION_ITEM_INVALID;
  
  for (int i = 0; i < CX_WEBVIEW_NUM_ACTION_ITEMS; ++i)
  {
    if (_actionItemIndices [i] == buttonIndex)
    {
      actionItem = i;
      break;
    }
  }
  
  switch (actionItem)
  {
    case CX_WEBVIEW_ACTION_ITEM_COPY_LINK:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "copy");
      
      UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
      pasteboard.string = _webView.request.URL.absoluteString;
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_MAIL_LINK:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "email");
      
      MFMailComposeViewController *mailViewController = [[MFMailComposeViewController alloc] init];
      
      [mailViewController setMailComposeDelegate:self];
      [mailViewController setSubject:[_webView stringByEvaluatingJavaScriptFromString:@"document.title"]];
      [mailViewController setMessageBody:_webView.request.URL.absoluteString isHTML:NO];
      [mailViewController setModalPresentationStyle:UIModalPresentationFormSheet];
      
      [self presentViewController:mailViewController animated:YES completion:NULL];
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_OPEN_CHROME:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "chrome");
      
      NSURL *link = _webView.request.URL;
      
      NSString *chromeScheme = [link.scheme isEqualToString:@"https"] ? @"googlechromes" : @"googlechrome";
      NSString *absoluteString = [link absoluteString];
      NSRange rangeForScheme = [absoluteString rangeOfString:@":"];
      NSString *urlNoScheme = [absoluteString substringFromIndex:rangeForScheme.location];
      NSString *chromeURLString = [chromeScheme stringByAppendingString:urlNoScheme];
      
      [[UIApplication sharedApplication] openURL:[NSURL URLWithString:chromeURLString]];
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_OPEN_SAFARI:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "safari");
      
      [[UIApplication sharedApplication] openURL:_webView.request.URL];
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_POST_TWITTER:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "twitter");
      
      NSURL *link = _webView.request.URL;
      NSString *title = [_webView stringByEvaluatingJavaScriptFromString:@"document.title"];
      NSString *sig = @"(via @now360_app)";
      NSString *post = [NSString stringWithFormat:@"%@ %@", title, sig];
      
      const int twitterMaxTextLen = 140;
      
      if (post.length > twitterMaxTextLen)
      {
        if (title.length > twitterMaxTextLen)
        {
          if (link.absoluteString.length > twitterMaxTextLen)
          {
            post = sig;
          }
          else
          {
            if ((link.absoluteString.length + sig.length + 1) > twitterMaxTextLen)
            {
              post = [NSString stringWithFormat:@"%@ %@", link.absoluteString, sig];
            }
            else
            {
              post = link.absoluteString;
            }
          }
        }
        else
        {
          post = title;
        }
      }
      
      SLComposeViewController *tweetSheet = [SLComposeViewController composeViewControllerForServiceType:SLServiceTypeTwitter];
      
      [tweetSheet setInitialText:post];
      [tweetSheet addURL:link];
      
      [self presentViewController:tweetSheet animated:YES completion:nil];
      
      [tweetSheet.view endEditing:YES];
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_POST_FACEBOOK:
    {
      metrics_event_log (METRICS_EVENT_LINK_SHARE, "facebook");
      
      NSURL *link = _webView.request.URL;
      NSString *title = [_webView stringByEvaluatingJavaScriptFromString:@"document.title"];
      NSString *sig = @"(via uonyechi.com/now360)";
      NSString *post = [NSString stringWithFormat:@"%@\n %@", title, sig];

      SLComposeViewController *fbPostSheet = [SLComposeViewController composeViewControllerForServiceType:SLServiceTypeFacebook];
      
      [fbPostSheet setInitialText:post];
      [fbPostSheet addURL:link];
      
      [self presentViewController:fbPostSheet animated:YES completion:nil];
      
      break;
    }
      
    case CX_WEBVIEW_ACTION_ITEM_CANCEL:
    {
      break;
    }
      
    default:
    {
      CX_ERROR ("CX_WEBVIEW_ACTION_ITEM_INVALID");
      
      break;
    }
  }
}
#endif

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation CXWebViewPopoverBackground

@synthesize arrowOffset;
@synthesize arrowDirection;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (id)initWithFrame:(CGRect)frame
{
  if (self = [super initWithFrame:frame])
  {
    self.backgroundColor = [UIColor colorWithWhite:0.15f alpha:0.5f];
    self.arrowDirection = 0;
    self.arrowOffset = 0.0f;
  }
  
  return self;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (UIEdgeInsets)contentViewInsets
{
  return UIEdgeInsetsMake (10.0f, 0.0f, 1.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (CGFloat)arrowHeight
{
  return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (CGFloat)arrowBase
{
  return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
