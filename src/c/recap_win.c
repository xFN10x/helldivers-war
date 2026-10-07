#include <pebble.h>

#include "storage.h"
#include "helldivers-war.h"
#include "messages.h"
#include "recap_win.h"

static Layer *map_layer;
static GDrawCommandImage *map_image;
static AppTimer *map_check_timer;
static GSize map_bounds;

static Layer *foreground_layer;
static GDrawCommandImage *foreground_image;
static Animation *foreground_animation;

static BitmapLayer *faction_icon_bg_layer;
static GBitmap *bugs_icon_bitmap;
static GBitmap *bots_icon_bitmap;
static GBitmap *illuminate_icon_bitmap;
static GBitmap *superearth_icon_bitmap;

static Window *this;

static bool showing_set = false;
static HN_MapData *showing;
static HN_MapData subject;

static int selected_sector = 33;
static int16_t sector_offsets[34][2];
static bool offsets_set = false;
static int16_t sector_offset_x = 0;
static int16_t sector_offset_y = 0;

static float __current_sector_offset_x = -60;
static float __current_sector_offset_y = -60;

static AppTimer *recap_timer;

static struct RecapTimerData bugRecapTimerData;
static struct RecapTimerData botRecapTimerData;
static struct RecapTimerData illuminateRecapTimerData;
static uint16_t bugsRecapArray[13];
static uint16_t botsRecapArray[13];
static uint16_t illumRecapArray[13];

// Source - https://stackoverflow.com/a/4353537
// Posted by aioobe, modified by community. See post 'Timeline' for change history
// Retrieved 2026-10-03, License - CC BY-SA 4.0

static float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

static size_t get_interp_data(uint16_t start, uint16_t end, uint16_t *output)
{
    // 0b1111 1000 0000 0000 start
    // 0b1110 0000 0000 0000 end
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Geting interp of start and end of recap, start: %u, end: %u", start, end);
    start = (start >> 1) << 1;
    end = (end >> 1) << 1;
    bool reved = false;
    if (start <= end)
    {
        uint16_t acstart = start;
        uint16_t acen = end;
        start = acen;
        end = acstart;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "SWAPPED, start: %u, end: %u", start, end);
        reved = true;
    }

    int i = 0;
    while (start >= end)
    {
        output[i] = start;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Map start: %u (%i)", start, i);
        i++;
        if (start << 1 == start)
            break;
        start = (start << 1);
    }
    if (reved)
    {
        uint16_t reversed[13];
        for (int i2 = 0; i2 < i; i2++)
        {
            int newI = (i - 1) - i2;
            reversed[newI] = output[i2];
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Swapped: %u (%i) -> %u (%i)", output[i2], i2, reversed[newI], newI);
        }
        memcpy(output, reversed, sizeof(uint16_t) * 12);
    }

    return i;
}

// set this when sector_offset is 0,0
static bool update_sector_offsets(GDrawCommand *command, uint32_t index, void *context)
{
    if (gdraw_command_get_type(command) == GDrawCommandTypeInvalid)
        return true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sector %i is at...", index);
    uint16_t points_num = gdraw_command_get_num_points(command);
    if (points_num == 0)
        return true;
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

    /*APP_LOG(APP_LOG_LEVEL_DEBUG, "---------------------");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "sum of xs: %i", xsum);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "median is: %i", avgx);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "first point is: %i", xposes[0]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "---------------------");*/
    if (index > 33)
    {
        APP_LOG(APP_LOG_LEVEL_WARNING, "More than 34 lines!");
        return false;
    }
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
    bool log = false;
    if (log)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "--------------");
    uint16_t current_front;
    if (index < 11)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is bugs");
        }
        current_front = showing->bugs;
    }
    else if (index < 22)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is bots");
        }
        current_front = showing->bots;
    }
    else if (index < 33)
    {
        if (log)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is illum");
        }
        current_front = showing->illum;
    }
    else
    {
        // this is super earth
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Front is OUR HOME");
        gdraw_command_set_fill_color(command, showing->superEarthPoints >= showing->superEarthMax ? GColorArmyGreen : GColorRed);
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

static void render_foreground(Layer *layer, GContext *ctx)
{
    gdraw_command_image_draw(ctx, foreground_image, GPoint(0, 0));
}

static void change_sector(int sec)
{
    sec--;
    if (sec > 33)
        sec = 33;
    if (sec < 0)
        sec = 33;
    selected_sector = sec;
    sector_offset_x = sector_offsets[selected_sector][0];
    sector_offset_y = sector_offsets[selected_sector][1];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "changed sector %i (%i, %i)", selected_sector, sector_offset_x, sector_offset_y);

    if (selected_sector < 11)
    {
        // bug
        bitmap_layer_set_background_color(faction_icon_bg_layer, GColorWindsorTan);
        bitmap_layer_set_bitmap(faction_icon_bg_layer, bugs_icon_bitmap);
    }
    else if (selected_sector < 22)
    {
        // cyborg
        bitmap_layer_set_background_color(faction_icon_bg_layer, GColorBulgarianRose);
        bitmap_layer_set_bitmap(faction_icon_bg_layer, bots_icon_bitmap);
    }
    else if (selected_sector < 33)
    {
        // illum
        bitmap_layer_set_background_color(faction_icon_bg_layer, GColorPictonBlue);
        bitmap_layer_set_bitmap(faction_icon_bg_layer, illuminate_icon_bitmap);
    }
    else
    {
        bitmap_layer_set_background_color(faction_icon_bg_layer, GColorCobaltBlue);
        bitmap_layer_set_bitmap(faction_icon_bg_layer, superearth_icon_bitmap);
    }
}

