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
#include "ship_view_menu.h"
#include "ship_facility_buy_menu.h"
#include "shipyard_facilities_menu.h"

typedef struct
{
    const char *filter;
    Ship *ship;
    int updated;
}ShipyardFacilitiesMenuData;

void shipyard_facilities_menu_refresh(Window *win);

int shipyard_facilities_menu_free(Window *win)
{
    ShipyardFacilitiesMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int shipyard_facilities_menu_facility_check()
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

int shipyard_facilities_menu_update(Window *win,List *updateList)
{
    int i,count;
    SJson *def;
    const char *name;
    Element *e;
    ShipyardFacilitiesMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (ShipyardFacilitiesMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        shipyard_facilities_menu_refresh(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 500)
        {
            if (win->child)return 1;
            def = config_def_get_by_parameter("ship_facilities","displayName",e->name);
            if (!def)return 1;
            name = sj_object_get_value_as_string(def,"name");
            if (!name)return 1;
            win->child = ship_facility_buy_menu(win,data->ship, name);
            return 1;
        }
        if (strcmp(e->name,"done")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return 0;
}


int shipyard_facilities_menu_draw(Window *win)
{
//    ShipyardFacilitiesMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

Element *shipyard_facilities_menu_build_row(Window *win, SJson *facility,int index)
{
    int count;
    SJson *item;
    const char *str;
    TextLine buffer;
    Element *rowList;
    List *costs;
    ListElement *le;
    if ((!win)||(!win->data)||(!facility))return NULL;
    
    str = sj_object_get_value_as_string(facility,"name");
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)str,
        gfc_rect(0,0,1,1),
        GFC_COLOR_WHITE,0,
        GFC_COLOR_DARKGREY,index%2,win);
    gf2d_element_make_list(rowList,le);
    //name
    str = sj_object_get_value_as_string(facility,"displayName");
    gf2d_element_list_add_item(
        rowList,
        gf2d_button_new_label_simple(
            win,500+index,
            str,
            FT_H6,vector2d(180,24),GFC_COLOR_LIGHTCYAN));
    //size
    str = sj_object_get_value_as_string(facility,"slot_type");
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,str,FT_H6,vector2d(130,24),GFC_COLOR_WHITE));
    //crew
    sj_object_get_value_as_int(facility,"staffPositions",&count);
    gfc_line_sprintf(buffer,"%i",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),GFC_COLOR_WHITE));
    //speed
    sj_object_get_value_as_int(facility,"speed",&count);
    gfc_line_sprintf(buffer,"%i Mm/s",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //damage
    str = sj_object_get_value_as_string(facility,"damageType");
    sj_object_get_value_as_int(facility,"damage",&count);
    if (str)
    {
        gfc_line_sprintf(buffer,"%i %s",count,str);
    }
    else gfc_line_sprintf(buffer,"<n/a>");
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    //cargo
    count = 0;
    sj_object_get_value_as_int(facility,"cargo",&count);
    gfc_line_sprintf(buffer,"%i T",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //cost
    item = sj_object_get_value(facility,"cost");
    costs = resources_list_parse(item);
    if (costs)
    {
        count = (int)resources_list_get_amount(costs,"credits");
        gfc_line_sprintf(buffer,"%i Cr",count);
        resources_list_free(costs);
    }
    else
    {
        gfc_line_sprintf(buffer,"<see details>");
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),GFC_COLOR_WHITE));
    return rowList;
}

void shipyard_facilities_menu_refresh(Window *win)
{
    int i,c;
    const char *str;
    SJson *facility;
    int count = 0;
    PlayerData *player;
    Element *list,*e;
    ShipyardFacilitiesMenuData *data;
    player = player_get_data();
    if ((!win)||(!win->data)||(!player))return;
    data = win->data;
    list = gf2d_window_get_element_by_name(win,"item_list");
    gf2d_element_list_free_items(list);
    if (!list)return;
    c = config_def_get_resource_count("ship_facilities");
    for (i = 0; i < c; i++)
    {
        facility = config_def_get_by_index("ship_facilities",i);
        if (!facility)continue;
        str = sj_object_get_value_as_string(facility,"slot_type");
        if (!str)continue;
        if (gfc_strlcmp(str,data->filter)!=0)continue;//skip those that dont match
        e = shipyard_facilities_menu_build_row(win, facility,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
        count++;
    }
    data->updated = player_get_day();
}

Window *shipyard_facilities_menu(Window *parent,Ship *ship,const char *filter)
{
    Window *win;
    ShipyardFacilitiesMenuData *data;

    win = gf2d_window_load("menus/shipyard_facility.menu");
    if (!win)
    {
        slog("failed to load shipyard facilities window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(ShipyardFacilitiesMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = shipyard_facilities_menu_update;
    win->free_data = shipyard_facilities_menu_free;
    win->draw = shipyard_facilities_menu_draw;
    win->refresh = shipyard_facilities_menu_refresh;
    data->filter = filter;
    data->ship = ship;
    shipyard_facilities_menu_refresh(win);
    message_buffer_bubble();
    return win;
}
