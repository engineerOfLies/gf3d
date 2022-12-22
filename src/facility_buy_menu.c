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
#include "facility_menu.h"
#include "facility_buy_menu.h"

typedef struct
{
    StationSection *parent;
    int facility_slot;
    const char *selected;
    List *list;
    List *cost;
    int choice;
}FacilityBuyMenuData;

int facility_buy_menu_free(Window *win)
{
    FacilityBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    resources_list_free(data->cost);
    gfc_list_delete(data->list);
    gf2d_window_close_child(win,win->child);
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int facility_buy_menu_draw(Window *win)
{
//    FacilityBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void facility_buy_menu_select_item(Window *win,const char *name)
{
    TextLine buffer;
    SJson *def;
    Element *e;
    Element *cost_list;
    List *resources;
    int energyOutput = 0,energyDraw = 0;
    int staff = 0;
    Bool officer = 0;
    int storage = 0;
    const char *str;
    FacilityBuyMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!name)return;
    if (data->selected == name)return;//nothing to do
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
    
    sj_object_get_value_as_int(def,"staff",&staff);
    gfc_line_sprintf(buffer,"Staff Required:  %i",staff);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);

    sj_object_get_value_as_bool(def,"officerSlot",(short int *)&officer);
    if (officer)
    {
        gfc_line_sprintf(buffer,"Officer Position: Yes");
    }
    else gfc_line_sprintf(buffer,"Officer Position:  No");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"officer"),buffer);

    sj_object_get_value_as_int(def,"storage",&storage);
    gfc_line_sprintf(buffer,"Storage Provided:  %iT",storage);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"storage"),buffer);
    
    sj_object_get_value_as_int(def,"energyOutput",&energyOutput);
    sj_object_get_value_as_int(def,"energyDraw",&energyDraw);
    if (energyOutput > 0 )
        gfc_line_sprintf(buffer,"Energy Ouput: %i",energyOutput);
    else if (energyDraw > 0 )
        gfc_line_sprintf(buffer,"Energy Draw: %i",energyDraw);
    else
        gfc_line_sprintf(buffer,"Energy Use: 0");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),buffer);

    resources_list_free(data->cost);
    data->cost = station_facility_get_resource_cost(sj_object_get_value_as_string(def,"name"),"cost");
    e = gf2d_window_get_element_by_name(win,"costs");
    if (!e)return;
    gf2d_element_list_free_items(e);
    if (data->cost)
    {
        cost_list = resource_list_element_new(win,"cost_list", vector2d(0,0),player_get_resources(),data->cost);
        gf2d_element_list_add_item(e,cost_list);
    }
    resources = station_facility_get_resource_cost(sj_object_get_value_as_string(def,"name"),"upkeep");
    e = gf2d_window_get_element_by_name(win,"upkeep");
    if (!e)return;
    gf2d_element_list_free_items(e);
    if (resources)
    {
        cost_list = resource_list_element_new(win,"cost_list", vector2d(0,0),resources,NULL);
        gf2d_element_list_add_item(e,cost_list);
        resources_list_free(resources);
    }
    resources = station_facility_get_resource_cost(sj_object_get_value_as_string(def,"name"),"produces");
    e = gf2d_window_get_element_by_name(win,"produces");
    if (!e)return;
    gf2d_element_list_free_items(e);
    if (resources)
    {
        cost_list = resource_list_element_new(win,"cost_list", vector2d(0,0),resources,NULL);        
        gf2d_element_list_add_item(e,cost_list);
        resources_list_free(resources);
    }
}

int facility_buy_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    StationFacility *new_facility;
    FacilityBuyMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (FacilityBuyMenuData*)win->data;
    if (!data)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"buy")==0)
        {
            if (win->child)return 1;
            if (resources_list_afford(player_get_resources(),data->cost))
            {
                resource_list_buy(player_get_resources(), data->cost);
                new_facility = station_facility_new_by_name(station_facility_get_name_from_display(data->selected));
                data->parent->facilities = gfc_list_append(data->parent->facilities,new_facility);
                facility_menu_set_list(win->parent);
                gf2d_window_free(win);
            }
            else
            {
                message_new("We require more resources");
            }
            return 1;
        }
        if (strcmp(e->name,"close")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
        if (e->index >= 1000)
        {
            facility_buy_menu_select_item(win,e->name);
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
    return 0;
}

void facility_buy_menu_set_list(Window *win)
{
    const char *str;
    const char *name;
    Element *button;
    Element *item_list;
    int i,c;
    FacilityBuyMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->parent)return;
    item_list = gf2d_window_get_element_by_name(win,"item_list");
    if (!item_list)return;
    c = gfc_list_get_count(data->list);
    for (i = 0; i < c; i++)
    {
        name = gfc_list_get_nth(data->list,i);
        str = station_facility_get_display_name(name);
        if (i == 0)
        {
            facility_buy_menu_select_item(win,str);
        }
        button = gf2d_button_new_label_simple(win,1000+i,str,gfc_color8(255,255,255,255));
        if (!button)continue;
        gf2d_element_list_add_item(item_list,button);
    }
}

Window *facility_buy_menu(Window *parent,StationSection *parentSection,int facility_slot)
{
    Window *win;
    FacilityBuyMenuData* data;
    win = gf2d_window_load("menus/facility_buy.menu");
    if (!win)
    {
        slog("failed to load facility buy menu");
        return NULL;
    }
    win->update = facility_buy_menu_update;
    win->free_data = facility_buy_menu_free;
    win->draw = facility_buy_menu_draw;
    data = (FacilityBuyMenuData*)gfc_allocate_array(sizeof(FacilityBuyMenuData),1);
    win->data = data;
    win->parent = parent;
    data->parent = parentSection;
    data->facility_slot = facility_slot;
    data->list = station_facility_get_possible_list(data->parent);
    facility_buy_menu_set_list(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