static void render_map(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    GDrawCommandList *commands = gdraw_command_image_get_command_list(map_image);
    if (!offsets_set)
    {
        gdraw_command_list_iterate(commands, update_sector_offsets, NULL);
        offsets_set = true;
        change_sector(33);
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

/*float scale = 1;*/
static void up_pressed(ClickRecognizerRef recognizer, void *context)
{
    change_sector(selected_sector + 1);
}

/*static void down_pressed(ClickRecognizerRef recognizer, void *context)
{
    selected_sector -= 1;
    if (selected_sector > 33)
        selected_sector = 0;
    if (selected_sector < 0)
        selected_sector = 33;

    sector_offset_x = sector_offsets[selected_sector][0];
    sector_offset_y = sector_offsets[selected_sector][1];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "down %i (%i, %i)", selected_sector, sector_offset_x, sector_offset_y);
}*/

static void check_map_updated(void *data)
{
    bool log = false;
    if (__current_sector_offset_x != sector_offset_x || __current_sector_offset_y != sector_offset_y)

        if (log)
            APP_LOG(APP_LOG_LEVEL_DEBUG, "different pos");
    if (__current_sector_offset_x != sector_offset_x)
    {
        __current_sector_offset_x = lerp(__current_sector_offset_x, sector_offset_x, 0.5f);
        if (log)
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Moved map x to: %i (%i)", (int)__current_sector_offset_x, sector_offset_x);
    }

    if (__current_sector_offset_y != sector_offset_y)
    {
        __current_sector_offset_y = lerp(__current_sector_offset_y, sector_offset_y, 0.5f);
        if (log)
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Moved map y to: %i (%i)", (int)__current_sector_offset_y, sector_offset_y);
    }

    // APP_LOG(APP_LOG_LEVEL_INFO, "Map image size is %ix%i", bounds.w, bounds.h);
    layer_set_frame(map_layer, GRect((-__current_sector_offset_x) + 100, (-__current_sector_offset_y) + 100, map_bounds.w, map_bounds.h));
    map_check_timer = app_timer_register(3, check_map_updated, NULL);
}

void HN_ClickProvWin_Recap(void *context)
{
    // window_single_click_subscribe(BUTTON_ID_SELECT, up_pressed);
    //  window_single_click_subscribe(BUTTON_ID_DOWN, down_pressed);
    //   window_single_click_subscribe(BUTTON_ID_BACK, cls);
}

void HN_GetWin_Recap(Window *window)
{
    this = window;
    HN_MapData *test = NULL;
    if (persist_exists(HN_STORKEY_MAPCACHE))
    {
        persist_read_data(HN_STORKEY_MAPCACHE, test, sizeof(HN_MapData));
        showing = test;
    }
    else
        showing = &HN_MAPDATA_START;

    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    map_image = gdraw_command_image_create_with_resource(RESOURCE_ID_MAP);
    map_bounds = gdraw_command_image_get_bounds_size(map_image);
    map_layer = layer_create(wbounds);
    layer_set_update_proc(map_layer, render_map);

    foreground_image = gdraw_command_image_create_with_resource(RESOURCE_ID_RECAP_FORE);
    foreground_layer = layer_create(GRect(0, 100, 200, 228));
    layer_set_update_proc(foreground_layer, render_foreground);

    GRect start = GRect(0, 100, 200, 228);
    GRect end = GRect(0, 20, 200, 228);

    PropertyAnimation *ani = property_animation_create_layer_frame(foreground_layer, &start, &end);
    foreground_animation = property_animation_get_animation(ani);

    animation_set_curve(foreground_animation, AnimationCurveEaseOut);
    animation_set_delay(foreground_animation, 2000);
    animation_set_duration(foreground_animation, 250);

    bugs_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BUGS_ICON);
    bots_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BOTS_ICON);
    illuminate_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ILLUMINATES_ICON);
    superearth_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUPEREARTH_ICON);

    faction_icon_bg_layer = bitmap_layer_create(GRect(0, 0, 200, 228));
    bitmap_layer_set_compositing_mode(faction_icon_bg_layer, GCompOpSet);

    map_check_timer = app_timer_register(3, check_map_updated, NULL);

    layer_add_child(wlayer, bitmap_layer_get_layer(faction_icon_bg_layer));
    layer_add_child(wlayer, map_layer);
    layer_add_child(wlayer, foreground_layer);
}

