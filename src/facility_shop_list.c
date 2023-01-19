#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"

#include "gf2d_font.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_button.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_message_buffer.h"

#include "config_def.h"
#include "resources.h"
#include "station.h"
#include "player.h"
#include "facility_buy_menu.h"
#include "facility_shop_list.h"

typedef struct
{
    int updated;
    Vector2D position;  //planet position to put it in
    List *list;         // list of possible
    List *facility_list;// which one the new purchase will go to
}FacilityShopListData;

void facility_shop_list_refresh(Window *win);

int facility_shop_list_free(Window *win)
{
    FacilityShopListData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    if (data->list)gfc_list_delete(data->list);
    free(data);
    return 0;
}

int facility_shop_list_update(Window *win,List *updateList)
{
    SJson *def;
    int i,count;
    Element *e;
    FacilityShopListData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (FacilityShopListData*)win->data;

    if (data->updated != player_get_day())
    {
        facility_shop_list_refresh(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 500)
        {
            if (win->child)return 1;
            def = config_def_get_by_parameter("facilities","displayName",e->name);
            if (!def)return 1;
            win->child = facility_buy_menu(
                win,
                sj_object_get_value_as_string(def,"name"),
                data->facility_list,
                data->position);
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


int facility_shop_list_draw(Window *win)
{
//    FacilityShopListData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

Element *facility_shop_list_build_row(Window *win, const char *name,int index)
{
    int count;
    Color color;
    Bool slot = 0;
    SJson *def,*item;
    const char *str;
    TextLine buffer;
    Element *rowList;
    List *costs;
    ListElement *le;
    if ((!win)||(!win->data)||(!name))return NULL;
    def = config_def_get_by_name("facilities",name);
    if (!def)return NULL;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)name,
        gfc_rect(0,0,1,1),
        GFC_COLOR_WHITE,0,
        GFC_COLOR_DARKGREY,index%2,win);
    gf2d_element_make_list(rowList,le);
    //name
    str = sj_object_get_value_as_string(def,"displayName");
    gf2d_element_list_add_item(
        rowList,
        gf2d_button_new_label_simple(
            win,500+index,
            str,
            FT_H6,vector2d(220,24),GFC_COLOR_LIGHTCYAN));
    //type
    str = sj_object_get_value_as_string(def,"type");
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,str,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    //Director
    sj_object_get_value_as_bool(def,"officerSlot",&slot);
    if (slot)
    {
        gfc_line_sprintf(buffer,"available");
    }
    else
    {
        gfc_line_sprintf(buffer,"none");
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //staff
    count = 0;
    sj_object_get_value_as_int(def,"staffPositions",&count);
    gfc_line_sprintf(buffer,"%i",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),GFC_COLOR_WHITE));
    //energy
    if (sj_object_get_value_as_int(def,"energyOutput",&count))
    {
        color = GFC_COLOR_CYAN;
        gfc_line_sprintf(buffer,"+%i",count);
    }
    else if (sj_object_get_value_as_int(def,"energyDraw",&count))
    {
        color = GFC_COLOR_RED;
        gfc_line_sprintf(buffer,"-%i",count);
    }
    else
    {
        color = GFC_COLOR_WHITE;
        gfc_line_sprintf(buffer,"---");
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),color));
    //cargo
    count = 0;
    sj_object_get_value_as_int(def,"storage",&count);
    gfc_line_sprintf(buffer,"%i",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //housing
    count = 0;
    sj_object_get_value_as_int(def,"housing",&count);
    gfc_line_sprintf(buffer,"%i",count);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //cost
    item = sj_object_get_value(def,"cost");
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

void facility_shop_list_refresh(Window *win)
{
    int i,c;
    const char *str;
    int count = 0;
    Element *list,*e;
    FacilityShopListData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    list = gf2d_window_get_element_by_name(win,"item_list");
    gf2d_element_list_free_items(list);
    if (!list)return;
    c = gfc_list_get_count(data->list);
    for (i = 0; i < c; i++)
    {
        str = gfc_list_get_nth(data->list,i);
        if (!str)continue;
        e = facility_shop_list_build_row(win, str,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
        count++;
    }
    data->updated = player_get_day();
}

Window *facility_shop_list(Window *parent,List *list,List *facility_list,Vector2D position)
{
    Window *win;
    FacilityShopListData *data;

    win = gf2d_window_load("menus/facility_shop_list.menu");
    if (!win)
    {
        slog("failed to load faciilty shop list window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(FacilityShopListData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = facility_shop_list_update;
    win->free_data = facility_shop_list_free;
    win->draw = facility_shop_list_draw;
    win->refresh = facility_shop_list_refresh;
    data->facility_list = facility_list;
    data->list = list;
    data->position = position;

    facility_shop_list_refresh(win);
    message_buffer_bubble();
    return win;
}
