#ifndef __HUD_WINDOW_H__
#define __HUD_WINDOW_H__

#include "gf2d_windows.h"
#include "gf3d_entity.h"

/**
 * @brief create the hud window and load the provided save file
 */
Window *hud_window();

/**
 * @brief open the options list select menu
 */
void hud_open_options_menu(Window *win);

/**
 * @brief reset the camera back to hud standard view
 * @param win the hud window
 */
void hud_reset_camera(Window *win);


#endif
