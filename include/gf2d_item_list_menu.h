#ifndef __ITEM_LIST_MENU_H__
#define __ITEM_LIST_MENU_H__

#include "gfc_list.h"
#include "gf2d_windows.h"

/**
 * @brief make a window to display a number of options
 * @param position where on the screen to place it (top left corner)
 * @param question the title to display
 * @param options a list of pointers to strings to populate the list with
 * @param onSelect a callback function to invoke when a selections is made
 * @param callbackData data to be provided to the callback function
 * @param result integer to be populated when a decision is made
 * @return a pointer to the newly created window, or NULL otherwise
 */
Window *item_list_menu(Window *parent,Vector2D position,char *question,List *options,void(*onSelect)(void *),void *callbackData,int *result);

#endif
