#ifndef __STATION_EXTENSION_MENU_H__
#define __STATION_EXTENSION_MENU_H__

#include "gf2d_windows.h"
#include "station.h"

Window *station_extension_menu(
    Window *parent,
    Vector2D position,
    StationSection *section,
    void(*onSelect)(void *),
    void *callbackData,
    int *result);
    
#endif