void HN_ReadyWin_Recap()
{
    animation_schedule(foreground_animation);

    APP_LOG(APP_LOG_LEVEL_INFO, "Getting map data...");
    DictionaryIterator *mapUpdatedMsg = HN_StartMsg();

    int in = 1;
    dict_write_int(mapUpdatedMsg, MESSAGE_KEY_HNGetCurrentMap, &in, sizeof(int), true);
    HN_SendMsg(mapUpdatedMsg);
}

int getSectorByData(uint16_t data)
{
    //removes the attacking bit, and then limits the max to 11 sectors, because this isn't counter super earth as the 0th
    data = (data >> 1) << 1;
    data = data & ~0b0000000000010000;
    for (int i = 11; i >= 0; i--)
    {
        uint16_t mask = (0b1111111111100000 << i);
        if (!(data ^ mask))
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Data: %u is sector: %u", data, 11 - i);
            return 11 - i;
        }
    }

    return -1;
}

static void change_showing(HN_MapData *change)
{
    showing = change;

    showing_set = false;
}

void show_recap_for_front(void *data)
{
    RecapTimerData *timerData = (RecapTimerData *)data;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "index: %zu, len: %zu", timerData->index, timerData->length);
    if (timerData->index >= timerData->length)
    {
        if (timerData->next != NULL)
            recap_timer = app_timer_register(2000, show_recap_for_front, timerData->next);
        return;
    }
    uint16_t mapData = timerData->array[timerData->index];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "-- Showing recap... index: %zu + %zu, data: %u", timerData->index,timerData->sector_offset, mapData);
    int sector = getSectorByData(mapData);
    //if (timerData->index >=12) sector = 34;
    if (sector != 34)
        sector += timerData->sector_offset;
    change_sector(sector);

    *(timerData->sector_data_to_change) = mapData;
    showing_set = false;

    timerData->index += 1;
    recap_timer = app_timer_register(2000, show_recap_for_front, data);
}

void HN_DoRecap(HN_MapData *end)
{

    size_t bugsarrylen = get_interp_data(showing->bugs, end->bugs, bugsRecapArray);
    size_t botsarrylen = get_interp_data(showing->bots, end->bots, botsRecapArray);
    size_t illumarrylen = get_interp_data(showing->illum, end->illum, illumRecapArray);
    /*size_t bugsarrylen = get_interp_data(0b1111111111110000, 0, bugsRecapArray);
    size_t botsarrylen = get_interp_data(0b1111111111110000, 0, botsRecapArray);
    size_t illumarrylen = get_interp_data(0b1111111111110000, 0, illumRecapArray);*/

    illuminateRecapTimerData = (RecapTimerData){
        .index = 0,
        .length = illumarrylen,
        .array = illumRecapArray,
        .capitals = HN_illumPlanetNames,
        .regions = HN_illumRegionNames,
        .sector_offset = 22,
        .sector_data_to_change = &showing->illum,
        .next = NULL};

    botRecapTimerData = (RecapTimerData){
        .index = 0,
        .length = botsarrylen,
        .array = botsRecapArray,
        .capitals = HN_botPlanetNames,
        .regions = HN_botRegionNames,
        .sector_offset = 11,
        .sector_data_to_change = &showing->bots,
        .next = &illuminateRecapTimerData};

    bugRecapTimerData = (RecapTimerData){
        .index = 0,
        .length = bugsarrylen,
        .array = bugsRecapArray,
        .capitals = HN_bugPlanetNames,
        .regions = HN_bugRegionNames,
        .sector_offset = 0,
        .sector_data_to_change = &showing->bugs,
        .next = &botRecapTimerData};

    recap_timer = app_timer_register(2000, show_recap_for_front, &bugRecapTimerData);
}

/// @brief Called when the window is removed
void HN_DesWin_Recap(Window *window)
{
    layer_destroy(map_layer);
    gdraw_command_image_destroy(map_image);

    layer_destroy(foreground_layer);
    gdraw_command_image_destroy(foreground_image);
    if (foreground_animation)
    animation_destroy(foreground_animation);

    bitmap_layer_destroy(faction_icon_bg_layer);
    gbitmap_destroy(bugs_icon_bitmap);
    gbitmap_destroy(bots_icon_bitmap);
    gbitmap_destroy(illuminate_icon_bitmap);
    gbitmap_destroy(superearth_icon_bitmap);
}