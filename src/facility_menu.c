#include <stdio.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_input.h"

#include "gf2d_windows.h"
#include "gf2d_elements.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_button.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"
#include "gf2d_message_buffer.h"

#include "config_def.h"
#include "station_def.h"
#include "resources.h"
#include "player.h"
#include "station.h"
#include "station_extension_menu.h"
#include "facility_buy_menu.h"
#include "facility_menu.h"

typedef struct
{
    StationSection *parent;
    StationData *station;
    const char *selected;
    int choice;
}FacilityMenuData;

void facility_menu_set_list(Window *win);

int facility_menu_free(Window *win)
{
    FacilityMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    gf2d_window_close_child(win,win->child);
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int facility_menu_draw(Window *win)
{
//    FacilityMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void facility_menu_select_item(Window *win,const char *name)
{
    SJson *def;
    const char *str;
    FacilityMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!name)return;
    if (data->selected == name)return;//nothing to do
    if (strcmp(name,"<Empty>")==0)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),"Empty Slot");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),"This slot is free for new facility installation");
        gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),NULL);
        return;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),name);
    def = config_def_get_by_parameter("facilities","displayName",name);
    if (!def)return;
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    data->selected = name;
    str = sj_object_get_value_as_string(def,"icon");
    if (str)
    {
        gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    }

}

void facility_menu_yes(void *Data)
{
    Window *win;
    StationFacility *facility;
    FacilityMenuData *data;
    List *cost;
    win = Data;
    if (!win)return;
    win->child = NULL;
    data = win->data;
    if (!data)return;
    if (!data->selected)return;
    cost = station_facility_get_resource_cost(station_facility_get_name_from_display(data->selected));
    resource_list_sell(player_get_resources(), cost,0.9);
    resources_list_free(cost);
    
    facility = gfc_list_get_nth(data->parent->facilities,data->choice);
    station_facility_free(facility);
    gfc_list_delete_nth(data->parent->facilities,data->choice);
    facility_menu_set_list(win);
}

void facility_menu_no(void *Data)
{
    Window *win;
    win = Data;
    if (!win)return;
    win->child = NULL;
}

int facility_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    FacilityMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (FacilityMenuData*)win->data;
    if (!data)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"close")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
        if (strcmp(e->name,"buy")==0)
        {
            win->child = facility_buy_menu(win,data->parent,data->choice);
        }
        if (strcmp(e->name,"sell")==0)
        {
            win->child = window_yes_no("Sell selected facility?", facility_menu_yes,facility_menu_no,win);
            return 1;
        }
        if (e->index >= 1000)
        {
            data->choice = e->index - 1000;
            facility_menu_select_item(win,e->name);
            return 1;
        }
    }
    if (gfc_input_command_released("cancel"))
    {
        gf2d_window_free(win);
        return 1;
    }
    if ((gf2d_mouse_button_state(0))||(gf2d_mouse_button_state(1)))
    {
        if (!gf2d_window_mouse_in(win))
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return gf2d_window_mouse_in(win);
}

void facility_menu_set_list(Window *win)
{
    const char *str;
    StationFacility *facility;
    Element *button;
    Element *item_list;
    int i,c;
    FacilityMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->parent)return;
    c = data->parent->facilitySlots;
    item_list = gf2d_window_get_element_by_name(win,"item_list");
    if (!item_list)return;
    gf2d_element_list_free_items(item_list);
    for (i = 0; i < c; i++)
    {
        facility = gfc_list_get_nth(data->parent->facilities,i);
        if (!facility)
        {
            button = gf2d_button_new_label_simple(win,1000+i,"<Empty>",gfc_color8(255,255,255,255));
            if (!button)continue;
            gf2d_element_list_add_item(item_list,button);
            continue;
        }
        str = station_facility_get_display_name(facility->name);
        if (i == 0)
        {
            facility_menu_select_item(win,str);
        }
        button = gf2d_button_new_label_simple(win,1000+i,str,gfc_color8(255,255,255,255));
        if (!button)continue;
        gf2d_element_list_add_item(item_list,button);
    }
}

Window *facility_menu(Window *parent,StationData *station, StationSection *parentSection)
{
    Window *win;
    FacilityMenuData* data;
    win = gf2d_window_load("menus/facility.menu");
    if (!win)
    {
        slog("failed to load facility menu");
        return NULL;
    }
    win->update = facility_menu_update;
    win->free_data = facility_menu_free;
    win->draw = facility_menu_draw;
    data = (FacilityMenuData*)gfc_allocate_array(sizeof(FacilityMenuData),1);
    win->data = data;
    win->parent = parent;
    data->station = station;
    data->parent = parentSection;
    facility_menu_set_list(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
