//
//  app.h
//  earthnews
//
//  Created by Ubaka Onyechi on 02/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef EARTHNEWS_APP_H
#define EARTHNEWS_APP_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_init (int width, int height);
void app_deinit (void);
void app_update (void);
void app_render (void);

void app_input_touch_began (float x, float y);
void app_input_touch_moved (float x, float y, float px, float py);
void app_input_touch_ended (float x, float y);
void app_input_zoom (float factor);

void app_view_update (float deltaTime);


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
