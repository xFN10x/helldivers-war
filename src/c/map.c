#include <pebble.h>

static Layer *map_layer;
static GDrawCommandImage *map_layer_draw;

static void render_image(Layer *layer, GContext *ctx)
{
    gdraw_command_image_draw(ctx, map_layer_draw, GPoint(0, 0));
}

void HN_GetWin_Map(Window *window)
{
    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    map_layer_draw = gdraw_command_image_create_with_resource(RESOURCE_ID_GALACTIC_MAP);

    map_layer = layer_create(GRect(0, 0, 200, 200));
    layer_set_update_proc(map_layer, render_image);
    layer_add_child(wlayer, map_layer);
}

/// @brief Called when the window is removed
void HN_DesWin_Map(Window *window)
{
}