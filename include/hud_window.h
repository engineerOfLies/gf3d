#ifndef __HUD_WINDOW_H__
#define __HUD_WINDOW_H__

#include "gf2d_windows.h"

/**
 * @brief create the hud window and load the provided save file
 * @param savefile the game to load
 */
Window *hud_window(const char *savefile);

/**
 * @brief reset the camera back to hud standard view
 * @param win the hud window
 */
void hud_reset_camera(Window *win);


#endif
