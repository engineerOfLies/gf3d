#ifndef __MESSAGE_BUFFER_H__
#define __MESSAGE_BUFFER_H__

#include "gfc_color.h"
#include "gf2d_windows.h"

/**
 * @brief intialize the message buffer window
 * @param count how many messages will be displayed at once
 */
Window *window_message_buffer(int count, Uint32 timeout, Color defaultColor);

void message_new(const char *newMessage);
void message_printf(const char *newMessage,...);
void message_buffer_bubble();


#endif
