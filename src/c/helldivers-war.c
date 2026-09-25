#include <pebble.h>

#include "map.h"

typedef struct
{
  WindowHandler create;
  WindowHandler destroy;
  void (*ready)();
} HN_Win;

HN_Win HN_MAIN_WIN = {HN_GetWin_Map, HN_DesWin_Map, HN_ReadyWin_Map};

static Window *s_window;
static TextLayer *s_text_layer;

void HN_SwitchWin(HN_Win *win, const bool animated)
{
  s_window = window_create();
  //window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = win->create,
                                           .unload = win->destroy,
                                       });
  window_stack_push(s_window, animated);
  win->ready();
}

static void prv_init(void)
{
  HN_SwitchWin(&HN_MAIN_WIN, true);
}

static void prv_deinit(void)
{
  window_destroy(s_window);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
