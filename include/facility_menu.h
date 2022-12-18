#ifndef __FACILITY_MENU_H__
#define __FACILITY_MENU_H__

#include "gf2d_windows.h"
#include "station_facility.h"

Window *facility_menu(Window *parent,StationData *station, StationSection *parentSection);

void facility_menu_set_list(Window *win);

#endif
