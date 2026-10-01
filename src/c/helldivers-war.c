#include <pebble.h>

#include "helldivers-war.h"
#include "loading_win.h"
#include "failed_connect_win.h"
#include "recap_loading_win.h"
#include "messages.h"

HN_Win HN_LOADING_WIN = {"loading_win", HN_GetWin_Loading, HN_DesWin_Loading, HN_ReadyWin_Loading};
HN_Win HN_FAILED_CONNECT_WIN = {"failed_connect_win", HN_GetWin_FailedConnec, HN_DesWin_FailedConnec, HN_ReadyWin_FailedConnec};
HN_Win HN_RECAP_LOADING_WIN = {"recap_loading", HN_GetWin_RecapLoading, HN_DesWin_RecapLoading, HN_ReadyWin_RecapLoading};

static Window *current_window = NULL;
static HN_Win *current_hn_window = NULL;

GFont HN_Font1;
GFont HN_Font2;

int HN_STORKEY_MAPCACHE = 0;

void HN_SwitchWin(HN_Win *hnwin, const bool animated)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Switching win: %s", hnwin->name);
  Window *building = window_create();
  // window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(building, (WindowHandlers){
                                           .load = hnwin->create,
                                           .unload = hnwin->destroy,
                                       });
  window_stack_push(building, animated);
  if (current_hn_window)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Removing last win: %s", current_hn_window->name);
    window_stack_remove(current_window, true);
  }
  current_window = building;
  current_hn_window = hnwin;
  hnwin->ready();
}

static void prv_init(void)
{
  msg_init();

  HN_Font1 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MAIN_FONT_20));

  HN_SwitchWin(&HN_LOADING_WIN, true);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", current_window);

  app_event_loop();
}
