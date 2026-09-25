#include <pebble.h>
#define DELTA 13

static Layer *loading_bg_layer;
static BitmapLayer *loading_text_layer;
static GDrawCommandImage *loading_bg_draw;
static GBitmap *loading_text_draw;

static void render_loading_vec(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect(0, 0, 200, 228), 0, GCornerNone);

    gdraw_command_image_draw(ctx, loading_bg_draw, GPoint(0, 0));
}

static void render_loading_text(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
}

void HN_GetWin_Map(Window *window)
{
    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    loading_bg_draw = gdraw_command_image_create_with_resource(RESOURCE_ID_LOADING_ANI);
    if (loading_bg_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load loading vec!");
        return;
    }
    loading_text_draw = gbitmap_create_with_resource(RESOURCE_ID_LOADING_TEXT);
    if (loading_text_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load loading text!");
        return;
    }

    loading_bg_layer = layer_create(GRect(0, 0, 200, 228));
    layer_set_update_proc(loading_bg_layer, render_loading_vec);
    loading_text_layer = bitmap_layer_create(GRect(0,170,200,21));
    bitmap_layer_set_bitmap(loading_text_layer, loading_text_draw);
    bitmap_layer_set_alignment(loading_text_layer, GAlignCenter);
    bitmap_layer_set_compositing_mode(loading_text_layer, GCompOpSet);

    layer_add_child(wlayer, loading_bg_layer);
    layer_add_child(wlayer, bitmap_layer_get_layer(loading_text_layer));
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
}