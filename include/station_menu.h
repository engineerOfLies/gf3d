#ifndef __STATION_MENU_H__
#define __STATION_MENU_H__

#include "gf2d_windows.h"
#include "station.h"

Window *station_menu_window(Window *parent,StationData *station);
void station_menu_select_segment(Window *win,int segment);
const char *station_menu_get_selected(Window *win);

#endif
