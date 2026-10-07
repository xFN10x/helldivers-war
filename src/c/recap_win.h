#pragma once
#include <pebble.h>
#include "storage.h"

typedef struct RecapTimerData {
    size_t index;
    size_t length;
    uint16_t *array;
    char **capitals;
    char **regions;
    size_t sector_offset;
    uint16_t *sector_data_to_change;
    struct RecapTimerData *next;
} RecapTimerData;

void HN_GetWin_Recap(Window *window);
void HN_ReadyWin_Recap();
void HN_ClickProvWin_Recap(void *context);
void HN_DoRecap(HN_MapData *end);
/// @brief Called when the window is removed
void HN_DesWin_Recap(Window *window);