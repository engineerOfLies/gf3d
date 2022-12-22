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
    StationFacility *facility;
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

void facility_menu_select_item(Window *win,int choice)
{
    SJson *def;
    TextLine buffer;
    const char *str,*name;
    Element *e;
    Element *cost_list;
    List *resources;
    FacilityMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    data->facility = gfc_list_get_nth(data->parent->facilities,choice);
    if (!data->facility)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),"Empty Slot");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),"This slot is free for new facility installation");
        gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),NULL);
        gf2d_element_list_free_items(gf2d_window_get_element_by_name(win,"produces"));
        gf2d_element_list_free_items(gf2d_window_get_element_by_name(win,"upkeep"));
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),"Staff: 0 / 0");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),"Energy Use: 0");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"active"),"Active: No");
        return;
    }
    
    if ((!data->facility->inactive)&&(!data->facility->disabled))
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"active"),GFC_WHITE);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"active"),"Active: Yes");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"disable_label"),"Disable");
    }
    else
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"active"),GFC_RED);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"active"),"Active: No");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"disable_label"),"Enable");
    }
    
    name = data->selected = station_facility_get_display_name(data->facility->name);
    gfc_line_sprintf(buffer,"Staff: %i / %i",data->facility->staffAssigned,data->facility->staffPositions);
    if (data->facility->staffAssigned < data->facility->staffRequired)
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_RED);
    }
    else
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_WHITE);
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);

    if (data->facility->energyOutput > 0 )
        gfc_line_sprintf(buffer,"Energy Ouput: %i",data->facility->energyOutput);
    else if (data->facility->energyDraw > 0 )
        gfc_line_sprintf(buffer,"Energy Draw: %i",data->facility->energyDraw);
    else
        gfc_line_sprintf(buffer,"Energy Use: 0");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),buffer);
    
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

    resources = station_facility_get_resource_cost(sj_object_get_value_as_string(def,"name"),"upkeep");
    e = gf2d_window_get_element_by_name(win,"upkeep");
    gf2d_element_list_free_items(e);
    if (resources)
    {
        cost_list = resource_list_element_new(win,"upkeep_list", vector2d(0,0),resources,NULL);
        gf2d_element_list_add_item(e,cost_list);
        resources_list_free(resources);
    }
    resources = station_facility_get_resource_cost(sj_object_get_value_as_string(def,"name"),"produces");
    e = gf2d_window_get_element_by_name(win,"produces");
    gf2d_element_list_free_items(e);
    if (resources)
    {
        cost_list = resource_list_element_new(win,"produces_list", vector2d(0,0),resources,NULL);        
        gf2d_element_list_add_item(e,cost_list);
        resources_list_free(resources);
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
    cost = station_facility_get_resource_cost(station_facility_get_name_from_display(data->selected),"cost");
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
    PlayerData *player;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (FacilityMenuData*)win->data;
    if (!data)return 0;
    player = player_get_data();
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
        if (strcmp(e->name,"staff_assign")==0)
        {
            if (!data->facility)return 1;//nothing selected
            if (data->facility->staffAssigned < data->facility->staffPositions)
            {
                if (player->staff <= 0)
                {
                    message_new("Cannot assign any more staff.  Please hire more.");
                    return 1;
                }
                if (station_facility_change_staff(data->facility,1) == 0)
                {
                    player->staff--;
                    facility_menu_select_item(win,data->choice);//this will redraw
                }
            }
            return 1;
        }
        if (strcmp(e->name,"staff_remove")==0)
        {
            if (!data->facility)return 1;//nothing selected
            if (data->facility->staffAssigned > 0)
            {
                if (station_facility_change_staff(data->facility,-1) == 0)
                {
                    player->staff++;
                    facility_menu_select_item(win,data->choice);//this will redraw
                }
            }
            return 1;
        }
        if (strcmp(e->name,"disable")==0)
        {
            station_facility_check(data->facility);
            if (data->facility->inactive)
            {
                message_new("Cannot Enable Facility");
                return 1;
            }
            data->facility->disabled = !data->facility->disabled;
            facility_menu_select_item(win,data->choice);//this will redraw
            return 1;
        }
        if (strcmp(e->name,"buy")==0)
        {
            win->child = facility_buy_menu(win,data->parent,data->choice);
            return 1;
        }
        if (strcmp(e->name,"sell")==0)
        {
            win->child = window_yes_no("Sell selected facility?", facility_menu_yes,facility_menu_no,win);
            return 1;
        }
        if (e->index >= 1000)
        {
            data->choice = e->index - 1000;
            facility_menu_select_item(win,data->choice);
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
        button = gf2d_button_new_label_simple(win,1000+i,str,gfc_color8(255,255,255,255));
        if (!button)continue;
        gf2d_element_list_add_item(item_list,button);
    }
    facility_menu_select_item(win,0);
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
