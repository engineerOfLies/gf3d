#ifndef __ITEM_LIST_MENU_H__
#define __ITEM_LIST_MENU_H__

#include "gfc_list.h"
#include "gf2d_font.h"
#include "gf2d_element_button.h"
#include "gf2d_windows.h"

/**
 * @brief make a window to display a number of options
 * @param parent the parent window that this was spawned from
 * @param position where on the screen to place it (top left corner)
 * @param persist if true, don't close after a selection is made
 * @param allowClose if true, allow the "close" button to close the window without a selection made
 * @param question the title to display
 * @param options a list of pointers to strings to populate the list with
 * @param questionFont the font to use for the question
 * @param questionColor the colof to use for the question
 * @param font the font to use for the menu items
 * @param fontBase color for text options
 * @param fontSelected color for text options when selected
 * @param backgroundBase color for text options background
 * @param backgroundSelected color for text options background when selected
 * @param onSelect a callback function to invoke when a selections is made
 * @param callbackData data to be provided to the callback function
 * @param result integer to be populated when a decision is made.  If this is closed without makine a choice, this will be set to -1, the index in the provided otherwise
 * @return a pointer to the newly created window, or NULL otherwise
 */
Window *item_list_menu(
    Window *parent,
    GFC_Vector2D    position,
    Uint8           persist,
    Uint8           allowClose,
    const char     *question,
    FontTypes       questionFont,
    GFC_Color       questionColor,
    GFC_List       *options,
    FontTypes       font,
    GFC_Color       fontBase,
    GFC_Color       backgroundBase,
    GFC_Color       Selected,
    ButtonCustomAction highStyle,
    void(*onSelect)(void *),
    void *callbackData,
    int *result);

#endif
