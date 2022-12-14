#ifndef __RESOURCE_LIST_H__
#define __RESOURCE_LIST_H__

#include "gfc_list.h"
#include "gf2d_elements.h"
#include "gf2d_windows.h"

/**
 * @brief create an window element list of resources, optionally color code based on supply if provided
 * @param win the parent window
 * @param name the name id to get it by again later
 * @param offset offset position of the list.  relative to parent element
 * @param supply list of resources to display.  how much of a resource there is available.  
 * @param cost If provided, this will be shown as supply/cost and show in Green if supply>=cost or red otherwise
 * @note if a resource is more than the supply
 */
Element *resource_list_new(Window *win,const char *name, Vector2D offset,List *supply,List *cost);

#endif
