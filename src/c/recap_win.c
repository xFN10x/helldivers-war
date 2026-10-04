#include <pebble.h>

#include "storage.h"

static Layer *map_layer;
static GDrawCommandImage *map_image;
static AppTimer *map_check_timer;

static Layer *foreground_layer;

static Window *this;

static bool showing_set = false;
static struct HN_MapData showing;

static int16_t sector_offsets[34][2];
static bool offsets_set = false;
static int16_t sector_offset_x = 0;
static int16_t sector_offset_y = 0;

static float __current_sector_offset_x = 0;
static float __current_sector_offset_y = 0;

// Source - https://stackoverflow.com/a/4353537
// Posted by aioobe, modified by community. See post 'Timeline' for change history
// Retrieved 2026-10-03, License - CC BY-SA 4.0

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

// set this when sector_offset is 0,0
static bool update_sector_offsets(GDrawCommand *command, uint32_t index, void *context)
{
    if (gdraw_command_get_type(command) == GDrawCommandTypeInvalid)
        return true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sector %i is at...", index);
    uint16_t points_num = gdraw_command_get_num_points(command);
    int16_t xposes[points_num];
    int16_t yposes[points_num];
    for (size_t i = 0; i < points_num; i++)
    {
        GPoint point = gdraw_command_get_point(command, i);
        if (gdraw_command_get_type(command) == GDrawCommandTypeCircle)
        {
            xposes[i] = point.x;
            yposes[i] = point.y;
        }
        else
        {
            xposes[i] = (float)point.x / 8.0f;
            yposes[i] = (float)point.y / 8.0f;
        }
    }
    int16_t xsum = 0;
    int16_t ysum = 0;

    for (size_t i = 0; i < points_num; i++)
    {
        xsum += xposes[i];
    }

    for (size_t i = 0; i < points_num; i++)
    {
        ysum += yposes[i];
    }

    int16_t avgx = xsum / points_num;
    int16_t avgy = ysum / points_num;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "---------------------");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "sum of xs: %i", xsum);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "median is: %i", avgx);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "first point is: %i", xposes[0]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "---------------------");

    sector_offsets[index][0] = avgx;
    sector_offsets[index][1] = avgy;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sector %i is at: %i, %i", index, avgx, avgy);
    return true;
}

static bool update_map_commands(GDrawCommand *command, uint32_t index, void *context)
{
    // index 0-10 are bug sectors 1-11
    // index 11-21 are cyborg
    // index 22-32 are illuminate
    // index 33 is super earth
    if (gdraw_command_get_type(command) == GDrawCommandTypeInvalid)
        return true;
    bool log = true;
    if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "--------------");
    uint16_t current_front;
    if (index < 11)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is bugs");
        }
        current_front = showing.bugs;
    }
    else if (index < 22)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is bots");
        }
        current_front = showing.bots;
    }
    else if (index < 33)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is illum");
        }
        current_front = showing.illum;
    }
    else
    {
        // this is super earth
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is OUR HOME");
        gdraw_command_set_fill_color(command, showing.superEarthPoints >= showing.superEarthMax ? GColorArmyGreen : GColorRed);
        return true;
    }
    if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Front data: %u (%u)", current_front, ((current_front >> 1) << 1));

    int sector = (index % 11) + 1;
    if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Index is: %u, sector is: %u", index, sector);

    uint16_t mask = (0b1111111111111111 << 4) << (12 - sector);
    if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Mask is: %u", mask);

    bool hasSector = (((current_front >> 1) << 1) & mask) == mask;
    if (hasSector && log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Sector %i is taken", sector);
    else if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Sector %i is not taken", sector);
    gdraw_command_set_fill_color(command, hasSector ? GColorArmyGreen : GColorDarkGray);
    gdraw_command_set_stroke_color(command, hasSector ? GColorYellow : GColorLightGray);
    return true;
}

static void render_map(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    GDrawCommandList *commands = gdraw_command_image_get_command_list(map_image);
    if (!offsets_set)
    {
        gdraw_command_list_iterate(commands, update_sector_offsets, NULL);
        offsets_set = true;
    }
    if (!showing_set)
    {
        gdraw_command_list_iterate(commands, update_map_commands, NULL);
        showing_set = true;
    }
    gdraw_command_list_draw(ctx, commands);
}

static void set_sector_offsets()
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting offsets");
    GDrawCommandList *commands = gdraw_command_image_get_command_list(map_image);
    gdraw_command_list_iterate(commands, update_sector_offsets, NULL);
}

static int selected = 33;

float scale = 1;
static void up_pressed(ClickRecognizerRef recognizer, void *context)
{
    selected += 1;
    if (selected > 33)
        selected = 0;
    if (selected < 0)
        selected = 33;

    sector_offset_x = sector_offsets[selected][0];
    sector_offset_y = sector_offsets[selected][1];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "up %i (%i, %i)", selected, sector_offset_x, sector_offset_y);
}

static void down_pressed(ClickRecognizerRef recognizer, void *context)
{
    selected -= 1;
    if (selected > 33)
        selected = 0;
    if (selected < 0)
        selected = 33;

    sector_offset_x = sector_offsets[selected][0];
    sector_offset_y = sector_offsets[selected][1];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "down %i (%i, %i)", selected, sector_offset_x, sector_offset_y);
}

static void check_map_updated(void *data)
{
    if (__current_sector_offset_x != sector_offset_x || __current_sector_offset_y != sector_offset_y)
    {
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "different pos");
        if (__current_sector_offset_x != sector_offset_x)
        {
            __current_sector_offset_x = lerp(__current_sector_offset_x, sector_offset_x, 0.5f);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "Moved map x to: %i (%i)", (int)__current_sector_offset_x, sector_offset_x);
        }

        if (__current_sector_offset_y != sector_offset_y)
        {
            __current_sector_offset_y = lerp(__current_sector_offset_y, sector_offset_y, 0.5f);
        }
    }
    GSize bounds = gdraw_command_image_get_bounds_size(map_image);
    // APP_LOG(APP_LOG_LEVEL_INFO, "Map image size is %ix%i", bounds.w, bounds.h);
    layer_set_frame(map_layer, GRect((-__current_sector_offset_x) + 100, (-__current_sector_offset_y) + 114, bounds.w, bounds.h));
    map_check_timer = app_timer_register(3, check_map_updated, NULL);
}

void HN_ClickProvWin_Recap(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_pressed);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_pressed);

    window_single_repeating_click_subscribe(BUTTON_ID_UP, 5,
                                            up_pressed);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 5,
                                            down_pressed);
    // window_single_click_subscribe(BUTTON_ID_BACK, cls);
}

void HN_GetWin_Recap(Window *window)
{
    this = window;
    showing = HN_MAPDATA_TEST;

    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    map_image = gdraw_command_image_create_with_resource(RESOURCE_ID_MAP);

    map_layer = layer_create(wbounds);
    layer_set_update_proc(map_layer, render_map);

    layer_add_child(wlayer, map_layer);
    map_check_timer = app_timer_register(3, check_map_updated, NULL);
}

void HN_ReadyWin_Recap()
{
}

/// @brief Called when the window is removed
void HN_DesWin_Recap(Window *window)
{
    gdraw_command_image_destroy(map_image);
    layer_destroy(map_layer);
}