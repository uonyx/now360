//
//  ui.h
//  earthnews
//
//  Created by Ubaka Onyechi on 23/12/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EARTHNEWS_UI_H
#define EARTHNEWS_UI_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"
#include "input.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum ui_widget_t
{
  UI_WIDGET_INVALID,
  UI_WIDGET_BUTTON,
  UI_WIDGET_LIST,
  UI_WIDGET_CUSTOM,
  UI_WIDGET_CHECKBOX,
  NUM_UI_WIDGETS
} ui_widget_t;

typedef enum ui_widget_state_t
{
  UI_WIDGET_STATE_NORMAL,
  UI_WIDGET_STATE_HOVER,
  UI_WIDGET_STATE_FOCUS,
  NUM_UI_WIDGET_STATES,
} ui_widget_state_t;

typedef struct ui_intrinsic_t
{
  void *_widget;
  ui_widget_t wtype;
  int uid;
  cx_vec2 position;
  cx_vec2 dimension;
  cx_colour colour [NUM_UI_WIDGET_STATES];
  cx_texture *texture [NUM_UI_WIDGET_STATES];
  float opacity;
  unsigned int show : 1;
  unsigned int enable : 1;
} ui_intrinsic_t;

typedef struct ui_custom_t
{
  ui_intrinsic_t intr;
  const void *userdata;
  void *_callbacks;
} ui_custom_t;

typedef struct ui_button_t
{
  ui_intrinsic_t intr;
  const char *text;
  void *_callbacks;
} ui_button_t;

typedef struct ui_checkbox_t
{
  ui_intrinsic_t intr;
#if 0
  cx_colour colour;
  cx_texture *texture;
#endif
  bool checked;
  const char *text;
  void *_callbacks;
} ui_checkbox_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*ui_button_cb_pressed) (const ui_button_t *button);
typedef void (*ui_button_cb_render) (const ui_button_t *button);
typedef void (*ui_custom_cb_pressed) (ui_custom_t *custom);
typedef void (*ui_custom_cb_render) (ui_custom_t *custom);
typedef void (*ui_checkbox_cb_pressed) (const ui_checkbox_t *checkbox);
typedef void (*ui_checkbox_cb_render) (const ui_checkbox_t *checkbox);

typedef struct ui_custom_callbacks_t
{
  ui_custom_cb_pressed pressFn;
  ui_custom_cb_render  renderFn;
} ui_custom_callbacks_t;

typedef struct ui_button_callbacks_t
{
  ui_button_cb_pressed pressFn;
  ui_button_cb_render renderFn;
} ui_button_callbacks_t;

typedef struct ui_checkbox_callbacks_t
{
  ui_checkbox_cb_pressed pressFn;
  ui_checkbox_cb_render renderFn;
} ui_checkbox_callbacks_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SINGLY_LINKED_LIST 0

typedef struct ui_context_t
{
  float canvasWidth;
  float canvasHeight;
  const ui_intrinsic_t *hover;
  const ui_intrinsic_t *focus;
  cx_font *font;
#if SINGLY_LINKED_LIST
  cx_list *intrList;
#else
  cx_list2 intrList;
#endif
} ui_context_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_context_t *ui_init (float width, float height);
void ui_deinit (ui_context_t *ctx);
bool ui_input (ui_context_t *ctx, const input_touch_event *tevent);
void ui_render (ui_context_t *ctx);
void ui_canvas_resize (ui_context_t *ctx, float width, float height);
void ui_clear_focus (ui_context_t *ctx);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_button_t *ui_button_create (ui_context_t *ctx, int uid);
void ui_button_destroy (ui_context_t *ctx, ui_button_t *button);
void ui_button_set_callbacks (ui_button_t *button, const ui_button_callbacks_t *callbacks);

ui_custom_t *ui_custom_create (ui_context_t *ctx, int uid);
void ui_custom_destroy (ui_context_t *ctx, ui_custom_t *custom);
void ui_custom_set_callbacks (ui_custom_t *custom, const ui_custom_callbacks_t *callbacks);

ui_checkbox_t *ui_checkbox_create (ui_context_t *ctx, int uid);
void ui_checkbox_destroy (ui_checkbox_t *checkbox);
void ui_checkbox_set_callbacks (ui_checkbox_t *checkbox, const ui_checkbox_callbacks_t *callbacks);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ui_widget_set_focus(context, widget) _ui_intrinsic_widget_focus_set(context, &widget->intr)
#define ui_widget_get_state(context, widget) _ui_intrinsic_widget_state_get(context, &widget->intr)

#define ui_widget_get_visible(widget) _ui_intrinsic_show_get(&widget->intr)
#define ui_widget_get_opacity(widget) _ui_intrinsic_opacity_get(&widget->intr)
#define ui_widget_get_colour(widget, widget_state) _ui_intrinsic_colour_get(&widget->intr, widget_state)
#define ui_widget_get_position(widget) _ui_intrinsic_position_get(&widget->intr)
#define ui_widget_get_dimension(widget) _ui_intrinsic_dimension_get(&widget->intr)

#define ui_widget_set_enabled(widget, enable) _ui_intrinsic_enable_set(&widget->intr, enable)
#define ui_widget_set_visible(widget, visible) _ui_intrinsic_show_set(&widget->intr, visible)
#define ui_widget_set_opacity(widget, opacity) _ui_intrinsic_opacity_set(&widget->intr, opacity)
#define ui_widget_set_colour(widget, widget_state, colour) _ui_intrinsic_colour_set(&widget->intr, widget_state, colour)
#define ui_widget_set_position(widget, pos_x, pos_y) _ui_intrinsic_position_set(&widget->intr, pos_x, pos_y)
#define ui_widget_set_dimension(widget, width, height) _ui_intrinsic_dimension_set(&widget->intr, width, height)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_colour_set (ui_intrinsic_t *intr, ui_widget_state_t wstate, const cx_colour *colour);
void _ui_intrinsic_texture_set (ui_intrinsic_t *intr, ui_widget_state_t wstate, cx_texture *texture);
void _ui_intrinsic_position_set (ui_intrinsic_t *intr, float x, float y);
void _ui_intrinsic_dimension_set (ui_intrinsic_t *intr, float w, float h);
void _ui_intrinsic_opacity_set (ui_intrinsic_t *intr, float opacity);
void _ui_intrinsic_show_set (ui_intrinsic_t *intr, bool show);
void _ui_intrinsic_enable_set (ui_intrinsic_t *intr, bool enable);

const cx_colour *_ui_intrinsic_colour_get (const ui_intrinsic_t *intr, ui_widget_state_t wstate);
const cx_vec2 *_ui_intrinsic_position_get (const ui_intrinsic_t *intr);
const cx_vec2 *_ui_intrinsic_dimension_get (const ui_intrinsic_t *intr);
float _ui_intrinsic_opacity_get (const ui_intrinsic_t *intr);
bool _ui_intrinsic_show_get (const ui_intrinsic_t *intr);

ui_widget_state_t _ui_intrinsic_widget_state_get (ui_context_t *ctx, const ui_intrinsic_t *intr);
void _ui_intrinsic_widget_focus_set (ui_context_t *ctx, const ui_intrinsic_t *intr);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

