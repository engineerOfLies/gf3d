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
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"

#include "player.h"
#include "config_def.h"
#include "station_def.h"
#include "resources.h"
#include "work_menu.h"
#include "facility_menu.h"
#include "facility_shop_list.h"
#include "station_buy_menu.h"
#include "section_view_menu.h"

extern int freeBuildMode;

typedef struct
{
    StationSection *section;
}SectionViewMenuData;

int section_view_menu_free(Window *win)
{
    SectionViewMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int section_view_menu_draw(Window *win)
{
//    SectionViewMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void section_view_menu_refresh(Window *win)
{
    SJson *def;
    Color color;
    StationSection *child;
    StationFacility *facility;
    int i,c;
    int count = 0;
    TextLine buffer;
    const char *str;
    Element *elist;
    SectionViewMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->section)return;
    def = config_def_get_by_name("sections",data->section->name);
    if (!def)return;
    //icon
    str = sj_object_get_value_as_string(def,"icon");
    gf2d_element_actor_set_actor(gf2d_window_get_element_by_name(win,"item_picture"),str);
    //name
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_name"),data->section->displayName);
    //model
    gfc_line_sprintf(buffer,"Model: %s",data->section->name);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"model"),buffer);
    //mission
    if (data->section->mission)
    {
        gfc_line_sprintf(buffer,"Action: %s",data->section->mission->title);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"mission"),buffer);
    }
    else
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"mission"),"Action: ---");        
    }

    //description
    str = sj_object_get_value_as_string(def,"description");
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"item_description"),str);
    //hull
    if (data->section->hull < 0)
    {
        gfc_line_sprintf(buffer,"Hull: ---/%i",(int)data->section->hullMax);
        color = GFC_COLOR_WHITE;
    }
    else
    {
        if (data->section->hull < data->section->hullMax)color = GFC_COLOR_RED;
        else color = GFC_COLOR_WHITE;
        gfc_line_sprintf(buffer,"Hull: %i/%i",(int)data->section->hull,(int)data->section->hullMax);
    }
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"hull"),color);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"hull"),buffer);
    //storage
    gfc_line_sprintf(buffer,"Storage: %i T",data->section->storageCapacity);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cargo"),buffer);
    //energy
    if (data->section->energyDraw > data->section->energyOutput)color = GFC_COLOR_RED;
    else color = GFC_COLOR_CYAN;
    gfc_line_sprintf(buffer,"Energy: %i/%i",(int)data->section->energyDraw,(int)data->section->energyOutput);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy"),buffer);
    gf2d_element_set_color(gf2d_window_get_element_by_name(win,"energy"),color);
    //passengers
    gfc_line_sprintf(buffer,"Housing: %i",data->section->housing);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"housing"),buffer);
    //extensions
    elist = gf2d_window_get_element_by_name(win,"extensions");
    if (elist)
    {
        gf2d_element_list_free_items(elist);
        c = station_def_get_extension_count_by_name(data->section->name);
        for (i = 0; i < c; i++)
        {
            child = station_section_get_child_by_slot(data->section,i);
            if (child)
            {
                gfc_line_sprintf(buffer,"%s : %s",station_def_get_extension_mount_type(data->section->name,i),child->displayName);
                gf2d_element_list_add_item(
                    elist,
                    gf2d_button_new_label_simple(
                        win,700+child->id,
                        buffer,
                        FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
                continue;
            }
            gfc_line_sprintf(buffer,"%s : <empty>",station_def_get_extension_mount_type(data->section->name,i));
            gf2d_element_list_add_item(
                elist,
                gf2d_button_new_label_simple(
                    win,800+i,
                    buffer,
                    FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
        }
    }
    //facilities
    elist = gf2d_window_get_element_by_name(win,"facilities_list");
    if (elist)
    {
        count = 0;
        gf2d_element_list_free_items(elist);
        c = gfc_list_get_count(data->section->facilities);
        for (i = 0; i < c; i++)
        {
            facility = gfc_list_get_nth(data->section->facilities,i);
            if (!facility)continue;
            gf2d_element_list_add_item(
                elist,
                gf2d_button_new_label_simple(
                    win,500+i,
                    facility->displayName,
                    FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
            count++;
        }
        for (i = count; i < data->section->facilitySlots; i++)
        {
            gf2d_element_list_add_item(
                elist,
                gf2d_button_new_label_simple(
                    win,600+i,
                    "<empty>",
                    FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));
        }
    }
}

void section_view_sell_ok(Window *win)
{
//    SectionViewMenuData* data;
    if ((!win)||(!win->data))return;
    win->child = NULL;
  //  data = win->data;
    
    //TODO
    gf2d_window_free(win);
}

void section_view_sell_cancel(Window *win)
{
    if ((!win)||(!win->data))return;
    win->child = NULL;
}

int section_view_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlayerData *player;
    StationSection *child;
    StationFacility *facility;
    SectionViewMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (SectionViewMenuData*)win->data;
    if (!data)return 0;
    player = player_get_data();
    if (!player)return 0;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (e->index >= 800)
        {
            // extension buy
            if (win->child)return 1;
            if (data->section)
            {
                if (data->section->hull < 0)
                {
                    message_new("under construction");
                    return 1;
                }
                if (gfc_list_get_count(data->section->children) >= data->section->expansionSlots)
                {
                    message_new("no free expansion slots for this section");
                    return 1;
                }
                win->child = station_buy_menu(win,player_get_station_data(),data->section,e->index-800);
            }
            return 1;
        }
        if (e->index >= 700)
        {
            // view child
            child = station_get_section_by_id(player_get_station_data(),e->index - 700);
            if (child)
            {
                data->section = child;
                gf2d_window_refresh(win);
            }
            return 1;
        }
        if (e->index >= 600)
        {
            if (win->child)return 1;
            //facility_buy_menu
            win->child = facility_shop_list(win,station_facility_get_possible_list(data->section),data->section->facilities,vector2d(0,0));
            return 1;
        }
        if (e->index >= 500)
        {
            if (win->child)return 1;
            facility = gfc_list_get_nth(data->section->facilities,e->index - 500);
            if (!facility) return 1;
            win->child = facility_menu(win, facility);
            return 1;
        }
        if (strcmp(e->name,"repair")==0)
        {
            if (!data->section)return 1;
            if (data->section->hull < 0)
            {
                message_new("under construction");
                return 1;
            }
            if (data->section->hull >= data->section->hullMax)
            {
                message_printf("Section is not damaged");
                return 1;
            }
            if (data->section->working)
            {
                message_new("cannot repair section, it has work in progress");
                return 1;
            }
            if (win->child)return 1;
            win->child = work_menu(
                win,
                NULL,
                data->section,
                NULL,
                "repair",
                NULL,
                vector2d(0,0),
                (gfc_work_func*)gf2d_window_refresh,
                win);
            return 1;
        }
        if (strcmp(e->name,"sell")==0)
        {
            if (win->child)return 1;
            if (data->section)
            {
                if (data->section->id == 0)
                {
                    message_new("cannot sell the base core of the station!");
                    return 1;
                }
                if (gfc_list_get_count(data->section->children) > 0)
                {
                    message_new("cannot sell section, it has child extensions!");
                    return 1;
                }
                if (data->section->working)
                {
                    message_new("cannot sell section, it has work in progress");
                    return 1;
                }
                if (station_section_facility_working(data->section))
                {
                    message_new("cannot sell section, it's facilities have active jobs");
                    return 1;
                }
                win->child = work_menu(win,NULL,data->section,NULL,"remove",NULL,vector2d(0,0),(gfc_work_func*)gf2d_window_free,win);
            }
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

Window *section_view_menu(Window *parent,StationSection *section)
{
    Window *win;
    SectionViewMenuData* data;
    win = gf2d_window_load("menus/section_view.menu");
    if (!win)
    {
        slog("failed to load section view menu");
        return NULL;
    }
    data = (SectionViewMenuData*)gfc_allocate_array(sizeof(SectionViewMenuData),1);
    win->parent = parent;
    win->data = data;
    //setup callbacks
    win->update = section_view_menu_update;
    win->free_data = section_view_menu_free;
    win->refresh = section_view_menu_refresh;
    win->draw = section_view_menu_draw;
    //setup this window
    data->section = section;
    section_view_menu_refresh(win);
    message_buffer_bubble();
    return win;
}


/*eol@eof*/
