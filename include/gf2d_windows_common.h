#ifndef __WINDOWS_COMMON_H__
#define __WINDOWS_COMMON_H__

#include "gf2d_windows.h"

/**
 * @purpose this file contains common window that can be customized through modifying their menus/config file
 */

/**
 * @brief open an alert dialog box
 * @param title title of the window
 * @param text text of the dialog
 * @param onOK the function to call if the user selects OK
 * @param data data to be provided to the callback functions
 */
Window *window_alert(const char *title, const  char *text, void(*onOK)(void *),void *okData);

/**
 * @brief open a text dialog box
 * @param title title of the window
 * @param text text of the dialog
 * @param onOK the function to call if the user selects OK
 * @param data data to be provided to the callback functions
 */
Window *window_dialog(const char *title, const char *text, void(*onOK)(void *),void *okData);

/**
 * @brief open a yes / no dialog box
 * @param parent [optional ]the window that spawned this window
 * @param text question to prompt the user
 * @param onYes the function to call if the user selects yes
 * @param onNo the function to call if the user selects no
 * @param data data to be provided to the callback functions
 */
Window *window_yes_no(Window *parent,const char *text, void(*onYes)(void *),void(*onNo)(void *),void *data);

/**
 * @brief open a dialog box to take a text entry
 * @param question question to prompt the user
 * @param defaultText the text to be prepopulated in the dialog/ what is changed by the dialog
 * @param callbackData data to be provided to the callback functions
 * @param length limit the value to this length
 * @param onOK the function to call if the user selects okay
 * @param onCancel the function to call if the user selects cancel
 */
Window *window_text_entry(const char *question, char *defaultText, void *callbackData, size_t length, void(*onOk)(void *),void(*onCancel)(void *));

/**
 * @brief open a dialog box to take a key/value pair
 * @param question question to prompt the user
 * @param defaultKey the key to be prepopulated in the dialog/ what is changed by the dialog
 * @param defaultValue the value to be prepopulated in the dialog/ what is changed by the dialog
 * @param callbackData data to be provided to the callback functions
 * @param keyLength limit the key to this length
 * @param valueLength limit the value to this length
 * @param onOK the function to call if the user selects okay
 * @param onCancel the function to call if the user selects cancel
 */
Window *window_key_value(const char *question, char *defaultKey,char *defaultValue,void *callbackData, size_t keyLength,size_t valueLength, void(*onOk)(void *),void(*onCancel)(void *));


/**
 * @brief open a color selection dialog
 * @param title the title to display
 * @param color the color to be changed / what is changed by this dialog
 * @param onOK callback function to be invoked if the users selects Okay
 * @param onCancel callback function to be invoked if the users selects cancel
 * @param data data to be sent with the callback function
 */
Window *window_color_select(const char *title, GFC_Color *color, void(*onOK)(void *),void(*onCancel)(void *),void *data);


#endif
