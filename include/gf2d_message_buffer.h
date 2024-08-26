#ifndef __MESSAGE_BUFFER_H__
#define __MESSAGE_BUFFER_H__

#include "gfc_color.h"
#include "gf2d_font.h"
#include "gf2d_windows.h"

/**
 * @brief intialize the message buffer window
 * @note this is a singleton and won't make a new one if one already exists
 * @param count how many messages will be displayed at once
 * @param timeout how long messages stay in the buffer before timing out
 * @param defaultColor the color the messages are drawn with
 * @param font the font to use for the messages
 */
Window *window_message_buffer(int count, Uint32 timeout, GFC_Color defaultColor, FontTypes font);

/**
 * @brief set behavior to always show the most recent message and push older messages away faster
 * @param on if true, enables this mode, if false disables it
 */
void message_buffer_set_latest_mode(Uint8 on);

/**
 * @brief print a new message to the message buffer
 * @param color the color to print with
 * @param newMessage the message to print
 */
void message_new(const char *newMessage);

/**
 * @brief print a new message to the message buffer
 * @param newMessage the message to print
 */
void message_new_color(const char *newMessage,GFC_Color color);

/**
 * @brief print a new message to the message buffer using printf style
 * @param newMessage the message to print
 * @param ... additional arguments
 */
void message_printf(const char *newMessage,...);

/**
 * @brief print a new message to the message buffer using printf style with a specific color
 * @param newMessage the message to print
 * @param color the color to print with
 * @param ... additional arguments
 */
void message_printf_color(GFC_Color color,const char *newMessage,...);

/**
 * @brief make the message buffer the top drawing window
 */
void message_buffer_bubble();

/**
 * @brief change where on the screen the message buffer is
 * @param position where on the screen to move the buffer to
 */
void message_buffer_set_position(GFC_Vector2D position);


#endif
