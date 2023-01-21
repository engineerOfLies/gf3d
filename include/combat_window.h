#ifndef __COMBAT_WINDOW_H__
#define __COMBAT_WINDOW_H__

#include "gf2d_windows.h"

/**
 * @brief create the combat window 
 * @param parent the parent window (the hud)
 * @param zone which combat zone to setup
 * @return a pointer to ME
 */
Window *combat_window(Window *parent,const char *zone);


#endif
