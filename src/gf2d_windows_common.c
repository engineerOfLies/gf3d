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
    List *list;
    int count,i;
    Callback *callback;

    if (!win)return 0;
    if (!win->data)return 0;

    list = (List*)win->data;
    if (list)
    {
    count = gfc_list_get_count(list);

        for (i = 0; i < count; i++)
        {
            callback = (Callback*)gfc_list_get_nth(list,i);
            if (callback)
            {
                gfc_callback_free(callback);
            }
        }

        gfc_list_delete(list);
    }
    return 0;
}

int yes_no_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    Element *focus;
    List *callbacks;
    Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    
    if ((gf2d_mouse_hidden())&&(gfc_input_command_pressed("nextelement")))
    {
        gf2d_window_next_focus(win);
        return 1;
    }

    callbacks = (List*)win->data;
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
                callback = (Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
                break;
            case 52:
                callback = (Callback*)gfc_list_get_nth(callbacks,1);
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

Window *window_yes_no(char *text, void(*onYes)(void *),void(*onNo)(void *),void *data)
{
    Window *win;
    List *callbacks;
    win = gf2d_window_load("menus/yes_no_window.json");
    if (!win)
    {
        slog("failed to load yes/no window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),text);
    if (gf2d_mouse_hidden())gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,51));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onYes)
    {
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onYes,data));
    }
    if (onNo)
    {
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onNo,data));
    }
    win->data = callbacks;
    return win;
}


int ok_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    List *callbacks;
    Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    callbacks = (List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        switch(e->index)
        {
            case 51:
                callback = (Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
                break;
        }
    }
    return 0;
}

int alert_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    List *callbacks;
    Callback *callback;
    if (!win)return 0;
    callbacks = (List*)win->data;
    if (gf2d_mouse_button_pressed(0))
    {
        if (callbacks)
        {
            callback = (Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
        }
        gf2d_window_free(win);
        return 1;
    }
    if (!updateList)return 0;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"ok")==0)
        {
            callback = (Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            gf2d_window_free(win);
        }
    }
    return 0;
}

Window *window_alert(char *title, char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    List *callbacks;
    win = gf2d_window_load("menus/alert_menu.json");
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
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    return win;
}

Window *window_dialog(char *title, char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    List *callbacks;
    win = gf2d_window_load("menus/dialog.json");
    if (!win)
    {
        slog("failed to load alert window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,2),text);
    win->update = alert_update;
    win->free_data = yes_no_free;
    if (onOK)
    {
        callbacks = gfc_list_new();
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    return win;
}


Window *window_text_entry(char *question, char *defaultText,void *callbackData, size_t length, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    List *callbacks;
    win = gf2d_window_load("menus/text_entry_window.json");
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
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}

Window *window_key_value(char *question, char *defaultKey,char *defaultValue,void *callbackData, size_t keyLength,size_t valueLength, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    List *callbacks;
    win = gf2d_window_load("menus/key_value.json");
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
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        callbacks = gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}

/*eol@eof*/
