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
#include "ship_buy_menu.h"

extern int freeBuildMode;

typedef struct
{
    const char *ship;
    List *cost;
}ShipBuyMenuData;

int ship_buy_menu_free(Window *win)
{
    ShipBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    if (data->cost)resources_list_free(data->cost);
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int ship_buy_menu_draw(Window *win)
{
//    ShipBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void ship_buy_menu_refresh(Window *win)
{
    int count,limit;
    SJson *def,*facDef,*list,*item;
    TextLine buffer;
    const char *str;
    const char *search;
    const char *s;
    Element *elist,*cost_list,*e;
    int i,c;
    int j,k;
    ShipBuyMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->ship)return;
    def = config_def_get_by_name("ships",data->ship);
    //icon
    str = sj_object_get_value_as_string(def,"icon");
    gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    //name
    str = sj_object_get_value_as_string(def,"displayName");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),str);
    //description
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    //officers
    sj_object_get_value_as_int(def,"officers",&count);
    gfc_line_sprintf(buffer,"Officer Positions: %i",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"officers"),buffer);
    //crew
    sj_object_get_value_as_int(def,"crew",&count);
    gfc_line_sprintf(buffer,"Crew Positions: %i",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"crew"),buffer);
    //hull
    sj_object_get_value_as_int(def,"hull",&count);
    gfc_line_sprintf(buffer,"Hull: %i",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"hull"),buffer);
    //storage
    count = 0;
    sj_object_get_value_as_int(def,"cargo",&count);
    gfc_line_sprintf(buffer,"Cargo: %iT",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cargo"),buffer);
    //speed
    count = 0;
    sj_object_get_value_as_int(def,"speed",&count);
    gfc_line_sprintf(buffer,"Speed: %i Mm/s",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"speed"),buffer);
    //passengers
    count = 0;
    sj_object_get_value_as_int(def,"passengers",&count);
    gfc_line_sprintf(buffer,"Passenger Space: %i Mm/s",count);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"passengers"),buffer);
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
    //facilities
    elist = gf2d_window_get_element_by_name(win,"facilities_list");
    if (elist)
    {
        gf2d_element_list_free_items(elist);
        k = ship_name_get_slot_name_count(data->ship);
        for (j = 0; j < k;j++)
        {
            str = ship_name_get_slot_name_by_index(data->ship, j);
            if (!str)continue;
            limit = ship_name_get_slot_count_by_type(data->ship,str);
            gfc_line_sprintf(buffer,"%s slots: %i",str,limit);
            gf2d_element_list_add_item(
                elist,
                gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(1,24),
                GFC_COLOR_LIGHTBLUE));

            count = 0;
            list = sj_object_get_value(def,"default_facilities");
            c = sj_array_get_count(list);
            for (i = 0; i < c; i++)
            {
                item = sj_array_get_nth(list,i);
                if (!item)continue;
                search = sj_get_string_value(item);
                facDef = config_def_get_by_name("ship_facilities",search);
                s = sj_object_get_value_as_string(facDef,"slot_type");
                if (gfc_strlcmp(s,str)!= 0)continue;//skip, not a match
                s = sj_object_get_value_as_string(facDef,"displayName");
                
                gf2d_element_list_add_item(
                    elist,
                    gf2d_button_new_label_simple(
                        win,500+i,
                        s,
                        FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
                count++;
            }
            for (i = count; i < limit; i++)
            {
                gf2d_element_list_add_item(
                    elist,
                    gf2d_button_new_label_simple(
                        win,600+j,
                        "<empty>",
                        FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
            }
        }
    }
}

void ship_buy_ok(Window *win)
{
    Window *parent;
    ShipBuyMenuData* data;
    if ((!win)||(!win->data))return;
    win->child = NULL;
    data = win->data;
    resource_list_buy(player_get_resources(), data->cost);
    player_give_new_ship(data->ship);
    parent = win->parent;
    gf2d_window_free(win);    
    gf2d_window_free(parent);
    gf2d_window_refresh_by_name("ship_list_view");
}

void ship_buy_cancel(Window *win)
{
    if ((!win)||(!win->data))return;
    win->child = NULL;
}

int ship_buy_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlayerData *player;
    ShipBuyMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (ShipBuyMenuData*)win->data;
    if (!data)return 0;
    player = player_get_data();
    if (!player)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"buy")==0)
        {
            if (win->child)return 1;
            if (!resources_list_afford(player_get_resources(), data->cost))
            {
                message_new("You do not have enough resources for this.");
                return 1;
            }
            win->child = window_yes_no("Purchase Ship?", (gfc_work_func*)ship_buy_ok,(gfc_work_func*)ship_buy_cancel,win);
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

Window *ship_buy_menu(Window *parent,const char*ship)
{
    Window *win;
    ShipBuyMenuData* data;
    win = gf2d_window_load("menus/ship_buy.menu");
    if (!win)
    {
        slog("failed to load ship view menu");
        return NULL;
    }
    data = (ShipBuyMenuData*)gfc_allocate_array(sizeof(ShipBuyMenuData),1);
    win->parent = parent;
    win->data = data;
    //setup callbacks
    win->update = ship_buy_menu_update;
    win->free_data = ship_buy_menu_free;
    win->refresh = ship_buy_menu_refresh;
    win->draw = ship_buy_menu_draw;
    //setup this window
    data->ship = ship;
    ship_buy_menu_refresh(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
