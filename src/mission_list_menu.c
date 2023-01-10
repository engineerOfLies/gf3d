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
#include "camera_entity.h"
#include "resources.h"
#include "station_def.h"
#include "station.h"
#include "planet.h"
#include "player.h"
#include "facility_menu.h"
#include "mission_list_menu.h"

typedef struct
{
    int updated;
}MissionListMenuData;

void mission_list_menu_update_resources(Window *win);

int mission_list_menu_free(Window *win)
{
    MissionListMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int mission_list_menu_update(Window *win,List *updateList)
{
    int i,count;
//    TextLine buffer;
    Element *e;
    MissionListMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (MissionListMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        mission_list_menu_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 500)
        {
            //View
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


int mission_list_menu_draw(Window *win)
{
//    MissionListMenuData *data;
    if ((!win)||(!win->data))return 0;

//    data = win->data;
    return 0;
}

Element *mission_list_menu_build_row(Window *win, Mission *mission, int index)
{
    Color color = GFC_COLOR_WHITE;
    Element *rowList;
    TextLine buffer;
    ListElement *le;
//    MissionListMenuData *data;
    if ((!win)||(!win->data)||(!mission))return NULL;
//    data = win->data;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)mission->title,
        gfc_rect(0,0,1,1),
        GFC_COLOR_WHITE,0,
        GFC_COLOR_BLACK,0,win);
    gf2d_element_make_list(rowList,le);
    // title
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,mission->title,FT_H6,vector2d(200,24),color));
    // id
    gfc_line_sprintf(buffer,"%i",mission->id);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    // target
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,mission->missionTarget,FT_H6,vector2d(200,24),color));
    // start time
    get_date_of(buffer,mission->dayStart);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(100,24),color));
    // Finish On
    get_date_of(buffer,mission->dayFinished);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(100,24),color));
    // days left
    gfc_line_sprintf(buffer,"%i",mission->dayFinished - player_get_day());
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    // staff
    gfc_line_sprintf(buffer,"%i",mission->staff);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));

    return rowList;
    
}

void mission_list_menu_update_resources(Window *win)
{
    int i,c;
    List *missions;
    Mission *mission;
    Element *list,*e;
    MissionListMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    list = gf2d_window_get_element_by_name(win,"list_items");
    gf2d_element_list_free_items(list);
    missions = mission_list_get();
    if ((!list)||(!missions))return;
    c = gfc_list_get_count(missions);
    for (i = 0; i < c; i++)
    {
        mission = gfc_list_get_nth(missions,i);
        if (!mission)continue;
        e = mission_list_menu_build_row(win, mission,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
    }
        
    data->updated = player_get_day();
}

Window *mission_list_menu(Window *parent)
{
    Window *win;
    MissionListMenuData *data;
    win = gf2d_window_load("menus/mission_list.menu");
    if (!win)
    {
        slog("failed to load mission list window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(MissionListMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = mission_list_menu_update;
    win->free_data = mission_list_menu_free;
    win->draw = mission_list_menu_draw;
    mission_list_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
