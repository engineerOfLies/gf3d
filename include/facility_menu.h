#ifndef __FACILITY_MENU_H__
#define __FACILITY_MENU_H__

#include "gf2d_windows.h"
#include "station_facility.h"

Window *facility_menu(Window *parent, List *facility_list,int slot_limit, List *type_list);

void facility_menu_set_list(Window *win);

void facility_menu_select_item(Window *win,int choice);


#endif
