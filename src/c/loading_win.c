#include <pebble.h>

#include "helldivers-war.h"

#define DELTA 30

static Layer *loading_bg_layer;
static BitmapLayer *loading_text_layer;
static GDrawCommandImage *loading_bg_draw;
static GDrawCommandSequence *loading_bg_animated_draw;
static GBitmap *loading_text_draw;
static AppTimer *bgTimer;
static int aniIndex;
static Window *this;
static AppTimer *timeout_timer;
static bool reverse_animation = false;
static bool gave_up = false;

static void render_loading_vec(Layer *layer, GContext *ctx)
{

    gdraw_command_image_draw(ctx, loading_bg_draw, GPoint(0, 0));
}

static void next_frame_handler(void *context)
{
    // Draw the next frame
    layer_mark_dirty(loading_bg_layer);

    // Continue the sequence
    bgTimer = app_timer_register(DELTA, next_frame_handler, NULL);
}

static void render_loading_vec_animated(Layer *layer, GContext *ctx)
{

    // draw bg
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect(0, 0, 200, 228), 0, GCornerNone);

    GRect bounds = layer_get_bounds(layer);
    GSize seq_bounds = gdraw_command_sequence_get_bounds_size(loading_bg_animated_draw);

    // Get the next frame
    GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(loading_bg_animated_draw, aniIndex);

    // If another frame was found, draw it
    if (frame)
    {
        gdraw_command_frame_draw(ctx, loading_bg_animated_draw, frame, GPoint((bounds.size.w - seq_bounds.w) / 2, (bounds.size.h - seq_bounds.h) / 2));
    }

    // Advance to the next frame, wrapping if neccessary
    int num_frames = gdraw_command_sequence_get_num_frames(loading_bg_animated_draw);
    if (!reverse_animation)
        aniIndex++;
    else
        aniIndex--;
    if (aniIndex == num_frames)
    {
        aniIndex = num_frames - 1;
    }
    if (aniIndex < 0)
    {
        if (gave_up)
        {
            HN_SwitchWin(&HN_FAILED_CONNECT_WIN, true);
        }
        aniIndex = 0;
    }
}

static void render_loading_text(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
}

void HN_GetWin_Loading(Window *window)
{
    this = window;
    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    loading_bg_draw = gdraw_command_image_create_with_resource(RESOURCE_ID_LOADING_BG);
    if (loading_bg_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load loading vec!");
        return;
    }
    loading_bg_animated_draw = gdraw_command_sequence_create_with_resource(RESOURCE_ID_LOADING_ANI);
    if (loading_bg_animated_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load loading sequence!");
        return;
    }
    loading_text_draw = gbitmap_create_with_resource(RESOURCE_ID_LOADING_TEXT);
    if (loading_text_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load loading text!");
        return;
    }

    loading_bg_layer = layer_create(wbounds);
    // layer_set_update_proc(loading_bg_layer, render_loading_vec);
    layer_set_update_proc(loading_bg_layer, render_loading_vec_animated);

    loading_text_layer = bitmap_layer_create(GRect(0, 170, 200, 21));
    bitmap_layer_set_bitmap(loading_text_layer, loading_text_draw);
    bitmap_layer_set_alignment(loading_text_layer, GAlignCenter);
    bitmap_layer_set_compositing_mode(loading_text_layer, GCompOpSet);

    layer_add_child(wlayer, loading_bg_layer);
    layer_add_child(wlayer, bitmap_layer_get_layer(loading_text_layer));

    bgTimer = app_timer_register(DELTA, next_frame_handler, NULL);
}

static void timeout(void *context)
{
    reverse_animation = true;
    gave_up = true;
}

void HN_ReadyWin_Loading()
{
    timeout_timer = app_timer_register(10 * 1000, timeout, NULL);
}

void HN_Win_Loading_JSReady()
{
    if (gave_up)
        return;
    app_timer_reschedule(timeout_timer, 10 * 1000);
    DictionaryIterator *pingReq;

    AppMessageResult resBegin = app_message_outbox_begin(&pingReq);
    if (resBegin == APP_MSG_OK)
    {
        int val = 1;
        dict_write_int(pingReq, MESSAGE_KEY_HBPing, &val, sizeof(int), true);

        app_message_outbox_send();
        APP_LOG(APP_LOG_LEVEL_ERROR, "Sent Helldivers Bot ping req...");
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open outbox! %d", resBegin);
        window_stack_pop_all(true);
    }
}

/// @brief Called when the window is removed
void HN_DesWin_Loading(Window *window)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Removing loading win");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer is: %s", timeout_timer);
    layer_destroy(loading_bg_layer);
    bitmap_layer_destroy(loading_text_layer);
    gdraw_command_image_destroy(loading_bg_draw);
    gbitmap_destroy(loading_text_draw);
    gdraw_command_sequence_destroy(loading_bg_animated_draw);
    window_destroy(this);
    gave_up = true;
}