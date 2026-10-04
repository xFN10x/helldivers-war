#pragma once
#include <pebble.h>

//i have to make this take up less than 256 bytes. 2048 bits
typedef struct HN_MapData
{
    uint16_t war; //16 bits; 16

    uint16_t bugs; //16 again; 32 USAGE: 0000000000000000 each bit declares and area taken, 
    //so 1110000000000000 would mean the first 3 areas are taken
    uint16_t bots; //48 the last bit is used to declare if we are attacking a home planet.
    uint16_t illum; //64

    //                       unused
    //                       /| | |
    //0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 0
    //1 2 3 4 5 6 7 8 9 1 1         └ attacking
    //                  0 1 
    uint32_t bugMax; //96
    uint32_t bugPoints; // 128

    uint32_t botMax;
    uint32_t botPoints; //192

    uint32_t illumMax;
    uint32_t illumPoints; //256

    uint32_t superEarthMax;
    uint32_t superEarthPoints; //320
} HN_MapData;

extern struct HN_MapData HN_MAPDATA_TEST;
extern struct HN_MapData HN_MAPDATA_START;