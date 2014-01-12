//
//  audio.h
//  now360
//
//  Copyright (c) 2013 Ubaka Onyechi. All rights reserved.
//

#ifndef NOW360_AUDIO_H
#define NOW360_AUDIO_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  AUDIO_SOUNDFX_INVALID = -1,
  AUDIO_SOUNDFX_CLICK0,
  AUDIO_SOUNDFX_CLICK1,
  AUDIO_SOUNDFX_CLICK2,
  NUM_AUDIO_SOUNDFX,
} audio_soundfx_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum 
{
  AUDIO_MUSIC_NOTIFICATION_UNKNOWN,
  AUDIO_MUSIC_NOTIFICATION_INTERRUPTED,
  AUDIO_MUSIC_NOTIFICATION_PAUSED,
  AUDIO_MUSIC_NOTIFICATION_STOPPED,
} audio_music_notification_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*audio_music_picked_callback) (void);
typedef void (*audio_music_notification_callback) (audio_music_notification_t n);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool audio_init (const void *rootvc);
void audio_deinit (void);
void audio_soundfx_play (audio_soundfx_t sndfx);
void audio_music_notification_register (audio_music_notification_callback fn);
void audio_music_pick (audio_music_picked_callback fn);
void audio_music_play (void);
void audio_music_pause (void);
void audio_music_next (void);
void audio_music_prev (void);
bool audio_music_playing (void);
bool audio_music_paused (void);
bool audio_music_picker_active (void);
bool audio_music_queued (void);
int  audio_music_get_track_id (char *buffer, int bufferlen);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
