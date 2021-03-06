//
//  ui.c
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ui.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_add_intrinsic (ui_context_t *ctx, const ui_intrinsic_t *intr);
static void ui_ctx_remove_intrinsic (ui_context_t *ctx, const ui_intrinsic_t *intr);
static void ui_ctx_render (ui_context_t *ctx);
static void ui_ctx_press (ui_context_t *ctx, const cx_vec2 *point);
static ui_intrinsic_t *ui_ctx_input_hit (ui_context_t *ctx, const cx_vec2 *point, bool ext);
static ui_widget_state_t ui_ctx_widget_state (ui_context_t *ctx, const ui_intrinsic_t *intr);

static void ui_ctx_render_custom (ui_context_t *ctx, ui_custom_t *custom);
static void ui_ctx_render_button (ui_context_t *ctx, ui_button_t *button);
static void ui_ctx_render_checkbox (ui_context_t *ctx, ui_checkbox_t *checkbox);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_context_t *ui_init (float width, float height)
{
  ui_context_t *ctx = (ui_context_t *) cx_malloc (sizeof (ui_context_t));
  
  memset (ctx, 0, sizeof (ui_context_t));
  
  ctx->canvasWidth = width;
  ctx->canvasHeight = height;
  ctx->hover = NULL;
  ctx->focus = NULL;
  ctx->font = NULL;

  cx_list2_init (&ctx->intrList);

  return ctx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_deinit (ui_context_t *ctx)
{
  CX_FATAL_ASSERT (ctx);
  
  cx_list2_deinit (&ctx->intrList);
  
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
      ui_intrinsic_t *hit = ui_ctx_input_hit (ctx, &tevent->point, false);
      
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
#if 1
      if (ctx->hover)
      {
        ui_intrinsic_t *hit = ui_ctx_input_hit (ctx, &tevent->point, false);
        
        if (hit)
        {
          if (hit != ctx->focus)
          {
            ctx->hover = hit;
            
            handled = true;
          }
        }
      }
#else
      if (ctx->hover)
      {
        handled = true;
      }
#endif
      break;
    }
      
    case INPUT_TOUCH_TYPE_END:
    {
      ui_intrinsic_t *hit = ui_ctx_input_hit (ctx, &tevent->point, true);
      
      if (ctx->hover && (ctx->hover == hit))
      {
        if (hit != ctx->focus)
        {
          ctx->focus = hit;
          
          cx_vec2 point;
          
          point.x = ctx->canvasWidth * tevent->point.x;
          point.y = ctx->canvasHeight * tevent->point.y;
          
          ui_ctx_press (ctx, &point);          
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

void ui_clear_focus (ui_context_t *ctx)
{
  CX_FATAL_ASSERT (ctx);
  
  ctx->hover = NULL;
  ctx->focus = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_button_t *ui_button_create (ui_context_t *ctx, int uid)
{
  CX_FATAL_ASSERT (ctx);
  
  ui_button_t *button = (ui_button_t *) cx_malloc (sizeof (ui_button_t));
  
  memset (button, 0, sizeof (ui_button_t));
  
  button->intr.uid = uid;
  button->intr.wtype = UI_WIDGET_BUTTON;
  button->intr._widget = button;
  button->intr.opacity = 1.0f;
  button->intr.show = 1;
  button->intr.enable = 1;
  
  ui_ctx_add_intrinsic (ctx, &button->intr);
  
  return button;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_custom_t *ui_custom_create (ui_context_t *ctx, int uid)
{
  CX_FATAL_ASSERT (ctx);
  
  ui_custom_t *custom = (ui_custom_t *) cx_malloc (sizeof (ui_custom_t));
  
  memset (custom, 0, sizeof (ui_custom_t));
  
  custom->intr.uid = uid;
  custom->intr.wtype = UI_WIDGET_CUSTOM;
  custom->intr._widget = custom;
  custom->intr.opacity = 1.0f;
  custom->intr.show = 1;
  custom->intr.enable = 1;
  
  ui_ctx_add_intrinsic (ctx, &custom->intr);
  
  return custom;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_custom_destroy (ui_context_t *ctx, ui_custom_t *custom)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (custom);
  
  ui_ctx_remove_intrinsic (ctx, &custom->intr);
  
  if (custom->_callbacks)
  {
    cx_free (custom->_callbacks);
  }
  
  cx_free (custom);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  
  cx_list2_insert_front (&ctx->intrList, intr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_remove_intrinsic (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  
  cx_list2_remove (&ctx->intrList, intr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctx_press (ui_context_t *ctx, const cx_vec2 *point)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (ctx->focus);
  
  switch (ctx->focus->wtype) 
  {
    case UI_WIDGET_CUSTOM:
    {
      ui_custom_t *custom = (ui_custom_t *) ctx->focus->_widget;
      ui_custom_callbacks_t *callbacks = (ui_custom_callbacks_t *) custom->_callbacks;
      
      if (callbacks && callbacks->pressFn)
      {
        callbacks->pressFn (custom, point);
      }
      
      break;
    }
      
    case UI_WIDGET_BUTTON:
    {
      ui_button_t *button = (ui_button_t *) ctx->focus->_widget;
      ui_button_callbacks_t *callbacks = (ui_button_callbacks_t *) button->_callbacks;
      
      if (callbacks && callbacks->pressFn)
      {
        callbacks->pressFn (button, point);
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
  cx_list2_node *intrNode = ctx->intrList.head;
  
  while (intrNode)
  {
    ui_intrinsic_t *intr = (ui_intrinsic_t *) intrNode->data;
    CX_ASSERT (intr);
    
    switch (intr->wtype)
    {
      case UI_WIDGET_CUSTOM:    { ui_ctx_render_custom (ctx, intr->_widget); break; }
      case UI_WIDGET_BUTTON:    { ui_ctx_render_button (ctx, intr->_widget); break; }
      case UI_WIDGET_CHECKBOX:  { ui_ctx_render_checkbox (ctx, intr->_widget); break; }
      default:                  { break; }
    }
    
    intrNode = intrNode->next;
  }
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
    if (custom->intr.show)
    {  
      callbacks->renderFn (custom);
    }
  }
  else
  {
    // do default render
    
    if (custom->intr.show)
    {    
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
    if (button->intr.show)
    {   
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

static ui_intrinsic_t *ui_ctx_input_hit (ui_context_t *ctx, const cx_vec2 *point, bool ext)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (point);
  
  ui_intrinsic_t *hit = NULL;

  cx_list2_node *intrNode = ctx->intrList.tail;
  
  float tx = ctx->canvasWidth * point->x;
  float ty = ctx->canvasHeight * point->y;
  
  while (intrNode)
  {
    ui_intrinsic_t *intr = (ui_intrinsic_t *) intrNode->data;
    CX_ASSERT (intr);
    
    if (intr->enable)
    {    
      float w = intr->dimension.x;
      float h = intr->dimension.y;
      float x = intr->position.x;
      float y = intr->position.y;
      
      if (ext && (ctx->hover == intr))
      {
        const float ex = 4.0f;
        const float ey = 6.0f;
        
        w += ex;
        h += ey;
        x -= (ex * 0.5f);
        y -= (ey * 0.5f);
      }
      
      if ((tx >= x) && (tx <= (x + w)))
      {
        if ((ty >= y) && (ty <= (y + h)))
        {
          hit = intr;
          break;
        }
      }
    }
    
    intrNode = intrNode->prev;
  }
  
  return hit;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_widget_focus_set (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  
  ctx->hover = NULL;
  ctx->focus = intr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ui_widget_state_t _ui_intrinsic_widget_state_get (ui_context_t *ctx, const ui_intrinsic_t *intr)
{
  CX_FATAL_ASSERT (ctx);
  CX_ASSERT (intr);
  
  return ui_ctx_widget_state (ctx, intr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

float _ui_intrinsic_opacity_get (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr);
  
  return intr->opacity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_vec2 *_ui_intrinsic_position_get (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr);
  
  return &intr->position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_vec2 *_ui_intrinsic_dimension_get (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr);
  
  return &intr->dimension;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_colour *_ui_intrinsic_colour_get (const ui_intrinsic_t *intr, ui_widget_state_t wstate)
{
  CX_ASSERT (intr);
  CX_ASSERT ((wstate >= UI_WIDGET_STATE_NORMAL) && (wstate < NUM_UI_WIDGET_STATES));
  
  return &intr->colour [wstate];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _ui_intrinsic_show_get (const ui_intrinsic_t *intr)
{
  CX_ASSERT (intr);
  
  return intr->show;
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

void _ui_intrinsic_dimension_set (ui_intrinsic_t *intr, float w, float h)
{
  CX_ASSERT (intr);
  
  cx_vec2_set (&intr->dimension, w, h);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_opacity_set (ui_intrinsic_t *intr, float opacity)
{
  CX_ASSERT (intr);
  
  intr->opacity = opacity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_show_set (ui_intrinsic_t *intr, bool show)
{
  CX_ASSERT (intr);
  
  intr->show = show;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _ui_intrinsic_enable_set (ui_intrinsic_t *intr, bool enable)
{
  CX_ASSERT (intr);
  
  intr->enable = enable;
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
    
    cx_font_render (font, elem->text, tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &elem->fgColour);
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
