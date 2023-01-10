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
#include "ship_view_menu.h"

extern int freeBuildMode;

typedef struct
{
    TextLine name;
    int offset;
    int scrollCount;
    Ship *ship;
}ShipViewMenuData;

int ship_view_menu_free(Window *win)
{
    ShipViewMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int ship_view_menu_draw(Window *win)
{
//    ShipViewMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void onShipRenameCancel(Window *win)
{
    ShipViewMenuData* data;
    if ((!win)||(!win->data))return;
    data = win->data;
    gfc_line_cpy(data->name,data->ship->displayName);
    return;
}

void onShipRenameOk(Window *win)
{
    ShipViewMenuData* data;
    if ((!win)||(!win->data))return;
    data = win->data;
    gfc_line_cpy(data->ship->displayName,data->name);
    gf2d_window_refresh(win);
    gf2d_window_refresh(win->parent);
    return;
}

void ship_view_scroll_up(Window *win)
{
    ShipViewMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset > 0)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"facilities_list"),--data->offset);
        gf2d_window_refresh(win);
    }
}

void ship_view_scroll_down(Window *win)
{
    ShipViewMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset < data->scrollCount)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"facilities_list"),++data->offset);
        gf2d_window_refresh(win);
    }
}


void ship_view_menu_refresh(Window *win)
{
    SJson *def;
    Color color;
    ShipFacility *facility;
    int i,c;
    TextLine buffer;
    const char *str;
    Element *elist;
    ShipViewMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->ship)return;
    def = config_def_get_by_name("ships",data->ship->name);
    //icon
    str = sj_object_get_value_as_string(def,"icon");
    gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    //name
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),data->ship->displayName);
    //model
    gfc_line_sprintf(buffer,"Model: %s",data->ship->name);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"model"),buffer);
    //description
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    //captain
    if (strlen(data->ship->captain)> 0)gfc_line_cpy(buffer,data->ship->captain);
    else gfc_line_sprintf(buffer,"<none>");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"captain"),buffer);
    //staff
    if (data->ship->staffAssigned < data->ship->staffRequired)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gfc_line_sprintf(buffer,"%i/%i",data->ship->staffAssigned,data->ship->staffPositions);
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),color);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
    //hull
    if (data->ship->hull < data->ship->hullMax)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gfc_line_sprintf(buffer,"Hull: %i/%i",(int)data->ship->hull,(int)data->ship->hullMax);
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"hull"),color);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"hull"),buffer);
    //mission
    if (!data->ship->mission)
    {
        gfc_line_sprintf(buffer,"Mission: <unassigned>");
    }
    else
    {
        gfc_line_sprintf(buffer,"Mission: %s",data->ship->mission->title);
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"mission"),buffer);
    //storage
    gfc_line_sprintf(buffer,"Cargo: %i/%i T",resources_get_total_commodity_mass(data->ship->cargo),data->ship->storageCapacity);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cargo"),buffer);
    //energy
    if (data->ship->energyDraw > data->ship->energyOutput)color = GFC_COLOR_RED;
    else color = GFC_COLOR_CYAN;
    gfc_line_sprintf(buffer,"Energy: %i/%i",(int)data->ship->energyDraw,(int)data->ship->energyOutput);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),buffer);
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"energy"),color);
    //efficiency
    gfc_line_sprintf(buffer,"Efficiency: %.2f%%",data->ship->efficiency);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"efficiency"),buffer);
    //passengers
    gfc_line_sprintf(buffer,"Passengers: %i/%i",data->ship->passengers,data->ship->housing);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"passengers"),buffer);
    //facilities
    elist = gf2d_window_get_element_by_name(win,"facilities_list");
    if (elist)
    {
        c = gfc_list_get_count(data->ship->facilities);
        for (i = 0; i < c; i++)
        {
            facility = gfc_list_get_nth(data->ship->facilities,i);
            if (!facility)continue;
            gf2d_element_list_add_item(
                elist,
                gf2d_label_new_simple_size(win,0,facility->displayName,FT_H6,vector2d(1,24),
                GFC_COLOR_WHITE));
        }
        if (c > gf2d_element_list_get_row_count(elist))
        {
            data->scrollCount = c - gf2d_element_list_get_row_count(elist);
        }
        else data->scrollCount = 0;
    }
}

int ship_view_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlayerData *player;
    ShipViewMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (ShipViewMenuData*)win->data;
    if (!data)return 0;
    player = player_get_data();
    if (!player)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"staff_assign")==0)
        {
            if (!data->ship)return 1;//nothing selected
            if (data->ship->mission != NULL)
            {
                message_new("Cannot assign any staff while the ship is on a mission");
                return 1;
            }
            if (data->ship->staffAssigned < data->ship->staffPositions)
            {
                if (player->staff <= 0)
                {
                    message_new("Cannot assign any more staff.  Please hire more.");
                    return 1;
                }
                if (ship_change_staff(data->ship,1) == 0)
                {
                    player->staff--;
                    gf2d_window_refresh(win);
                    gf2d_window_refresh(win->parent);
                }
            }
            return 1;
        }
        if (strcmp(e->name,"staff_remove")==0)
        {
            if (!data->ship)return 1;//nothing selected
            if (data->ship->mission != NULL)
            {
                message_new("Cannot remove any staff while the ship is on a mission");
                return 1;
            }
            if (data->ship->staffAssigned > 0)
            {
                if (ship_change_staff(data->ship,-1) == 0)
                {
                    player->staff++;
                    gf2d_window_refresh(win);
                    gf2d_window_refresh(win->parent);
                }
            }
            return 1;
        }
        if (strcmp(e->name,"rename")==0)
        {
            gfc_line_cpy(data->name,data->ship->displayName);
            window_text_entry(
                "Rename Ship",
                data->name,
                win,
                GFCLINELEN,
                (gfc_work_func*)onShipRenameOk,
                (gfc_work_func*)onShipRenameCancel);
            return 1;
        }
        if (strcmp(e->name,"scroll_down")==0)
        {
            ship_view_scroll_down(win);
            return 1;
        }
        if (strcmp(e->name,"scroll_up")==0)
        {
            ship_view_scroll_up(win);
            return 1;
        }
        if (strcmp(e->name,"close")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    if (gf2d_window_mouse_in(win))
    {
        if (gfc_input_mouse_wheel_up())
        {
            ship_view_scroll_up(win);
            return 1;
        }
        if (gfc_input_mouse_wheel_down())
        {
            ship_view_scroll_down(win);
            return 1;
        }
    }
    return 0;
}

Window *ship_view_menu(Window *parent,Ship *ship)
{
    Window *win;
    ShipViewMenuData* data;
    win = gf2d_window_load("menus/ship_view.menu");
    if (!win)
    {
        slog("failed to load ship view menu");
        return NULL;
    }
    data = (ShipViewMenuData*)gfc_allocate_array(sizeof(ShipViewMenuData),1);
    win->parent = parent;
    win->data = data;
    //setup callbacks
    win->update = ship_view_menu_update;
    win->free_data = ship_view_menu_free;
    win->refresh = ship_view_menu_refresh;
    win->draw = ship_view_menu_draw;
    //setup this window
    data->ship = ship;
    ship_view_menu_refresh(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
