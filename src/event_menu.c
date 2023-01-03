#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_button.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"
#include "gf2d_windows_common.h"

#include "config_def.h"
#include "entity.h"
#include "resources.h"
#include "player.h"
#include "event_manager.h"
#include "event_menu.h"

typedef struct
{
    SJson *eventDef;
}EventMenuData;

int event_menu_free(Window *win)
{
    EventMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int event_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    EventMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (EventMenuData*)win->data;

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 1000)
        {
            event_manager_handle_choice(data->eventDef, e->index - 1000);
            gf2d_window_free(win);
            return 1;
        }
    }
    return 1;
}

int event_menu_draw(Window *win)
{
 //   EventMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void event_menu_setup(Window *win,EventMenuData *data)
{
    int i,c;
    Vector4D color;
    const char *text;
    SJson *array,*item;
    Element *questions,*button;
    if ((!win)||(!data))return;

    text = sj_object_get_value_as_string(data->eventDef,"portrait");
    if (text)gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"portrait"),text);
    
    text = sj_object_get_value_as_string(data->eventDef,"title");
    if (text)gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),text);

    text = sj_object_get_value_as_string(data->eventDef,"text");
    if (text)gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),text);
    
    questions = gf2d_window_get_element_by_name(win,"questions");
    if (!questions)return;
    array = sj_object_get_value(data->eventDef,"options");
    if (!array)return;
    c = sj_array_get_count(array);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        vector4d_set(color,255,255,255,255);
        sj_value_as_vector4d(sj_object_get_value(item,"color"),&color);
        text = sj_object_get_value_as_string(item,"text");
        button = gf2d_button_new_label_simple(win,1000+i,text,FT_Small,vector2d(1,30),gfc_color_from_vector4(color));
        if (!button)continue;
        gf2d_element_list_add_item(questions,button);
    }
}

Window *event_menu(Window *parent,const char *eventName)
{
    Window *win;
    EventMenuData *data;
    win = gf2d_window_load("menus/event.menu");
    if (!win)
    {
        slog("failed to load event menu");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(EventMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = event_menu_update;
    win->free_data = event_menu_free;
    win->draw = event_menu_draw;
    data->eventDef = config_def_get_by_name("events",eventName);
    if (!data->eventDef)
    {
        slog("no event by name %s found!",eventName);
        gf2d_window_free(win);
        return NULL;
    }
    event_menu_setup(win,data);
    message_buffer_bubble();
    return win;
}
