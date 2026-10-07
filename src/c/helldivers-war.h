#pragma once
#include <pebble.h>

typedef struct
{
  char *name;
  WindowHandler create;
  WindowHandler destroy;
  void (*ready)();
  ClickConfigProvider clickProvider;
} HN_Win;

extern HN_Win HN_LOADING_WIN;
extern HN_Win HN_FAILED_CONNECT_WIN;
extern HN_Win HN_RECAP_LOADING_WIN;
extern HN_Win HN_RECAP_WIN;

extern GFont HN_Font1;
extern GFont HN_Font2;

extern int HN_STORKEY_MAPCACHE;

extern char *HN_bugRegionNames[11];
extern char *HN_bugPlanetNames[11];

extern char *HN_botRegionNames[11];
extern char *HN_botPlanetNames[11];

extern char *HN_illumRegionNames[11];
extern char *HN_illumPlanetNames[11];

void HN_SwitchWin(HN_Win *win, const bool animated);