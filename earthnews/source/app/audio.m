//
//  audio.c
//  earthnews
//
//  Created by Ubaka Onyechi on 12/01/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#import "audio.h"
#import <MediaPlayer/MediaPlayer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG_ENABLE 1
#define USE_MUSIC_PICKER_POP_UP 1 // ipad only
#define PLAYBACK_STATE_HACK_FIX 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SYSTEM_VERSION_GREATER_THAN(v)              ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedDescending)
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_init (void);
static void audio_music_deinit (void);
static bool audio_music_update_queue (MPMediaItemCollection *collection);
static void audio_music_picker (bool show, audio_music_picked_callback fn);
static void audio_music_playback_state_changed (void);
static void audio_music_now_playing_state_changed (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicPickerDelegate : UIViewController<MPMediaPickerControllerDelegate>
{
}
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MusicPickerDelegate

- (void) mediaPicker:(MPMediaPickerController *)mediaPicker didPickMediaItems:(MPMediaItemCollection *)mediaItemCollection
{
  audio_music_update_queue (mediaItemCollection);
      
  audio_music_picker (false, NULL);  
}

- (void) mediaPickerDidCancel:(MPMediaPickerController *)mediaPicker
{
  audio_music_picker (false, NULL);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicNotifcation : NSObject
{
}
- (void) handleNowPlayingItemChanged:(NSNotification *)notification;
- (void) handlePlaybackStateChanged:(NSNotification *)notification;
- (void) handleVolumeChanged:(NSNotification *)notification;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MusicNotifcation

- (void) handleNowPlayingItemChanged:(NSNotification *)notification
{
  audio_music_now_playing_state_changed ();
}

- (void) handlePlaybackStateChanged:(NSNotification *)notification
{
  audio_music_playback_state_changed ();
}

- (void) handleVolumeChanged:(NSNotification *)notification
{
  CX_DEBUG_BREAKABLE_EXPR;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicPickerPopupBackground : UIPopoverBackgroundView
{
}

@property (nonatomic, readwrite) UIPopoverArrowDirection arrowDirection;
@property (nonatomic, readwrite) CGFloat arrowOffset;

+(UIEdgeInsets)contentViewInsets;
+(CGFloat)arrowHeight;
+(CGFloat)arrowBase;
@end

@implementation MusicPickerPopupBackground

@synthesize arrowOffset, arrowDirection;

-(id)initWithFrame:(CGRect)frame
{
  if (self = [super initWithFrame:frame])
  {
    self.backgroundColor = [UIColor colorWithWhite:0.1f alpha:0.5f];
    self.arrowDirection = 0;
    self.arrowOffset = 0.0f;
  }

  return self;
}

+(UIEdgeInsets)contentViewInsets
{
  return UIEdgeInsetsMake (10.0f, 10.0f, 10.0f, 10.0f);
}

+(CGFloat)arrowHeight
{
  return 30.0f;
}

+(CGFloat)arrowBase
{
  return 30.0f;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;
static bool s_pickerActive = false;
static UIViewController *s_rootViewController = nil;
static MPMusicPlayerController *s_musicPlayer = nil;
static MPMediaPickerController *s_musicPicker = nil;
static MusicPickerDelegate *s_musicPickerDelegate = nil;
static MusicNotifcation *s_musicNotification = nil;
static MPMediaItemCollection *s_currentCollection = nil;
static audio_music_picked_callback s_pickerCallback = NULL;
static cx_list2 s_musicNotificationCallbacks;
#if USE_MUSIC_PICKER_POP_UP
static UIPopoverController *s_musicPopOver = nil;
#endif
#if PLAYBACK_STATE_HACK_FIX
static bool s_musicPlaying = false;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_init (void *rootvc)
{
  CX_ASSERT (!s_initialised);
  CX_ASSERT (rootvc);
  
  s_rootViewController = (UIViewController *) rootvc;
  
  cx_list2_init (&s_musicNotificationCallbacks);
  
  audio_music_init ();
  
  s_initialised = true;
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_deinit (void)
{
  CX_ASSERT (s_initialised);
  
  audio_music_deinit ();
  
  cx_list2_deinit (&s_musicNotificationCallbacks);
  
  s_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_notification_register (audio_music_notification_callback fn)
{
  CX_ASSERT (fn);
  CX_ASSERT (!cx_list2_exists (&s_musicNotificationCallbacks, fn));
  
  cx_list2_insert_back (&s_musicNotificationCallbacks, fn);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_pick (audio_music_picked_callback fn)
{
  audio_music_picker (true, fn);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_play (void)
{
  CX_ASSERT (s_musicPlayer);
  
  [s_musicPlayer play];
  
#if PLAYBACK_STATE_HACK_FIX
  s_musicPlaying = true;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_pause (void)
{
  CX_ASSERT (s_musicPlayer);
  
  [s_musicPlayer pause];
  
#if PLAYBACK_STATE_HACK_FIX
  s_musicPlaying = false;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_next (void)
{
  CX_ASSERT (s_musicPlayer);
  
  [s_musicPlayer skipToNextItem];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_prev (void)
{
  CX_ASSERT (s_musicPlayer);
  
  [s_musicPlayer skipToPreviousItem];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_playing (void)
{
  CX_ASSERT (s_musicPlayer);
  
  bool ret = false;

  MPMediaItem *nowPlayingItem = [s_musicPlayer nowPlayingItem];
  
#if PLAYBACK_STATE_HACK_FIX
  ret = nowPlayingItem && s_musicPlaying;
#else

  MPMusicPlaybackState playbackState = [s_musicPlayer playbackState];

  ret = (nowPlayingItem && (playbackState == MPMusicPlaybackStatePlaying))
#endif
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_picker_active (void)
{
  return s_pickerActive;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_queued (void)
{
  return s_currentCollection ? true : false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int audio_music_get_track_id (char *buffer, int bufferlen)
{
  CX_ASSERT (s_musicPlayer);
  CX_ASSERT (buffer);
  CX_ASSERT (bufferlen > 0);
  
  int size = 0;
  
  MPMediaItem *nowPlayingItem = [s_musicPlayer nowPlayingItem];
  
  if (nowPlayingItem)
  {
    NSString *artist = [nowPlayingItem valueForProperty:MPMediaItemPropertyArtist];
    NSString *title = [nowPlayingItem valueForProperty:MPMediaItemPropertyTitle];
    
    const char *a = [artist cStringUsingEncoding:NSASCIIStringEncoding];
    const char *t = [title cStringUsingEncoding:NSASCIIStringEncoding];
    
    size = cx_sprintf (buffer, bufferlen, "%s - %s", a, t);
  }
  
  return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_init (void)
{
  s_musicPlayer = [MPMusicPlayerController applicationMusicPlayer];
  
  s_musicNotification = [[MusicNotifcation alloc] init];
  
  s_musicPickerDelegate = [[MusicPickerDelegate alloc] init];
  
  s_musicPicker = [[MPMediaPickerController alloc] initWithMediaTypes:MPMediaTypeAnyAudio];
  
#if USE_MUSIC_PICKER_POP_UP
  s_musicPopOver = [[UIPopoverController alloc] initWithContentViewController:s_musicPicker];
#endif
  
  [s_musicPicker setDelegate:s_musicPickerDelegate];
  [s_musicPicker setAllowsPickingMultipleItems:YES];
  [s_musicPicker setTitle:@""];
  [s_musicPicker setModalInPopover:YES];
  
  //[s_musicPicker setPrompt:@"Queue songs for playback"];
  
  //[[UIBarButtonItem appearanceWhenContainedIn:[UIPopoverController class], nil] setTintColor:[UIColor colorWithWhite:0.1f alpha:1.0f]];
  
  [[UINavigationBar appearanceWhenContainedIn:[UIPopoverController class], nil] setTintColor:[UIColor colorWithWhite:0.0f alpha:0.5f]];

  [[UITableViewCell appearanceWhenContainedIn:[UIPopoverController class], nil] setBackgroundColor:[UIColor colorWithWhite:0.0f alpha:0.4f]];
  
  //[[UITableViewCell appearanceWhenContainedIn:[UIPopoverController class], nil] setBackgroundView:nil];
  
  //[[UITableViewCell appearanceWhenContainedIn:[UIPopoverController class], nil] setAlpha:0.3f];
  

  if (SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO (@"5.0"))
  {
    [s_musicPopOver setPopoverBackgroundViewClass:[MusicPickerPopupBackground class]];
  }
  
  NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
  
  [notificationCenter addObserver:s_musicNotification 
                         selector:@selector (handleNowPlayingItemChanged:) 
                             name:MPMusicPlayerControllerNowPlayingItemDidChangeNotification 
                           object:s_musicPlayer];
  
  [notificationCenter addObserver:s_musicNotification 
                         selector:@selector (handlePlaybackStateChanged:) 
                             name:MPMusicPlayerControllerPlaybackStateDidChangeNotification 
                           object:s_musicPlayer];

  [notificationCenter addObserver:s_musicNotification 
                         selector:@selector (handleVolumeChanged:) 
                             name:MPMusicPlayerControllerVolumeDidChangeNotification 
                           object:s_musicPlayer];
  
  [s_musicPlayer beginGeneratingPlaybackNotifications];
  [s_musicPlayer setRepeatMode:MPMusicRepeatModeAll];
  [s_musicPlayer setShuffleMode:MPMusicShuffleModeOff];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_deinit (void)
{
  NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
  
  [notificationCenter removeObserver:s_musicNotification 
                             name:MPMusicPlayerControllerNowPlayingItemDidChangeNotification 
                           object:s_musicPlayer];
  
  [notificationCenter removeObserver:s_musicNotification 
                             name:MPMusicPlayerControllerPlaybackStateDidChangeNotification 
                           object:s_musicPlayer];
  
  [notificationCenter removeObserver:s_musicNotification 
                                name:MPMusicPlayerControllerVolumeDidChangeNotification 
                              object:s_musicPlayer];
  
  [s_musicPlayer endGeneratingPlaybackNotifications];
  
#if USE_MUSIC_PICKER_POP_UP
  [s_musicPopOver release];
#endif
  
  [s_musicPicker release];
  
  [s_musicPickerDelegate release];
  
  [s_musicNotification release];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool audio_music_update_queue (MPMediaItemCollection *collection)
{  
  if (!collection)
  {
    return false;
  }
  
  if (s_currentCollection)
  {
    MPMusicPlaybackState playbackState = [s_musicPlayer playbackState];
    NSTimeInterval currentPlaybackTime = [s_musicPlayer currentPlaybackTime];
    MPMediaItem *nowPlayingItem = [s_musicPlayer nowPlayingItem];
    
    NSMutableArray *currentItems = [[s_currentCollection items] mutableCopy];
    NSArray *newItems = [collection items];
    
    [currentItems addObjectsFromArray:newItems];
    s_currentCollection = [MPMediaItemCollection collectionWithItems:(NSArray *) currentItems];
    [s_musicPlayer setQueueWithItemCollection:s_currentCollection];
    
    [s_musicPlayer setCurrentPlaybackTime:currentPlaybackTime];
    [s_musicPlayer setNowPlayingItem:nowPlayingItem];
    
    if (playbackState == MPMusicPlaybackStatePlaying)
    {
      audio_music_play ();
    }
  }
  else
  {
    s_currentCollection = collection;
    
    [s_musicPlayer setQueueWithItemCollection:s_currentCollection];
    
    audio_music_play ();
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_picker (bool show, audio_music_picked_callback fn)
{
  if (show)
  {
    if (!s_pickerActive)
    {
#if USE_MUSIC_PICKER_POP_UP
      UIView *parentView = s_rootViewController.view;
      
      float viewPosX = parentView.bounds.origin.x;
      float viewPosY = parentView.bounds.origin.y;
      float viewWidth  = parentView.bounds.size.width;
      float viewHeight = parentView.bounds.size.height;
      
      float width = 800.0f;
      float height = 600.0f;
      float posX = viewPosX + ((viewWidth - width) * 0.5f);
      float posY = viewPosY + ((viewHeight - height) * 0.5f);
      
      [s_musicPopOver setPassthroughViews:[NSArray arrayWithObject:parentView]];
      [s_musicPopOver presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
#else
      [s_musicPicker setModalTransitionStyle:UIModalTransitionStyleFlipHorizontal];
      [s_musicPicker setModalPresentationStyle:UIModalPresentationFormSheet];
      [s_rootViewController presentViewController:s_musicPicker animated:YES completion:nil];
#endif
      
      s_pickerActive = true;
      s_pickerCallback = fn;
    }
  }
  else
  {
    if (s_pickerActive)
    {
#if USE_MUSIC_PICKER_POP_UP
      [s_musicPopOver dismissPopoverAnimated:YES];
#else
      [s_rootViewController dismissViewControllerAnimated:YES completion:nil];
#endif
      
      s_pickerActive = false;
      
      if (s_pickerCallback)
      {
        s_pickerCallback ();
        s_pickerCallback = NULL;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_playback_state_changed (void)
{
  audio_music_notification notification = AUDIO_MUSIC_NOTIFICATION_UNKNOWN;
  
  MPMusicPlaybackState playbackState = [s_musicPlayer playbackState];
  
  switch (playbackState) 
  {
    case MPMusicPlaybackStatePaused: 
    { 
      CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStatePaused"); 
      notification = AUDIO_MUSIC_NOTIFICATION_PAUSED;
      break; 
    }
      
    case MPMusicPlaybackStateStopped: 
    { 
      CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateStopped"); 
      notification = AUDIO_MUSIC_NOTIFICATION_STOPPED;
      
#if PLAYBACK_STATE_HACK_FIX
      s_musicPlaying = false;
#endif
      break; 
    }
      
    case MPMusicPlaybackStateInterrupted: 
    { 
      CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateInterrupted"); 
      notification = AUDIO_MUSIC_NOTIFICATION_INTERRUPTED;
      
#if PLAYBACK_STATE_HACK_FIX
      s_musicPlaying = false;
#endif
      
      break; 
    }
      
    case MPMusicPlaybackStatePlaying: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStatePlaying"); break; }
    case MPMusicPlaybackStateSeekingForward: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateSeekingForward"); break; }
    case MPMusicPlaybackStateSeekingBackward: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateSeekingBackward"); break; }

    default: { break; }
  }
  
  cx_list2_node *node = s_musicNotificationCallbacks.head;
  
  while (node)
  {
    audio_music_notification_callback fn = node->data;
    
    CX_ASSERT (fn);
   
    fn (notification);
    
    node = node->next;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_now_playing_state_changed (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

