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
#include "ship_facility_buy_menu.h"

extern int freeBuildMode;

typedef struct
{
    TextLine name;
    Ship *ship;
    const char *facility;
    List *cost;
}ShipFacilityBuyMenueData;

int ship_facility_buy_menu_free(Window *win)
{
    ShipFacilityBuyMenueData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    if (data->cost)resources_list_free(data->cost);
    free(data);
    return 0;
}

int ship_facility_buy_menu_draw(Window *win)
{
//    ShipFacilityBuyMenueData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void ship_facility_buy_menu_refresh(Window *win)
{
    int amount;
    SJson *def;
    Element *cost_list,*e;
    TextLine buffer;
    const char *str;
    ShipFacilityBuyMenueData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->ship)return;
    def = config_def_get_by_name("ship_facilities",data->facility);
    //icon
    str = sj_object_get_value_as_string(def,"icon");
    gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    //name
    str = sj_object_get_value_as_string(def,"displayName");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),str);
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
    //cost
    if (data->cost)resources_list_free(data->cost);
    data->cost = resources_list_parse(sj_object_get_value(def,"cost"));
    if (data->cost)
    {
        cost_list = resource_list_element_new(win,"cost_list", vector2d(0,0),player_get_resources(),data->cost,NULL);
        
        e = gf2d_window_get_element_by_name(win,"costs");
        if (!e)return;
        gf2d_element_list_free_items(e);
        gf2d_element_list_add_item(e,cost_list);
    }
}

void ship_facility_buy_cancel(Window *win)
{
    if(!win)return;
    win->child = NULL;
}

void ship_facility_buy_ok(Window *win)
{
    Window *parent;
    ShipFacilityBuyMenueData* data;
    if ((!win)||(!win->data))return;
    data = win->data;
    win->child = NULL;
    resource_list_buy(player_get_resources(), data->cost);
    ship_give_new_facility(data->ship, data->facility);
    parent = win->parent;
    gf2d_window_free(win);    
    gf2d_window_free(parent);
    gf2d_window_refresh_by_name("ship_view_menu");
}

int ship_facility_buy_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlayerData *player;
    ShipFacilityBuyMenueData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (ShipFacilityBuyMenueData*)win->data;
    if (!data)return 0;
    player = player_get_data();
    if (!player)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"purchase")==0)
        {
            if (win->child)return 1;
            if (!resources_list_afford(player_get_resources(), data->cost))
            {
                message_new("You do not have enough resources for this.");
                return 1;
            }
            win->child = window_yes_no("Purchase?", (gfc_work_func*)ship_facility_buy_ok,(gfc_work_func*)ship_facility_buy_cancel,win);
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

Window *ship_facility_buy_menu(Window *parent,Ship *ship, const char *facility)
{
    Window *win;
    ShipFacilityBuyMenueData* data;
    win = gf2d_window_load("menus/ship_facility_buy.menu");
    if (!win)
    {
        slog("failed to load ship facility view menu");
        return NULL;
    }
    data = (ShipFacilityBuyMenueData*)gfc_allocate_array(sizeof(ShipFacilityBuyMenueData),1);
    win->parent = parent;
    win->data = data;
    //setup callbacks
    win->update = ship_facility_buy_menu_update;
    win->free_data = ship_facility_buy_menu_free;
    win->refresh = ship_facility_buy_menu_refresh;
    win->draw = ship_facility_buy_menu_draw;
    //setup this window
    data->ship = ship;
    data->facility = facility;
    ship_facility_buy_menu_refresh(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
