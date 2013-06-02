//
//  audio.c
//  earthnews
//
//  Created by Ubaka Onyechi on 12/01/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#import "audio.h"
#import "util.h"
#import <MediaPlayer/MediaPlayer.h>
#import <AudioToolbox/AudioToolbox.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG_ENABLE 1
#define USE_MUSIC_PICKER_POP_UP 1 // ipad only
#define PLAYBACK_STATE_HACK_FIX 1
#define SCREEN_FADE_OPACITY (0.6f)
#define SCREEN_FADE_DURATION (0.5f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SYSTEM_VERSION_GREATER_THAN(v)              ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedDescending)
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicPickerDelegate : UIViewController<MPMediaPickerControllerDelegate>
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicNotifcation : NSObject
- (void) handleNowPlayingItemChanged:(NSNotification *)notification;
- (void) handlePlaybackStateChanged:(NSNotification *)notification;
- (void) handleVolumeChanged:(NSNotification *)notification;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface MusicPickerPopoverBackground : UIPopoverBackgroundView
@property (nonatomic, readwrite) UIPopoverArrowDirection arrowDirection;
@property (nonatomic, readwrite) CGFloat arrowOffset;
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool g_initialised = false;
static bool g_pickerActive = false;
static UIViewController *g_rootViewCtrlr = nil;
static MPMusicPlayerController *g_musicPlayer = nil;
static MPMediaPickerController *g_musicPicker = nil;
static MusicPickerDelegate *g_musicPickerDelegate = nil;
static MusicNotifcation *g_musicNotification = nil;
static MPMediaItemCollection *g_currentCollection = nil;
static audio_music_picked_callback g_pickerCallback = NULL;
static cx_list2 g_musicNotificationCallbacks;
#if USE_MUSIC_PICKER_POP_UP
static UIPopoverController *g_musicPopOver = nil;
#endif
#if PLAYBACK_STATE_HACK_FIX
static bool g_musicPlaying = false;
#endif

