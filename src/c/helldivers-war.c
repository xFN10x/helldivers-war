#include <pebble.h>

#include "helldivers-war.h"
#include "loading_win.h"
#include "failed_connect_win.h"

HN_Win HN_LOADING_WIN = {"loading_win", HN_GetWin_Loading, HN_DesWin_Loading, HN_ReadyWin_Loading};
HN_Win HN_FAILED_CONNECT_WIN = {"failed_connect_win", HN_GetWin_FailedConnc, HN_DesWin_FailedConnc, HN_ReadyWin_FailedConnc};

static Window *current_window = NULL;
static HN_Win *current_hn_window = NULL;

GFont HN_Font1;
GFont HN_Font2;

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

static void message_recieved(DictionaryIterator *iterator, void *context)
{
  Tuple *ready = dict_find(iterator, MESSAGE_KEY_ready);
  Tuple *HBPing = dict_find(iterator, MESSAGE_KEY_HBPing);

  if (ready)
  {
    HN_Win_Loading_JSReady();
  }
}

static void prv_init(void)
{
  app_message_register_inbox_received(message_recieved);

  HN_Font1 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MAIN_FONT_20));

  HN_SwitchWin(&HN_LOADING_WIN, true);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", current_window);

  app_event_loop();
}
