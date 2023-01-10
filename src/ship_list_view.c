#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_button.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"
#include "gf2d_windows_common.h"

#include "gf3d_entity.h"
#include "config_def.h"
#include "camera_entity.h"
#include "resources.h"
#include "station_def.h"
#include "station.h"
#include "player.h"
#include "ship_list_view.h"

typedef struct
{
    TextLine entryText;
    int updated;
    int offset;
    int scrollCount;
    List *ships;
}ShipListViewData;

void ship_list_view_update_resources(Window *win);

int ship_list_view_free(Window *win)
{
    ShipListViewData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

void ship_list_view_scroll_up(Window *win)
{
    ShipListViewData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset > 0)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"commodities"),--data->offset);
        ship_list_view_update_resources(win);
    }
}

void ship_list_view_scroll_down(Window *win)
{
    ShipListViewData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset < data->scrollCount)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"commodities"),++data->offset);
        ship_list_view_update_resources(win);
    }
}

int ship_list_view_facility_check()
{
    StationFacility *facility;
    facility = player_get_facility_by_name("commodities_market");
    if (facility == NULL)
    {
        message_printf("Commodities Market is not available, please install the facility to the station");
        return 0;
    }
    else if ((facility->inactive)||(facility->disabled))
    {
        message_printf("Commodities Market facility is not functioning.  Cannot buy or sell goods");
        return 0;
    }
    if (!player_has_working_dock())
    {
        message_printf("Cannot purchase goods on the open market, functioning dock is required!");
        return 0;
    }
    return 1;
}

int ship_list_view_update(Window *win,List *updateList)
{
    int i,count;
//    int choice;
//    const char *name;
//    TextLine buffer;
    Element *e;
    ShipListViewData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (ShipListViewData*)win->data;

    if (data->updated != player_get_day())
    {
        ship_list_view_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"scroll_down")==0)
        {
            ship_list_view_scroll_down(win);
            return 1;
        }
        if (strcmp(e->name,"scroll_up")==0)
        {
            ship_list_view_scroll_up(win);
            return 1;
        }
        if ((e->index >= 500)&&(e->index < 600))
        {
            return 1;
        }
        if (strcmp(e->name,"done")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    if (gf2d_window_mouse_in(win))
    {
        if (gfc_input_mouse_wheel_up())
        {
            ship_list_view_scroll_up(win);
            return 1;
        }
        if (gfc_input_mouse_wheel_down())
        {
            ship_list_view_scroll_down(win);
            return 1;
        }
    }
    return 0;
}


int ship_list_view_draw(Window *win)
{
//    ShipListViewData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

Element *ship_list_view_build_row(Window *win, Ship *ship,int index)
{
    TextLine buffer;
    Color color;
    Element *rowList;
    ListElement *le;
    PlayerData *player;
    if ((!win)||(!win->data)||(!ship))return NULL;
    player = player_get_data();
    if (!player)return NULL;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)ship->displayName,
        gfc_rect(0,0,1,1),
        GFC_COLOR_WHITE,0,
        GFC_COLOR_DARKGREY,index%2,win);
    gf2d_element_make_list(rowList,le);
    //name
    if (ship->disabled)color = GFC_COLOR_RED;
    else color = gfc_color(0.9,0.9,0.8,1);
    gf2d_element_list_add_item(
        rowList,
        gf2d_button_new_label_simple(
            win,500+index,
            ship->displayName,
            FT_H6,vector2d(200,24),color));
    //captain
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,ship->captain,FT_H6,vector2d(150,24),GFC_COLOR_WHITE));
    //HULL
    if (ship->hull < ship->hullMax)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gfc_line_sprintf(buffer,"%i/%i",(int)ship->hull,(int)ship->hullMax);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),color));
    //staff
    if (ship->staffAssigned < ship->staffRequired)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gfc_line_sprintf(buffer,"%i/%i",(int)ship->staffAssigned,(int)ship->staffPositions);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    //location
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,ship->location,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    //mission
    if (!ship->mission)
    {
        gfc_line_sprintf(buffer,"<unassigned>");
    }
    else
    {
        gfc_line_sprintf(buffer,"%s",ship->mission->title);
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    
    return rowList;
}

void ship_list_view_update_resources(Window *win)
{
    int i,c;
    int count = 0;
    Ship *ship;
    PlayerData *player;
    Element *list,*e;
    ShipListViewData *data;
    player = player_get_data();
    if ((!win)||(!win->data)||(!player))return;
    data = win->data;
    data->ships = player->ships;
    list = gf2d_window_get_element_by_name(win,"item_list");
    gf2d_element_list_free_items(list);
    if (!list)return;
    c = gfc_list_get_count(data->ships);
    for (i = 0; i < c; i++)
    {
        ship = gfc_list_get_nth(data->ships,i);
        e = ship_list_view_build_row(win, ship,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
        count++;
    }
    if (count > gf2d_element_list_get_row_count(list))
    {
        data->scrollCount = count - gf2d_element_list_get_row_count(list);
    }
    else data->scrollCount = 0;

    data->updated = player_get_day();
}

Window *ship_list_view(Window *parent)
{
    Window *win;
    ShipListViewData *data;

    win = gf2d_window_load("menus/ship_list.menu");
    if (!win)
    {
        slog("failed to load ship list window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(ShipListViewData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = ship_list_view_update;
    win->free_data = ship_list_view_free;
    win->draw = ship_list_view_draw;
    ship_list_view_update_resources(win);
    message_buffer_bubble();
    return win;
}
