#include <pebble.h>
#define DELTA 100

static Layer *loading_bg_layer;
static BitmapLayer *loading_text_layer;
static GDrawCommandImage *loading_bg_draw;
static GDrawCommandSequence *loading_bg_animated_draw;
static GBitmap *loading_text_draw;
static AppTimer *bgTimer;
static int aniIndex;

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
    aniIndex++;
    if (aniIndex == num_frames)
    {
        aniIndex = num_frames-1;
    }
}

static void render_loading_text(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
}

void HN_GetWin_Map(Window *window)
{
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

    loading_bg_layer = layer_create(GRect(0, 0, 200, 228));
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

void HN_ReadyWin_Map()
{
}

/// @brief Called when the window is removed
void HN_DesWin_Map(Window *window)
{
    layer_destroy(loading_bg_layer);
    bitmap_layer_destroy(loading_text_layer);
    gdraw_command_image_destroy(loading_bg_draw);
    gbitmap_destroy(loading_text_draw);
    gdraw_command_sequence_destroy(loading_bg_animated_draw);
}