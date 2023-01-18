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

#include "resources.h"
#include "station_def.h"
#include "station.h"
#include "player.h"
#include "station_menu.h"
#include "facility_menu.h"
#include "section_view_menu.h"
#include "section_list_menu.h"

typedef struct
{
    int updated;
}SectionListMenuData;

void section_list_menu_update_resources(Window *win);

int section_list_menu_free(Window *win)
{
    SectionListMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int section_list_menu_update(Window *win,List *updateList)
{
    int i,count;
//    TextLine buffer;
    StationData *station;
    StationSection *section;
    Element *e;
    SectionListMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (SectionListMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        section_list_menu_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if ((e->index >= 500)&&((e->index < 1000)))
        {
            if (win->child)return 1;
            station = player_get_station_data();
            if (!station)return 1;
            section = gfc_list_get_nth(station->sections,e->index - 500);
            if (!section)return 1;
            station_menu_select_segment(gf2d_window_get_by_name("station_menu"),section->id);
            win->child = section_view_menu(win,section);
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


int section_list_menu_draw(Window *win)
{
//    SectionListMenuData *data;
    if ((!win)||(!win->data))return 0;

//    data = win->data;
    return 0;
}

Element *section_list_menu_build_row(Window *win, StationSection *section, int index)
{
    Color color;
    TextLine buffer;
    Element *rowList;
    ListElement *le;
    if ((!win)||(!win->data)||(!section))return NULL;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)section->name,
        gfc_rect(0,0,1,1),
        GFC_COLOR_WHITE,0,
        GFC_COLOR_DARKGREY,index%2,win);
    gf2d_element_make_list(rowList,le);
    
    //name
    gf2d_element_list_add_item(
        rowList,
        gf2d_button_new_label_simple(
            win,500+index,
            section->displayName,
            FT_H6,vector2d(200,24),GFC_COLOR_LIGHTCYAN));

    //hull
    if (section->hull < 0)
    {
        gfc_line_sprintf(buffer,"uc / %i",(int)section->hullMax);
    }
    else
    {
        gfc_line_sprintf(buffer,"%i / %i",(int)section->hull,(int)section->hullMax);
    }
    if (section->hull < section->hullMax)
    {
        color = GFC_COLOR_RED;
    }
    else
    {
        color = GFC_COLOR_WHITE;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),color));
    //energy
    if (section->energyOutput > 0)
    {
        gfc_line_sprintf(buffer,"+ %i",(int)(section->energyOutput));
        color = GFC_COLOR_CYAN;
    }
    else if (section->energyDraw > 0)
    {
        gfc_line_sprintf(buffer,"- %i",(int)section->energyDraw);
        color = GFC_COLOR_RED;
    }
    else
    {
        gfc_line_sprintf(buffer,"<none>");
        color = GFC_COLOR_WHITE;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),color));
    //cargo
    if (section->storageCapacity == 0)
    {
        gfc_line_sprintf(buffer,"---");
    }
    else 
    {
        gfc_line_sprintf(buffer,"%iT",section->storageCapacity);
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    //housing
    if (section->housing == 0)
    {
        gfc_line_sprintf(buffer,"---");
    }
    else 
    {
        gfc_line_sprintf(buffer,"%i",section->housing);
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //facilities
    gfc_line_sprintf(buffer,"%i/%i",gfc_list_get_count(section->facilities),section->facilitySlots);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),GFC_COLOR_WHITE));
    return rowList;
    
}

void section_list_menu_update_resources(Window *win)
{
    int i,c;
    StationData *station;
    StationSection *section;
    Element *list,*e;
    SectionListMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    station = player_get_station_data();
    if (!station)return;
    list = gf2d_window_get_element_by_name(win,"item_list");
    if (!list)return;
    gf2d_element_list_free_items(list);
    c = gfc_list_get_count(station->sections);
    for (i = 0; i < c; i++)
    {
        section = gfc_list_get_nth(station->sections,i);
        if (!section)continue;
        e = section_list_menu_build_row(win, section,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
    }
        
    data->updated = player_get_day();
}

Window *section_list_menu(Window *parent)
{
    Window *win;
    SectionListMenuData *data;
    win = gf2d_window_load("menus/section_list.menu");
    if (!win)
    {
        slog("failed to load section list window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(SectionListMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = section_list_menu_update;
    win->free_data = section_list_menu_free;
    win->draw = section_list_menu_draw;
    win->refresh = section_list_menu_update_resources;
    section_list_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
