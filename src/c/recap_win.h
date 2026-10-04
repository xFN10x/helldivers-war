#pragma once
#include <pebble.h>
#include "storage.h"

void HN_GetWin_Recap(Window *window);
void HN_ReadyWin_Recap();
void HN_ClickProvWin_Recap(void *context);
void HN_DoRecap(HN_MapData *end);
/// @brief Called when the window is removed
void HN_DesWin_Recap(Window *window);