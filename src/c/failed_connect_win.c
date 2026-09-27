#include <pebble.h>

#include "helldivers-war.h"

static Window *this;
static Layer *bg;
static TextLayer *text;

static void drawBg(struct Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorRed);
    GRect bounds = layer_get_bounds(layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BG size: w%d h%d", bounds.size.w, bounds.size.h);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void HN_GetWin_FailedConnc(Window *window)
{
    this = window;
    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    text = text_layer_create(GRect(0,104,200,20));
    text_layer_set_text(text, "CONNECTION FAILED");
    text_layer_set_text_alignment(text, GTextAlignmentCenter);
    text_layer_set_overflow_mode(text, GTextOverflowModeWordWrap);
    text_layer_set_font(text, HN_Font1);
    text_layer_set_background_color(text, GColorRed);
    text_layer_set_text_color(text, GColorBlack);

    bg = layer_create(wbounds);
    layer_set_update_proc(bg, drawBg);

    layer_add_child(wlayer, bg);
    layer_add_child(wlayer, text_layer_get_layer(text));
}

static void cls(void *context)
{
    window_stack_pop_all(true);
}

void HN_ReadyWin_FailedConnc()
{
    app_timer_register(5000, cls, NULL);
}

/// @brief Called when the window is removed
void HN_DesWin_FailedConnc(Window *window)
{
    layer_destroy(bg);
    window_destroy(this);
}