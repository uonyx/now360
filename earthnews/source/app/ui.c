//
//  ui.c
//  earthnews
//
//  Created by Ubaka Onyechi on 23/12/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ui.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_add_intrinsic (ui_context_t *ctx, const ui_intrinsic_t *intr);
static void ui_ctx_render (ui_context_t *ctx);
static void ui_ctx_press (ui_context_t *ctx, ui_intrinsic_t *intr);
static ui_intrinsic_t *ui_ctx_input_hit (ui_context_t *ctx, const cx_vec2 *point);
static ui_widget_state_t ui_ctx_widget_state (ui_context_t *ctx, const ui_intrinsic_t *intr);

static void ui_ctx_render_custom (ui_context_t *ctx, ui_custom_t *custom);
static void ui_ctx_render_button (ui_context_t *ctx, ui_button_t *button);
static void ui_ctx_render_checkbox (ui_context_t *ctx, ui_checkbox_t *checkbox);

static ui_button_t *ui_button (const ui_intrinsic_t *intr);
static ui_custom_t *ui_custom (const ui_intrinsic_t *intr);
static ui_checkbox_t *ui_checkbox (const ui_intrinsic_t *intr);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_context_t *ui_init (float width, float height)
{
  ui_context_t *ctx = (ui_context_t *) cx_malloc (sizeof (ui_context_t));
  
  ctx->canvasWidth = width;
  ctx->canvasHeight = height;

  return ctx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_deinit (ui_context_t *ctx)
{
  CX_FATAL_ASSERT (ctx);
  cx_free (ctx);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_canvas_resize (ui_context_t *ctx, float width, float height)
{
  ctx->canvasWidth = width;
  ctx->canvasHeight = height;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ui_input (ui_context_t *ctx, const input_touch_event *tevent)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (tevent);
  
  bool handled = false;
  
  switch (tevent->type)
  {
    case INPUT_TOUCH_TYPE_BEGIN:
    {
      ui_intrinsic_t *hit = ui_ctx_input_hit (ctx, &tevent->point);
      
      if (hit) 
      {
        if (hit != ctx->focus)
        {
          ctx->hover = hit;
        }
      }
      else
      {
        // disable focus if not interacting with the UI
        ctx->hover = NULL;
        ctx->focus = NULL;
      }
      
      break;
    }
      
    case INPUT_TOUCH_TYPE_MOVE:
    {
      if (ctx->hover)
      {
        handled = true;
      }
      
      break;
    }
      
    case INPUT_TOUCH_TYPE_END:
    {
      ui_intrinsic_t *hit = ui_ctx_input_hit (ctx, &tevent->point);
      
      if (ctx->hover && (ctx->hover == hit))
      {
        if (hit != ctx->focus)
        {
          ctx->focus = hit;
          ui_ctx_press (ctx, hit);          
        }
        
        handled = true;
      }
      
      ctx->hover = NULL;
      
      break;
    }
      
    default:
    {
      break;
    }
  }
  
  return handled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render (ui_context_t *ctx)
{
  CX_FATAL_ASSERT (ctx);
  
  ui_ctx_render (ctx);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_button_t *ui_button_create (ui_context_t *ctx, int uid)
{
  CX_FATAL_ASSERT (ctx);
  
  ui_button_t *button = (ui_button_t *) cx_malloc (sizeof (ui_button_t));
  
  button->intr.uid = uid;
  button->intr.wtype = UI_WIDGET_BUTTON;
  button->intr._widget = button;
  
  ui_ctx_add_intrinsic (ctx, &button->intr);
  
  return button;
}

ui_custom_t *ui_custom_create (ui_context_t *ctx, int uid)
{
  CX_FATAL_ASSERT (ctx);
  
  ui_custom_t *custom = (ui_custom_t *) cx_malloc (sizeof (ui_custom_t));
  
  custom->intr.uid = uid;
  custom->intr.wtype = UI_WIDGET_CUSTOM;
  custom->intr._widget = custom;
  
  ui_ctx_add_intrinsic (ctx, &custom->intr);
  
  return custom;
}

void ui_button_set_callbacks (ui_button_t *button, const ui_button_callbacks_t *callbacks)
{
  CX_ASSERT (button);
  
  ui_button_callbacks_t *cb = NULL;
  
  if (!button->_callbacks)
  {
    cb = (ui_button_callbacks_t *) cx_malloc (sizeof (ui_button_callbacks_t));
    button->_callbacks = cb;
  }
  else
  {
    cb = (ui_button_callbacks_t *) button->_callbacks;
  }
  
  *cb = *callbacks;
}

void ui_custom_set_callbacks (ui_custom_t *custom, const ui_custom_callbacks_t *callbacks)
{
  CX_ASSERT (custom);
  
  ui_custom_callbacks_t *cb = NULL;
  
  if (!custom->_callbacks)
  {
    cb = (ui_custom_callbacks_t *) cx_malloc (sizeof (ui_custom_callbacks_t));
    custom->_callbacks = cb;
  }
  else
  {
    cb = (ui_custom_callbacks_t *) custom->_callbacks;
  }
  
  *cb = *callbacks;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static ui_button_t *ui_button (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr->wtype == UI_WIDGET_BUTTON);
  CX_ASSERT (intr->_widget);
  
  return (ui_button_t *) intr->_widget;
}

static ui_custom_t *ui_custom (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr->wtype == UI_WIDGET_CUSTOM);
  CX_ASSERT (intr->_widget);
  
  return (ui_custom_t *) intr->_widget;
}

static ui_checkbox_t *ui_checkbox (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr->wtype == UI_WIDGET_CHECKBOX);
  CX_ASSERT (intr->_widget);
  
  return (ui_checkbox_t *) intr->_widget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static ui_widget_state_t ui_ctx_widget_state (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  
  ui_widget_state_t state;
  
  if (intr == ctx->hover)
  {
    state = UI_WIDGET_STATE_HOVER;
  }
  else if (intr == ctx->focus)
  {
    state = UI_WIDGET_STATE_FOCUS;
  }
  else
  {
    state = UI_WIDGET_STATE_NORMAL;
  }
  
  return state;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_add_intrinsic (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  
  ctx->intrList = cx_list_insert (ctx->intrList, intr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_press (ui_context_t *ctx, ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  CX_ASSERT (ctx->focus);
  
  switch (ctx->focus->wtype) 
  {
    case UI_WIDGET_CUSTOM:
    {
      ui_custom_t *custom = ui_custom (intr);
      ui_custom_callbacks_t *callbacks = (ui_custom_callbacks_t *) custom->_callbacks;
      
      if (callbacks && callbacks->pressFn)
      {
        callbacks->pressFn (custom);
      }
      
      break;
    }
      
    case UI_WIDGET_BUTTON:
    {
      ui_button_t *button = ui_button (intr);
      ui_button_callbacks_t *callbacks = (ui_button_callbacks_t *) button->_callbacks;
      
      if (callbacks && callbacks->pressFn)
      {
        callbacks->pressFn (button);
      }
      
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

static void ui_ctx_render (ui_context_t *ctx)
{
  cx_list_node *intrNode = ctx->intrList;
  
  while (intrNode)
  {
    ui_intrinsic_t *intr = (ui_intrinsic_t *) intrNode->data;
    CX_ASSERT (intr);
    
    switch (intr->wtype)
    {
      case UI_WIDGET_CUSTOM:    { ui_ctx_render_custom (ctx, ui_custom (intr)); break; }
      case UI_WIDGET_BUTTON:    { ui_ctx_render_button (ctx, ui_button (intr)); break; }
      case UI_WIDGET_CHECKBOX:  { ui_ctx_render_checkbox (ctx, ui_checkbox (intr)); break; }
      default:                  { break; }
    }
    
    intrNode = intrNode->next;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static ui_intrinsic_t *ui_ctx_input_hit (ui_context_t *ctx, const cx_vec2 *point)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (point);
  
  ui_intrinsic_t *hit = NULL;
  
  cx_list_node *intrNode = ctx->intrList;
  
  float tx = ctx->canvasWidth * point->x;
  float ty = ctx->canvasHeight * point->y;
  
  while (intrNode)
  {
    ui_intrinsic_t *intr = (ui_intrinsic_t *) intrNode->data;
    CX_ASSERT (intr);
    
    float w = intr->dimension.x;
    float h = intr->dimension.y;
    float x = intr->position.x;
    float y = intr->position.y;
    
    if ((tx >= x) && (tx <= (x + w)))
    {
      if ((ty >= y) && (ty <= (y + h)))
      {
        hit = intr;
        break;
      }
    }
    
    intrNode = intrNode->next;
  }
  
#if 0
  if (hit && (hit->wtype == UI_WIDGET_LIST))
  {
    ui_list_t *list = ui_list (hit);
    
    for (unsigned int i = 0; i < list->itemCount; ++i)
    {
      ui_list_item_t *listItem = &list->items [i];
      
      float w = listItem->intr.dimension.x;
      float h = listItem->intr.dimension.y;
      float x = listItem->intr.position.x;
      float y = listItem->intr.position.y;
      
      if ((tx >= x) && (tx <= (x + w)))
      {
        if ((ty >= y) && (ty <= (y + h)))
        {
          hit = &listItem->intr;
          list->selectedItem = (int) i;          
          break;
        }
      }
    }
  }
#endif
  
  return hit;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_render_custom (ui_context_t *ctx, ui_custom_t *custom)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (custom);
  
  ui_custom_callbacks_t *callbacks = (ui_custom_callbacks_t *) custom->_callbacks;
  
  if (callbacks && callbacks->renderFn)
  {
    callbacks->renderFn (custom);
  }
  else
  {
    
    // do default render
    
    ui_widget_state_t wstate = ui_ctx_widget_state (ctx, &custom->intr);
    
    float x1 = custom->intr.position.x;
    float y1 = custom->intr.position.y;
    float x2 = x1 + custom->intr.dimension.x;
    float y2 = y1 + custom->intr.dimension.y;
    
    cx_texture *texture = custom->intr.texture [wstate];
    cx_colour colour = custom->intr.colour [wstate];
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colour, texture);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_render_button (ui_context_t *ctx, ui_button_t *button)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (button);
  
  ui_button_callbacks_t *callbacks = (ui_button_callbacks_t *) button->_callbacks;
  
  if (callbacks && callbacks->renderFn)
  {
    callbacks->renderFn (button);
  }
  else
  {
    // do default render
    
    ui_widget_state_t wstate = ui_ctx_widget_state (ctx, &button->intr);
    
    float x1 = button->intr.position.x;
    float y1 = button->intr.position.y;
    float x2 = x1 + button->intr.dimension.x;
    float y2 = y1 + button->intr.dimension.y;
    
    cx_texture *texture = button->intr.texture [wstate];
    cx_colour colour = button->intr.colour [wstate];
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colour, texture);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_render_checkbox (ui_context_t *ctx, ui_checkbox_t *checkbox)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (checkbox);
  
  ui_checkbox_callbacks_t *callbacks = (ui_checkbox_callbacks_t *) checkbox->_callbacks;
  
  if (callbacks && callbacks->renderFn)
  {
    callbacks->renderFn (checkbox);
  }
  else
  {
    // default render
    
    ui_widget_state_t wstate = ui_ctx_widget_state (ctx, &checkbox->intr);
    
    if (checkbox->checked)
    {
    }
    else
    {
    }
    
    float x1 = checkbox->intr.position.x;
    float y1 = checkbox->intr.position.y;
    float x2 = x1 + checkbox->intr.dimension.x;
    float y2 = y1 + checkbox->intr.dimension.y;
    
    cx_texture *texture = checkbox->intr.texture [wstate];
    cx_colour colour = checkbox->intr.colour [wstate];
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colour, texture);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_widget_state_t _ui_intrinsic_widget_state (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  
  return ui_ctx_widget_state (ctx, intr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_colour_set (ui_intrinsic_t *intr, ui_widget_state_t wstate, const cx_colour *colour)
{
  CX_ASSERT (intr);
  CX_ASSERT (colour);
  CX_ASSERT ((wstate >= UI_WIDGET_STATE_NORMAL) && (wstate < NUM_UI_WIDGET_STATES));
  
  intr->colour [wstate] = *colour;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_texture_set (ui_intrinsic_t *intr, ui_widget_state_t wstate, cx_texture *texture)
{
  CX_ASSERT (intr);
  CX_ASSERT (texture);
  CX_ASSERT ((wstate >= UI_WIDGET_STATE_NORMAL) && (wstate < NUM_UI_WIDGET_STATES));
  
  intr->texture [wstate] = texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_position_set (ui_intrinsic_t *intr, float x, float y)
{
  CX_ASSERT (intr);
  
  cx_vec2_set (&intr->position, x, y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
bool ui_touch_hit (ui_context_t *context, const ui_intrinsic_t *elem);
bool ui_touch_hit (ui_context_t *context, const ui_intrinsic_t *elem)
{
  CX_ASSERT (elem);
  
  bool ret = false;
  
  float w = intr->dimension.x;
  float h = intr->dimension.y;
  float x = intr->position.x;
  float y = intr->position.y;
  
  float tx = ctx->canvasWidth * ctx->inputEv.point.x;
  float ty = ctx->canvasHeight * ctx->inputEv.point.y;
  
  
  if ((tx >= x) && (tx <= (x + w)))
  {
    if ((ty >= y) && (ty <= (y + h)))
    {
      ret = true;
    }
  }
  
  return ret;
}

bool ui_input_element (const ui_intrinsic_t *elem, float inputx, float inputy)
{
  CX_ASSERT (elem);
  
  bool ret = false;
  
  float w = elem->dimension.x;
  float h = elem->dimension.y;
  float x = elem->position.x;
  float y = elem->position.y;
  
  if ((inputx >= x) && (inputx <= (x + w)))
  {
    if ((inputy >= y) && (inputy <= (y + h)))
    {
      ret = true;
    }
  }
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ui_do_button (ui_context_t *context, const ui_button_t *button)
{
  CX_ASSERT (context);
  CX_ASSERT (button);
  
  bool ret = false;
  
  bool isHot = (button->elem == context->hover);
  bool isActive = (button->elem == context->active);
  
  if (isHot)
  {
    // is touch event end, return true;
    
    if (context->event.type == INPUT_TOUCH_TYPE_END)
    {
      bool isInside = ui_touch_hit (context, button->elem);
      
      if (isInside)
      {
        context->active = button->elem;
        ret = true;
      }
    }
    
    // render hover skin
    ui_render_element (button->elem);
    
  }
  else if (isActive)
  {
    // render active skin
    ui_render_element (button->elem);
  }
  else
  {
    // is touch event begin, return true;
    if (context->event.type == INPUT_TOUCH_TYPE_BEGIN)
    {    
      // if touch event inside, set hover
      bool isInside = ui_touch_hit (context, button->elem);
      
      if (isInside)
      {
        context->hover = button->elem;
      }
    }
    
    // render normal skin
    ui_render_element (button->elem);
  }
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render_element (const ui_intrinsic_t *elem)
{
  CX_ASSERT (elem);
  
  float x1 = elem->position.x;
  float y1 = elem->position.y;
  float x2 = x1 + elem->dimension.x;
  float y2 = y1 + elem->dimension.y;
  
  cx_texture *texture = elem->texture [0];
  cx_colour colour = elem->colour [0];
  
  colour.a = elem->opacity;
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colour, texture);
  
#if 0
  if (elem->text && *elem->text)
  {
    float tx = 4.0f;
    float ty = 36.0f;
    
    cx_font_render (s_font[0], elem->text, tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &elem->fgColour);
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
