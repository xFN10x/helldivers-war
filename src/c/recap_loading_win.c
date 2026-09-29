#include <pebble.h>

#include "helldivers-war.h"

#define DELTA 15

static Layer *animation_layer;
static TextLayer * text;
static GDrawCommandSequence *animation_animated_draw;
static AppTimer *aniTimer;
static int aniIndex;
static Window *this;

static void next_frame_handler(void *context)
{
    // Draw the next frame
    layer_mark_dirty(animation_layer);

    // Continue the sequence
    aniTimer = app_timer_register(DELTA, next_frame_handler, NULL);
}
static void render_ani_vec_animated(Layer *layer, GContext *ctx)
{

    // draw bg
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect(0, 0, 200, 228), 0, GCornerNone);

    GRect bounds = layer_get_bounds(layer);
    GSize seq_bounds = gdraw_command_sequence_get_bounds_size(animation_animated_draw);

    // Get the next frame
    GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(animation_animated_draw, aniIndex);

    // If another frame was found, draw it
    if (frame)
    {
        // gdraw_command_sequence_set_bounds_size(loading_bg_animated_draw, GSize(i0 * 10,300));
        gdraw_command_frame_draw(ctx, animation_animated_draw, frame, GPoint((bounds.size.w - seq_bounds.w) / 2, (bounds.size.h - seq_bounds.h) / 2));
    };

    // Advance to the next frame, wrapping if neccessary
    int num_frames = gdraw_command_sequence_get_num_frames(animation_animated_draw);
    aniIndex++;
    if (aniIndex == num_frames)
    {
        aniIndex = num_frames - 1;
    }
}

void HN_GetWin_RecapLoading(Window *window)
{
    this = window;
    Layer *wlayer = window_get_root_layer(window);
    GRect wbounds = layer_get_bounds(wlayer);

    animation_animated_draw = gdraw_command_sequence_create_with_resource(RESOURCE_ID_RECAP_ANI);
    if (animation_animated_draw == NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to load animation sequence!");
        return;
    }

    text = text_layer_create(GRect(0,104,200,0));
    text_layer_set_text(text, "RECAP");
    text_layer_set_text_alignment(text, GTextAlignmentCenter);
    text_layer_set_font(text, HN_Font1);
    text_layer_set_background_color(text, GColorYellow);
    text_layer_set_text_color(text, GColorBlack);

    GRect start = GRect(0, 102, 200, 0);
    GRect end = GRect(0, 102, 200, 22);

    PropertyAnimation *ani = property_animation_create_layer_frame(text_layer_get_layer(text), &start, &end);
    Animation *rawAni = property_animation_get_animation(ani);

    animation_set_curve(rawAni, AnimationCurveLinear);
    animation_set_delay(rawAni, 1000);
    animation_set_duration(rawAni, 500);

    animation_layer = layer_create(wbounds);
    layer_set_update_proc(animation_layer, render_ani_vec_animated);

    layer_add_child(wlayer, animation_layer);
    layer_add_child(wlayer, text_layer_get_layer(text));

    aniTimer = app_timer_register(DELTA, next_frame_handler, NULL);
    animation_schedule(rawAni);
}

void HN_ReadyWin_RecapLoading()
{
}

/// @brief Called when the window is removed
void HN_DesWin_RecapLoading(Window *window)
{
    layer_destroy(animation_layer);
    text_layer_destroy(text);
    gdraw_command_sequence_destroy(animation_animated_draw);
    window_destroy(this);
}