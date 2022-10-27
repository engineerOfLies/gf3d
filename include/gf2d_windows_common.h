#ifndef __WINDOWS_COMMON_H__
#define __WINDOWS_COMMON_H__

#include "gf2d_windows.h"

/**
 * @purpose this file contains common window that can be customized through modifying their menus/config file
 */

Window *window_alert(char *title, char *text, void(*onOK)(void *),void *okData);

Window *window_dialog(char *title, char *text, void(*onOK)(void *),void *okData);

Window *window_yes_no(char *text, void(*onYes)(void *),void(*onNo)(void *),void *data);

Window *window_text_entry(char *question, char *defaultText, void *callbackData, size_t length, void(*onOk)(void *),void(*onCancel)(void *));

Window *window_key_value(char *question, char *defaultKey,char *defaultValue,void *callbackData, size_t keyLength,size_t valueLength, void(*onOk)(void *),void(*onCancel)(void *));


#endif
