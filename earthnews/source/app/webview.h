//
//  webview.h
//  earthnews
//
//  Created by Ubaka Onyechi on 08/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef EARTHNEWS_WEBVIEW_H
#define EARTHNEWS_WEBVIEW_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool web_view_init (float screenWidth, float screenHeight);
bool web_view_deinit (void);
bool web_view_launch_url (const char *url);
void web_view_hide (bool hide);
bool web_view_is_shown (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif