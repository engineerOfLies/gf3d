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

#include "player.h"
#include "config_def.h"
#include "resources.h"
#include "work_menu.h"
#include "ship.h"
#include "ship_facility.h"
#include "ship_facility_view.h"

extern int freeBuildMode;

typedef struct
{
    TextLine name;
    Ship *ship;
    ShipFacility *facility;
}ShipFacilityViewData;

int ship_facility_view_free(Window *win)
{
    ShipFacilityViewData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int ship_facility_view_draw(Window *win)
{
//    ShipFacilityViewData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void ship_facility_view_refresh(Window *win)
{
    int amount;
    SJson *def;
    Color color;
    TextLine buffer;
    const char *str;
    ShipFacilityViewData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->ship)return;
    def = config_def_get_by_name("ship_facilities",data->facility->name);
    //icon
    str = sj_object_get_value_as_string(def,"icon");
    gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    //name
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),data->facility->displayName);
    //facility_type
    str = sj_object_get_value_as_string(def,"slot_type");
    gfc_line_sprintf(buffer,"type: %s",str);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"facility_type"),buffer);
    //description
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    //staff
    if (sj_object_get_value_as_int(def,"staffPositions",&amount))
    {
        gfc_line_sprintf(buffer,"Staff Position: %i",amount);
    }
    else
    {
        gfc_line_sprintf(buffer,"Staff Position: <none>");
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
    //damage
    if (data->facility->damage > 0)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gfc_line_sprintf(buffer,"Damage: %.2f%%",data->facility->damage);
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"damage"),color);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"damage"),buffer);
    //storage
    if (sj_object_get_value_as_int(def,"cargo",&amount))
    {
        gfc_line_sprintf(buffer,"Cargo Space: %iT",amount);
    }
    else
    {
        gfc_line_sprintf(buffer,"Cargo Space: <none>");
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cargo"),buffer);
    //energy
    if (sj_object_get_value_as_int(def,"energyDraw",&amount))
    {
        gfc_line_sprintf(buffer,"Energy Draw: %i",amount);
    }
    else if (sj_object_get_value_as_int(def,"energyOutput",&amount))
    {
        gfc_line_sprintf(buffer,"Energy Output: %i",amount);
    }
    else
    {
        gfc_line_sprintf(buffer,"Energy Draw: <none>");
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),buffer);
    //speed
    if (sj_object_get_value_as_int(def,"speed",&amount))
    {
        gfc_line_sprintf(buffer,"Speed: %i",amount);
    }
    else
    {
        gfc_line_sprintf(buffer,"Speed: <n/a>");
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"speed"),buffer);    //efficiency
    //passengers
    if (sj_object_get_value_as_int(def,"housing",&amount))
    {
        gfc_line_sprintf(buffer,"Passenger Cabins: %i",amount);
    }
    else
    {
        gfc_line_sprintf(buffer,"Passenger Cabins: <n/a>");
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"passengers"),buffer);    //efficiency
}

void ship_facility_remove_cancel(Window *win)
{
    if ((!win)||(!win->data))return;
    win->child = NULL;
}

void ship_facility_remove_ok(Window *win)
{
    SJson *def;
    List *cost;
    ShipFacilityViewData* data;
    if ((!win)||(!win->data))return;
    data = win->data;
    win->child = NULL;
    def = config_def_get_by_name("ship_facilities",data->facility->name);
    if (!def)return;
    cost = resources_list_parse(sj_object_get_value(def,"cost"));
    resource_list_sell(player_get_resources(), cost,0.9);
    resources_list_free(cost);
    ship_remove_facility(data->ship, data->facility->id);
    gf2d_window_free(win);
    gf2d_window_refresh_by_name("ship_view_menu");
}

int ship_facility_view_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlayerData *player;
    ShipFacilityViewData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (ShipFacilityViewData*)win->data;
    if (!data)return 0;
    player = player_get_data();
    if (!player)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"sell")==0)
        {
            if (win->child)return 1;
            win->child = window_yes_no("Sell Facility?", (gfc_work_func*)ship_facility_remove_ok,(gfc_work_func*)ship_facility_remove_cancel,win);
            return 1;
        }        
        if (strcmp(e->name,"close")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return 0;
}

Window *ship_facility_view(Window *parent,Ship *ship, ShipFacility *facility)
{
    Window *win;
    ShipFacilityViewData* data;
    win = gf2d_window_load("menus/ship_facility_view.menu");
    if (!win)
    {
        slog("failed to load ship facility view menu");
        return NULL;
    }
    data = (ShipFacilityViewData*)gfc_allocate_array(sizeof(ShipFacilityViewData),1);
    win->parent = parent;
    win->data = data;
    //setup callbacks
    win->update = ship_facility_view_update;
    win->free_data = ship_facility_view_free;
    win->refresh = ship_facility_view_refresh;
    win->draw = ship_facility_view_draw;
    //setup this window
    data->ship = ship;
    data->facility = facility;
    ship_facility_view_refresh(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
