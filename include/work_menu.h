#ifndef __WORK_MENU_H__
#define __WORK_MENU_H__

#include "station.h"
#include "station_facility.h"
#include "gf2d_windows.h"

/**
 * @brief open up a work menu for things like repair, remove, and build for station sections and facilities
 * @param parent the parent window
 * @param facilityList if you are building a new facility, you need this to know where to build it.
 * @param section you need this for a section remove or repair order
 * @param facility you need this for a facility remove or repair order
 * @param action what is happening "repair","remove","build_facility","build_section"
 * @param where if this a planet facility to be build, where to build it
 * @param callback (optional) if set, this function will be called when the menu closes
 * @param callbackData data provided to the callback
 * @return a pointer to the new window, NULL if failed
 */
Window *work_menu(
    Window *parent,
    List *facilityList,
    StationSection *section,
    StationFacility *facility,
    const char *action,
    const char *what,
    Vector2D where,
    void (*callback)(void *),
    void *callbackData);


#endif