SystemSoundID g_sndfxIds [NUM_AUDIO_SOUNDFX];
const char *g_sndfxPaths [NUM_AUDIO_SOUNDFX] =
{
  "data/soundfx/beep-23.mp3",    // AUDIO_SOUNDFX_CLICK0
  "data/soundfx/button-46.mp3",  // AUDIO_SOUNDFX_CLICK1
  "data/soundfx/button-50.mp3",  // AUDIO_SOUNDFX_CLICK2
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_sndfx_init (void);
static void audio_sndfx_deinit (void);
static void audio_music_init (void);
static void audio_music_deinit (void);
static bool audio_music_update_queue (MPMediaItemCollection *collection);
static void audio_music_picker (bool show, audio_music_picked_callback fn);
static void audio_music_playback_state_changed (void);
static void audio_music_now_playing_state_changed (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_init (const void *rootvc)
{
  CX_ASSERT (!g_initialised);
  CX_ASSERT (rootvc);
  
  g_rootViewCtrlr = (UIViewController *) rootvc;
  
  audio_sndfx_init ();
  
  audio_music_init ();
  
  g_initialised = true;
  
  return g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_deinit (void)
{
  CX_ASSERT (g_initialised);
  
  audio_sndfx_deinit ();
  
  audio_music_deinit ();
  
  g_rootViewCtrlr = nil;
  
  g_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_soundfx_play (audio_soundfx_t sndfx)
{
  CX_ASSERT ((sndfx > AUDIO_SOUNDFX_INVALID) && (sndfx < NUM_AUDIO_SOUNDFX));
  
  SystemSoundID sndId = g_sndfxIds [sndfx];
  
  AudioServicesPlaySystemSound (sndId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_notification_register (audio_music_notification_callback fn)
{
  CX_ASSERT (fn);
  CX_ASSERT (!cx_list2_exists (&g_musicNotificationCallbacks, fn));
  
  cx_list2_insert_back (&g_musicNotificationCallbacks, fn);
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
  CX_ASSERT (g_musicPlayer);
  
  [g_musicPlayer play];
  
#if PLAYBACK_STATE_HACK_FIX
  g_musicPlaying = true;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_pause (void)
{
  CX_ASSERT (g_musicPlayer);
  
  [g_musicPlayer pause];
  
#if PLAYBACK_STATE_HACK_FIX
  g_musicPlaying = false;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_next (void)
{
  CX_ASSERT (g_musicPlayer);
  
  [g_musicPlayer skipToNextItem];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void audio_music_prev (void)
{
  CX_ASSERT (g_musicPlayer);
  
  [g_musicPlayer skipToPreviousItem];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_playing (void)
{
  CX_ASSERT (g_musicPlayer);
  
  bool ret = false;

  MPMediaItem *nowPlayingItem = [g_musicPlayer nowPlayingItem];
  
#if PLAYBACK_STATE_HACK_FIX
  ret = nowPlayingItem && g_musicPlaying;
#else

  MPMusicPlaybackState playbackState = [g_musicPlayer playbackState];

  ret = (nowPlayingItem && (playbackState == MPMusicPlaybackStatePlaying))
#endif
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_picker_active (void)
{
  return g_pickerActive;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_music_queued (void)
{
  return g_currentCollection ? true : false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int audio_music_get_track_id (char *buffer, int bufferlen)
{
  CX_ASSERT (g_musicPlayer);
  CX_ASSERT (buffer);
  CX_ASSERT (bufferlen > 0);
  
  int size = 0;
  
  MPMediaItem *nowPlayingItem = [g_musicPlayer nowPlayingItem];
  
  if (nowPlayingItem)
  {
    NSString *artist = [nowPlayingItem valueForProperty:MPMediaItemPropertyArtist];
    NSString *title = [nowPlayingItem valueForProperty:MPMediaItemPropertyTitle];
    
    const char *a = [artist cStringUsingEncoding:NSUTF8StringEncoding];
    const char *t = [title cStringUsingEncoding:NSUTF8StringEncoding];
    
    size = cx_sprintf (buffer, bufferlen, "%s - %s", a, t);
  }
  
  return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_sndfx_init (void)
{  
  for (int i = 0; i < NUM_AUDIO_SOUNDFX; ++i)
  {
    const char *path = g_sndfxPaths [i];
    NSString *filepath = [NSString stringWithCString:path encoding:NSASCIIStringEncoding];
    
    NSURL *fileURL = [[NSBundle mainBundle] URLForResource:filepath withExtension:nil];
    CX_ASSERT (fileURL != nil);
    
    SystemSoundID sndId = 0;
#if __has_feature(objc_arc)
    OSStatus error = AudioServicesCreateSystemSoundID ((__bridge CFURLRef) fileURL, &sndId);
#else
    OSStatus error = AudioServicesCreateSystemSoundID ((CFURLRef) fileURL, &sndId);    
#endif
    CX_ASSERT (error == kAudioServicesNoError);
    CX_REF_UNUSED (error);
    
    g_sndfxIds [i] = sndId;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_sndfx_deinit (void)
{
  for (int i = 0; i < NUM_AUDIO_SOUNDFX; ++i)
  {
    SystemSoundID sndId = g_sndfxIds [i];
    AudioServicesDisposeSystemSoundID (sndId);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_init (void)
{
#if TARGET_IPHONE_SIMULATOR
  return;
#endif
  
  cx_list2_init (&g_musicNotificationCallbacks);
  
  g_musicPlayer = [MPMusicPlayerController applicationMusicPlayer];
  
  g_musicNotification = [[MusicNotifcation alloc] init];
  
  g_musicPickerDelegate = [[MusicPickerDelegate alloc] init];
  
  g_musicPicker = [[MPMediaPickerController alloc] initWithMediaTypes:MPMediaTypeAnyAudio];
  
#if USE_MUSIC_PICKER_POP_UP
  g_musicPopOver = [[UIPopoverController alloc] initWithContentViewController:g_musicPicker];
#endif
  
  [g_musicPicker setDelegate:g_musicPickerDelegate];
  [g_musicPicker setAllowsPickingMultipleItems:YES];
  [g_musicPicker setTitle:@""];
  [g_musicPicker setModalInPopover:YES];
  
  //[g_musicPicker setPrompt:@"Queue songs for playback"];
  
  if (SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO (@"6.0"))
  {
    [g_musicPicker setShowsCloudItems:YES];
  }
  
#if USE_MUSIC_PICKER_POP_UP && 1
  if (SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO (@"5.0"))
  {
    [g_musicPopOver setPopoverBackgroundViewClass:[MusicPickerPopoverBackground class]];
  }
#endif
  
  NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
  
  [notificationCenter addObserver:g_musicNotification 
                         selector:@selector (handleNowPlayingItemChanged:) 
                             name:MPMusicPlayerControllerNowPlayingItemDidChangeNotification 
                           object:g_musicPlayer];
  
  [notificationCenter addObserver:g_musicNotification 
                         selector:@selector (handlePlaybackStateChanged:) 
                             name:MPMusicPlayerControllerPlaybackStateDidChangeNotification 
                           object:g_musicPlayer];

  [notificationCenter addObserver:g_musicNotification 
                         selector:@selector (handleVolumeChanged:) 
                             name:MPMusicPlayerControllerVolumeDidChangeNotification 
                           object:g_musicPlayer];
  
  [g_musicPlayer beginGeneratingPlaybackNotifications];
  [g_musicPlayer setRepeatMode:MPMusicRepeatModeAll];
  [g_musicPlayer setShuffleMode:MPMusicShuffleModeOff];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_deinit (void)
{
#if TARGET_IPHONE_SIMULATOR
  return;
#endif
  
  NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
  
  [notificationCenter removeObserver:g_musicNotification 
                             name:MPMusicPlayerControllerNowPlayingItemDidChangeNotification 
                           object:g_musicPlayer];
  
  [notificationCenter removeObserver:g_musicNotification 
                             name:MPMusicPlayerControllerPlaybackStateDidChangeNotification 
                           object:g_musicPlayer];
  
  [notificationCenter removeObserver:g_musicNotification 
                                name:MPMusicPlayerControllerVolumeDidChangeNotification 
                              object:g_musicPlayer];
  
  [g_musicPlayer endGeneratingPlaybackNotifications];
  
#if USE_MUSIC_PICKER_POP_UP
  [g_musicPopOver release];
#endif
  
  [g_musicPicker release];
  
  [g_musicPickerDelegate release];
  
  [g_musicNotification release];
  
  cx_list2_deinit (&g_musicNotificationCallbacks);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool audio_music_update_queue (MPMediaItemCollection *collection)
{
#if TARGET_IPHONE_SIMULATOR
  return false;
#endif
  
  if (!collection)
  {
    return false;
  }
  
  if (g_currentCollection)
  {
    MPMusicPlaybackState playbackState = [g_musicPlayer playbackState];
    NSTimeInterval currentPlaybackTime = [g_musicPlayer currentPlaybackTime];
    MPMediaItem *nowPlayingItem = [g_musicPlayer nowPlayingItem];
    
    NSMutableArray *currentItems = [[g_currentCollection items] mutableCopy];
    NSArray *newItems = [collection items];
    
    [currentItems addObjectsFromArray:newItems];
    
    g_currentCollection = [MPMediaItemCollection collectionWithItems:(NSArray *) currentItems];
    [g_musicPlayer setQueueWithItemCollection:g_currentCollection];
    
    [g_musicPlayer setCurrentPlaybackTime:currentPlaybackTime];
    [g_musicPlayer setNowPlayingItem:nowPlayingItem];
    
    if (playbackState == MPMusicPlaybackStatePlaying)
    {
      audio_music_play ();
    }
  }
  else
  {
    g_currentCollection = collection;
    
    [g_musicPlayer setQueueWithItemCollection:g_currentCollection];
    
    audio_music_play ();
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_picker (bool show, audio_music_picked_callback fn)
{
#if TARGET_IPHONE_SIMULATOR
  return;
#endif
  
  if (show)
  {
    if (!g_pickerActive)
    {
#if USE_MUSIC_PICKER_POP_UP
      UIView *parentView = g_rootViewCtrlr.view;
      
      float viewPosX = parentView.bounds.origin.x;
      float viewPosY = parentView.bounds.origin.y;
      float viewWidth  = parentView.bounds.size.width;
      float viewHeight = parentView.bounds.size.height;
      
      float width = 800.0f;
      float height = 600.0f;
      float posX = viewPosX + ((viewWidth - width) * 0.5f);
      float posY = viewPosY + ((viewHeight - height) * 0.5f);
      
      [g_musicPopOver setPassthroughViews:[NSArray arrayWithObject:parentView]];
      [g_musicPopOver presentPopoverFromRect:CGRectMake(posX, posY, width, height) inView:parentView permittedArrowDirections:0 animated:YES];
#else
      [g_musicPicker setModalTransitionStyle:UIModalTransitionStyleFlipHorizontal];
      [g_musicPicker setModalPresentationStyle:UIModalPresentationFormSheet];
      [g_rootViewCtrlr presentViewController:g_musicPicker animated:YES completion:nil];
#endif
      
      g_pickerActive = true;
      g_pickerCallback = fn;
      
      util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, SCREEN_FADE_OPACITY, SCREEN_FADE_DURATION, NULL, NULL);
    }
  }
  else
  {
    if (g_pickerActive)
    {
#if USE_MUSIC_PICKER_POP_UP
      [g_musicPopOver dismissPopoverAnimated:YES];
#else
      [g_rootViewCtrlr dismissViewControllerAnimated:YES completion:nil];
#endif
      
      g_pickerActive = false;
      
      if (g_pickerCallback)
      {
        g_pickerCallback ();
        g_pickerCallback = NULL;
      }
      
      util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, SCREEN_FADE_OPACITY, SCREEN_FADE_DURATION, NULL, NULL);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void audio_music_playback_state_changed (void)
{
  audio_music_notification_t notification = AUDIO_MUSIC_NOTIFICATION_UNKNOWN;
  
  MPMusicPlaybackState playbackState = [g_musicPlayer playbackState];
  
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
      g_musicPlaying = false;
#endif
      break; 
    }
      
    case MPMusicPlaybackStateInterrupted: 
    { 
      CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateInterrupted"); 
      notification = AUDIO_MUSIC_NOTIFICATION_INTERRUPTED;
      
#if PLAYBACK_STATE_HACK_FIX
      g_musicPlaying = false;
#endif
      
      break; 
    }
      
    case MPMusicPlaybackStatePlaying: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStatePlaying"); break; }
    case MPMusicPlaybackStateSeekingForward: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateSeekingForward"); break; }
    case MPMusicPlaybackStateSeekingBackward: { CX_DEBUGLOG_CONSOLE (DEBUG_LOG_ENABLE, "MPMusicPlaybackStateSeekingBackward"); break; }

    default: { break; }
  }
  
  cx_list2_node *node = g_musicNotificationCallbacks.head;
  
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MusicPickerDelegate

- (void)mediaPicker:(MPMediaPickerController *)mediaPicker didPickMediaItems:(MPMediaItemCollection *)mediaItemCollection
{
  audio_music_update_queue (mediaItemCollection);
  
  audio_music_picker (false, NULL);
}

- (void)mediaPickerDidCancel:(MPMediaPickerController *)mediaPicker
{
  audio_music_picker (false, NULL);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MusicNotifcation

- (void)handleNowPlayingItemChanged:(NSNotification *)notification
{
  audio_music_now_playing_state_changed ();
}

- (void)handlePlaybackStateChanged:(NSNotification *)notification
{
  audio_music_playback_state_changed ();
}

- (void)handleVolumeChanged:(NSNotification *)notification
{
  CX_DEBUG_BREAKABLE_EXPR;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MusicPickerPopoverBackground

@synthesize arrowOffset, arrowDirection;

- (id) initWithFrame:(CGRect)frame
{
  if (self = [super initWithFrame:frame])
  {
    self.backgroundColor = [UIColor colorWithWhite:0.2f alpha:0.7f];
    self.arrowDirection = 0;
    self.arrowOffset = 0.0f;
  }
  
  return self;
}

+ (UIEdgeInsets)contentViewInsets
{
  return UIEdgeInsetsMake (0.0f, 0.0f, 1.0f, 0.0f);
}

+ (CGFloat)arrowHeight
{
  return 0.0f;
}

+ (CGFloat)arrowBase
{
  return 0.0f;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
