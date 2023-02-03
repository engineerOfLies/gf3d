#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_button.h"
#include "gf2d_element_list.h"
#include "gf2d_element_entry.h"

#include "gf2d_message_buffer.h"

typedef struct
{
    TextLine    text;
    Color       color;
    Uint32      ttl;
}Message;

typedef struct
{
    Color   defaultColor;
    int     bufferCount;// how many messages can be displayed at once
    Uint32  ttl;        // how long each message will be given to live
    List   *messages;   // text buffers for the messages
}MessageBufferData;

static Window *MessageWindow = NULL; // WE ARE A SINGLETON


int message_buffer_free(Window *win)
{
    int count,i;
    Message *message;
    MessageBufferData *data;

    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    count = gfc_list_get_count(data->messages);

    for (i = 0; i < count; i++)
    {
        message = gfc_list_get_nth(data->messages,i);
        if (message)
        {
            free(message);
        }
    }

    gfc_list_delete(data->messages);
    MessageWindow = NULL;
    return 0;
}

int message_buffer_update(Window *win,List *updateList)
{
    int i,count;
    Message *message;
    Element *e;
    MessageBufferData *data;
    if (!win)return 0;
    data = win->data;
    count = gfc_list_get_count(data->messages);
    // clean up old messages
    for (i = count -1; i >= 0; i--)
    {
        message = gfc_list_get_nth(data->messages,i);
        if (!message)continue;
        message->ttl--;
        if (message->ttl <= 0)
        {
            gfc_list_delete_nth(data->messages,i);
            free(message);
        }
    }
    //reset labels
    for (i = 0; i < data->bufferCount; i++)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,50+i)," ");
    }
    //populate labels
    count = gfc_list_get_count(data->messages);
    for (i = 0;(i < count)&&( i < data->bufferCount);i++)
    {
        message = gfc_list_get_nth(data->messages,i);
        if (!message)continue;
        e = gf2d_window_get_element_by_id(win,50+i);
        gf2d_element_label_set_text(e,message->text);
        gfc_color_copy(e->color,message->color);
    }
    return 0;
}

Window *window_message_buffer(int count, Uint32 timeout, Color defaultColor)
{
    int i;
    LabelElement *label;
    Element *l,*p;
    Window *win;
    MessageBufferData *data;
    
    win = gf2d_window_load("menus/message_buffer.json");
    if (!win)
    {
        slog("failed to load message buffer");
        return NULL;
    }
    win->update = message_buffer_update;
    win->free_data = message_buffer_free;
    //win->no_draw_generic = 1;
    
    data = gfc_allocate_array(sizeof(MessageBufferData),1);
    data->messages = gfc_list_new();
    data->bufferCount = count;
    data->ttl = timeout;
    gfc_color_copy(data->defaultColor,defaultColor);

    p = gf2d_window_get_element_by_id(win,0);
    for (i =0; i  < count; i++)
    {
        l = gf2d_element_new_full(
            p,
            50 + i,
            NULL,
            gfc_rect(0,0,1,19),
            gfc_color8(255,255,255,255),
            0,
            gfc_color8(255,255,255,255),
            0,
            win);
        label = gf2d_element_label_new_full(
            NULL,
            gfc_color8(255,255,255,255),
            FT_H6,
            LJ_Left,
            LA_Middle,
            0);
        gf2d_element_make_label(l,label);
        gf2d_element_list_add_item(p,l);
    }
    win->data = data;
    MessageWindow = win;
    return win;
}

void message_new(const char *newMessage)
{
    MessageBufferData *data;
    Message *message;
    if (!newMessage)return;
    if (!MessageWindow)return;
    message = gfc_allocate_array(sizeof(Message),1);
    if (!message)return;
    
    data = MessageWindow->data;
    
    gfc_color_copy(message->color,data->defaultColor);
    message->ttl = data->ttl;
    gfc_line_cpy(message->text,newMessage);
    data->messages = gfc_list_append(data->messages,message);
    slog((char *)newMessage);
}

void message_printf(const char *newMessage,...)
{
    TextLine msg;
    va_list ap;
    /*echo all logging to stdout*/
    va_start(ap,newMessage);
    vsprintf(msg,newMessage,ap);
    va_end(ap);
    message_new(msg);
}

void message_buffer_set_position(Vector2D position)
{
    if (!MessageWindow)return;
    gf2d_window_set_position(MessageWindow,position);
}

void message_buffer_bubble()
{
    gf2d_window_bring_to_front(MessageWindow);
}


/*eol@eof*/
