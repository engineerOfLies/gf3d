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
#include "station_def.h"
#include "station.h"
#include "station_menu.h"
#include "work_menu.h"
#include "station_extension_menu.h"
#include "station_buy_menu.h"

extern int freeBuildMode;

typedef struct
{
    const char *mountType;
    StationSection *parent;
    StationData *station;
    const char *selected;
    List *cost;
    Uint8 slot;
    int choice;
    List *list;
}StationBuyMenuData;

int station_buy_menu_free(Window *win)
{
    StationBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    resources_list_free(data->cost);
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int station_buy_menu_draw(Window *win)
{
//    StationBuyMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void station_buy_menu_select_item(Window *win,int choice, const char *name)
{
    SJson *def;
    Element *e;
    Element *cost_list;
    const char *str;
    StationBuyMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (choice != data->choice)
    {
        gf2d_element_set_color(gf2d_window_get_element_by_id(win,data->choice + 1000),GFC_COLOR_YELLOW);
    }
    gf2d_element_set_color(gf2d_window_get_element_by_id(win,choice + 1000),GFC_COLOR_CYAN);
    data->choice = choice;
    if (data->selected == name)return;//nothing to do
    
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),name);
    def = config_def_get_by_parameter("sections","display_name",name);
    if (!def)return;
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    data->selected = name;
    str = sj_object_get_value_as_string(def,"icon");
    if (str)
    {
        gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    }
    resources_list_free(data->cost);
    data->cost = station_get_resource_cost(sj_object_get_value_as_string(def,"name"));
    if (data->cost)
    {
        cost_list = resource_list_element_new(win,"cost_list", vector2d(0,0),player_get_resources(),data->cost,NULL);
        
        e = gf2d_window_get_element_by_name(win,"costs");
        if (!e)return;
        gf2d_element_list_free_items(e);
        gf2d_element_list_add_item(e,cost_list);
    }
}

int station_buy_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    StationBuyMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (StationBuyMenuData*)win->data;
    if (!data)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"cancel")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
        if (strcmp(e->name,"buy")==0)
        {
            if (win->child)return 1;
            if (resources_list_afford(player_get_resources(),data->cost))
            {
                win->child = work_menu(
                    win,
                    NULL,
                    data->parent,
                    NULL,
                    "build_section",
                    station_def_get_name_by_display(data->selected),
                    vector2d(data->slot,0));
/*                resource_list_buy(player_get_resources(), data->cost);
                newSection = station_add_section(data->station,station_def_get_name_by_display(data->selected),-1,data->parent,data->slot);
                if (!freeBuildMode)
                {
                    newSection->working = 1;
                    newSection->hull = -1;
                    gfc_line_sprintf(buffer,"%i",newSection->id);
                    newSection->mission = mission_begin(
                        "Section Construction",
                        NULL,
                        "build",
                        "section",
                        newSection->name,
                        newSection->id,
                        player_get_day(),
                        station_def_get_build_time_by_name(newSection->name),
                        0);
                }
                station_menu_select_segment(win->parent,newSection->id);
                gf2d_window_free(win);*/
                return 1;
            }
            else
            {
                message_new("We require more resources");
            }
            return 1;
        }
        if (e->index >= 1000)
        {
            station_buy_menu_select_item(win,e->index- 1000, e->name);
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

void station_buy_menu_set_list(Window *win)
{
    const char *str;
    const char *mountType;
    const char *first = NULL;
    Element *button;
    Element *item_list;
    int choice;
    int i,c;
    StationBuyMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    c = gfc_list_get_count(data->list);
    item_list = gf2d_window_get_element_by_name(win,"item_list");
    for (i = 0; i < c; i++)
    {
        str = gfc_list_get_nth(data->list,i);
        if (!str)continue;
        if (station_def_is_unique(str))continue;
        mountType = station_def_get_mount_type(str);
        if (strcmp(mountType,data->mountType) != 0)continue;
        str = station_def_get_display_name(str);
        if (!first)
        {
            choice = i;
            first = str;
        }
        button = gf2d_button_new_label_simple(win,1000+i,str,FT_Small,vector2d(1,30),GFC_COLOR_YELLOW);
        if (!button)continue;
        gf2d_element_list_add_item(item_list,button);
    }
    station_buy_menu_select_item(win,choice,first);
}

Window *station_buy_menu(Window *parent,StationData *station, StationSection *parentSection,Uint8 slot,List *list)
{
    Window *win;
    StationBuyMenuData* data;
    win = gf2d_window_load("menus/section_buy.menu");
    if (!win)
    {
        slog("failed to load station buy menu");
        return NULL;
    }
    win->update = station_buy_menu_update;
    win->free_data = station_buy_menu_free;
    win->draw = station_buy_menu_draw;
    data = (StationBuyMenuData*)gfc_allocate_array(sizeof(StationBuyMenuData),1);
    win->data = data;
    win->parent = parent;
    data->list = list;
    data->station = station;
    data->parent = parentSection;
    data->slot = slot;
    data->mountType = station_def_get_extension_mount_type(parentSection->name,slot);
    station_buy_menu_set_list(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
