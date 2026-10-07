#include <pebble.h>

#include "loading_win.h"
#include "failed_connect_win.h"
#include "recap_loading_win.h"
#include "recap_win.h"

#include "messages.h"
#include "helldivers-war.h"

HN_Win HN_LOADING_WIN = {"loading_win", HN_GetWin_Loading, HN_DesWin_Loading, HN_ReadyWin_Loading, NULL};
HN_Win HN_FAILED_CONNECT_WIN = {"failed_connect_win", HN_GetWin_FailedConnec, HN_DesWin_FailedConnec, HN_ReadyWin_FailedConnec, NULL};
HN_Win HN_RECAP_LOADING_WIN = {"recap_loading", HN_GetWin_RecapLoading, HN_DesWin_RecapLoading, HN_ReadyWin_RecapLoading, NULL};
HN_Win HN_RECAP_WIN = {"recap", HN_GetWin_Recap, HN_DesWin_Recap, HN_ReadyWin_Recap, HN_ClickProvWin_Recap};

static Window *current_window = NULL;
static HN_Win *current_hn_window = NULL;

GFont HN_Font1;
GFont HN_Font2;

char *HN_bugRegionNames[11] = {
  "Wise Region",
  "Kruger System",
  "Ross System",
  "Struve Region",
  "Xi Tauri Region",
  "Cancri System",
  "Higgs Region",
  "Hawking Region",
  "Rigel System",
  "Aurigae Region",
  "Kepler System"
};

char *HN_bugPlanetNames[11] = {
  "New New York",
  "Liberty City",
  "Tiberia",
  "Northman's Creek",
  "New Haven",
  "Freedom Fortress",
  "Martyr's Bay",
  "Segma Prime",
  "Freedom Peak",
  "Final Frontier",
  "Kepler Prime"
};

char *HN_botRegionNames[11] = {
  "Sirius Region",
  "Polaris Region",
  "Pictor Sector",
  "Sagan Region",
  "Horolium System",
  "Gellert Region",
  "Lacaille Region",
  "Indi System",
  "Ceti System",
  "Cygni Region",
  "Cyberstan Region"
};

char *HN_botPlanetNames[11] = {
  "Stockholm City",
  "Thunder Head",
  "New Moscow",
  "Highwind",
  "Providence",
  "Gellert City",
  "Bahia Democracia",
  "Winter Hold",
  "Doral Creek",
  "New Berlin",
  "Cyberstan"
};

char *HN_illumRegionNames[11] = {
  "Centaury Region",
  "Barnard Region",
  "Procyon Region",
  "Castor System",
  "Orionis Region",
  "Prometheus System",
  "Cassiopaiae Region",
  "Ursa Region",
  "Canes Region",
  "Arcturus Region",
  "Squ'bai System"
};

char *HN_illumPlanetNames[11] = {
  "New Hanover",
  "Iron Tower",
  "White Landing",
  "Justice Bay",
  "New Alexandria",
  "Ribatishiti",
  "Dal Rage",
  "Ultima",
  "Jiyu Toshi",
  "Hawk Nest",
  "Squ'bai Shrine"
};

int HN_STORKEY_MAPCACHE = 0;

void HN_SwitchWin(HN_Win *hnwin, const bool animated)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Switching win: %s", hnwin->name);
  Window *building = window_create();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Click prov is: %i", hnwin->clickProvider == NULL);
  if (hnwin->clickProvider)
  {
    window_set_click_config_provider(building, hnwin->clickProvider);
  }
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
