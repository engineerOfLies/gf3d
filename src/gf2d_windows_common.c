#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"

#include "gf2d_windows_common.h"

int yes_no_free(Window *win)
{
    GFC_List *list;
    int count,i;
    GFC_Callback *callback;

    if (!win)return 0;
    if (!win->data)return 0;

    list = (GFC_List*)win->data;
    if (list)
    {
    count = gfc_list_get_count(list);

        for (i = 0; i < count; i++)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(list,i);
            if (callback)
            {
                gfc_callback_free(callback);
            }
        }

        gfc_list_delete(list);
    }
    return 0;
}

int yes_no_update(Window *win,GFC_List *updateList)
{
    int i,count;
    Element *e;
    Element *focus;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    
    if ((gf2d_mouse_hidden())&&(gfc_input_command_pressed("nextelement")))
    {
        gf2d_window_next_focus(win);
        return 1;
    }

    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if ((strcmp(e->name,"item_right")==0)||(strcmp(e->name,"item_left")==0))
        {
            focus = gf2d_window_get_element_by_focus(win);
            if ((!focus)||(focus->index == 52))
            {
                gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,51));
            }
            else
            {
                gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,52));
            }
            return 1;
        }
        switch(e->index)
        {
            case 51:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
            case 52:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,1);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
        }
    }
    return 1;
}

Window *window_yes_no(Window *parent,const char *text, void(*onYes)(void *),void(*onNo)(void *),void *data)
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/yes_no_window.menu");
    if (!win)
    {
        slog("failed to load yes/no window");
        return NULL;
    }
    win->parent = parent;
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),text);
    if (gf2d_mouse_hidden())gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,51));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onYes)
    {
        gfc_list_append(callbacks,gfc_callback_new(onYes,data));
    }
    if (onNo)
    {
        gfc_list_append(callbacks,gfc_callback_new(onNo,data));
    }
    win->data = callbacks;
    return win;
}


int ok_update(Window *win,GFC_List *updateList)
{
    int i,count;
    Element *e;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        switch(e->index)
        {
            case 51:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
        }
    }
    return 0;
}

int alert_update(Window *win,GFC_List *updateList)
{
    int i,count;
    Element *e;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    callbacks = (GFC_List*)win->data;
    if (gf2d_mouse_button_pressed(0))
    {
        if (callbacks)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
        }
        gf2d_window_free(win);
        return 1;
    }
    if (!updateList)return 1;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if ((strcmp(e->name,"ok")==0)||(strcmp(e->name,"cancel")==0))
        {
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            gf2d_window_free(win);
            return 1;
        }
    }
    return 1;
}

Window *window_alert(const char *title, const char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/alert_menu.menu");
    if (!win)
    {
        slog("failed to load alert window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),text);
    win->update = alert_update;
    win->free_data = yes_no_free;
    if (onOK)
    {
        callbacks = gfc_list_new();
        gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    return win;
}

Window *window_dialog(const char *title, const char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/dialog.menu");
    if (!win)
    {
        slog("failed to load dialog window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),text);
    win->update = alert_update;
    win->free_data = yes_no_free;
    if (onOK)
    {
        callbacks = gfc_list_new();
        gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    gf2d_window_bring_to_front(win);
    return win;
}


Window *window_text_entry(const char *question, char *defaultText,void *callbackData, size_t length, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/text_entry_window.menu");
    if (!win)
    {
        slog("failed to load text entry window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),question);
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,2),defaultText,length);
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,2));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onOk)
    {
        gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}

Window *window_key_value(const char *question, char *defaultKey,char *defaultValue,void *callbackData, size_t keyLength,size_t valueLength, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/key_value.menu");
    if (!win)
    {
        slog("failed to load key_value window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),question);
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,2),defaultKey,keyLength);
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,2));
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,3),defaultValue,valueLength);
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onOk)
    {
        gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}


typedef struct
{
    GFC_Color *color;
    GFC_Color color8;
    GFC_Callback okay;
    GFC_Callback cancel;
}GF2DWindowGFC_ColorPickData;

int window_color_select_free(Window *win)
{
    GF2DWindowGFC_ColorPickData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    free(data);
    return 0;
}

int window_color_select_update(Window *win,GFC_List *updateList)
{
    int i,count;
    Element *e;
    GF2DWindowGFC_ColorPickData *data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"red_up")==0)
        {
            if (data->color8.r < 255)
            {
                data->color8.r++;
                if (data->color8.r > 255)data->color8.r = 255;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"red_down")==0)
        {
            if (data->color8.r > 0)
            {
                data->color8.r--;
                if (data->color8.r < 0)data->color8.r = 0;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"green_up")==0)
        {
            if (data->color8.g < 255)
            {
                data->color8.g++;
                if (data->color8.g > 255)data->color8.g = 255;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"green_down")==0)
        {
            if (data->color8.g > 0)
            {
                data->color8.g--;
                if (data->color8.g < 0)data->color8.g = 0;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"blue_up")==0)
        {
            if (data->color8.b < 255)
            {
                data->color8.b++;
                if (data->color8.b > 255)data->color8.b = 255;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"blue_down")==0)
        {
            if (data->color8.b > 0)
            {
                data->color8.b--;
                if (data->color8.b < 0)data->color8.b = 0;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (strcmp(e->name,"ok")==0)
        {
            gfc_color_copy((*data->color),data->color8);
            gfc_callback_call(&data->okay);
            gf2d_window_free(win);
            return 1;
        }
        if (strcmp(e->name,"cancel")==0)
        {
            gfc_callback_call(&data->cancel);
            gf2d_window_free(win);
            return 1;
        }
    }
    return 0;
}

void window_color_refresh(Window *win)
{
    GFC_TextLine buffer;
    GF2DWindowGFC_ColorPickData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    gf2d_element_set_background_color(gf2d_window_get_element_by_name(win,"color"),data->color8);
    gfc_line_sprintf(buffer,"%i",(int)data->color8.r);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"red"),buffer);
    gfc_line_sprintf(buffer,"%i",(int)data->color8.g);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"green"),buffer);
    gfc_line_sprintf(buffer,"%i",(int)data->color8.b);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"blue"),buffer);
}

Window *window_color_select(const char *title, GFC_Color *color, void(*onOK)(void *),void(*onCancel)(void *),void *okData)
{
    Window *win;
    GF2DWindowGFC_ColorPickData *data;
    if (!color)return NULL;
    win = gf2d_window_load("menus/color_pick.menu");
    if (!win)
    {
        slog("failed to load color pick menu");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(GF2DWindowGFC_ColorPickData),1);
    if (!data)
    {
        gf2d_window_free(win);
        slog("failed to allocate data for window color pick menu");
        return NULL;
    }
    win->data = data;
    data->okay.callback = onOK;
    data->okay.data = okData;
    data->cancel.callback = onCancel;
    data->cancel.data = okData;
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),title);
    data->color = color;
    data->color8 = gfc_color_to_int8(*color);
    //set colors
    window_color_refresh(win);
    win->refresh = window_color_refresh;
    win->update = window_color_select_update;
    win->free_data = window_color_select_free;
    return win;
}


/*eol@eof*/
