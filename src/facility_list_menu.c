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

#include "entity.h"
#include "camera_entity.h"
#include "resources.h"
#include "station_def.h"
#include "station.h"
#include "planet.h"
#include "player.h"
#include "facility_menu.h"
#include "facility_list_menu.h"

typedef struct
{
    int offset;
    int scrollCount;
    int updated;
}FacilityListMenuData;

void facility_list_menu_update_resources(Window *win);

int facility_list_menu_free(Window *win)
{
    FacilityListMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

void facility_list_menu_scroll_up(Window *win)
{
    FacilityListMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset > 0)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"facilities"),--data->offset);
        facility_list_menu_update_resources(win);
    }
}

void facility_list_menu_scroll_down(Window *win)
{
    FacilityListMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (data->offset < data->scrollCount)
    {
        gf2d_element_list_set_scroll_offset(gf2d_window_get_element_by_name(win,"facilities"),++data->offset);
        facility_list_menu_update_resources(win);
    }
}

int facility_list_menu_update(Window *win,List *updateList)
{
    int i,count;
//    TextLine buffer;
    Element *e;
    PlanetData *planet;
    StationFacility *facility;
    StationSection *section;
    FacilityListMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (FacilityListMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        facility_list_menu_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"scroll_down")==0)
        {
            facility_list_menu_scroll_down(win);
            return 1;
        }
        if (strcmp(e->name,"scroll_up")==0)
        {
            facility_list_menu_scroll_up(win);
            return 1;
        }
        if (e->index >= 500)
        {
            //View
            facility = player_get_facility_nth(e->index - 500);
            if (strcmp(facility->facilityType,"planetary")==0)
            {
                //planet based facility
                if (!win->child)
                {
                    planet = player_get_planet();
                    win->child = facility_menu(
                        win,
                        planet->facilities,
                        gfc_list_get_count(planet->facilities),
                        NULL);
                    facility_menu_select_item(win->child,gfc_list_get_item_index(planet->facilities,facility));
                }
                return 1;
            }
            section = player_get_section_by_facility(facility);
            if (section)
            {
                if (!win->child)
                {
                    win->child = facility_menu(
                        win,
                        section->facilities,
                        section->facilitySlots,
                        station_facility_get_possible_list(section));
                    facility_menu_select_item(win->child,gfc_list_get_item_index(section->facilities,facility));
                }
                return 1;
            }
            return 1;
        }
        if (strcmp(e->name,"done")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    if (gfc_input_mouse_wheel_up())
    {
        facility_list_menu_scroll_up(win);
        return 1;
    }
    if (gfc_input_mouse_wheel_down())
    {
        facility_list_menu_scroll_down(win);
        return 1;
    }

    return 0;
}


int facility_list_menu_draw(Window *win)
{
//    FacilityListMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

Element *facility_list_menu_build_row(Window *win, StationFacility *facility, int index)
{
    Color color;
    TextLine buffer;
    Element *rowList;
    ListElement *le;
//    FacilityListMenuData *data;
    if ((!win)||(!win->data)||(!facility))return NULL;
//    data = win->data;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)facility->name,
        gfc_rect(0,0,1,1),
        GFC_WHITE,0,
        GFC_BLACK,0,win);
    gf2d_element_make_list(rowList,le);
    
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,facility->displayName,FT_H6,vector2d(200,24),GFC_WHITE));
    
    gfc_line_sprintf(buffer,"%i",(int)(facility->damage * 100));
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),GFC_WHITE));
    
    if (facility->energyOutput > 0)
    {
        gfc_line_sprintf(buffer,"+ %i",facility->energyOutput);
        color = GFC_CYAN;
    }
    else if (facility->energyDraw > 0)
    {
        gfc_line_sprintf(buffer,"- %i",facility->energyDraw);
        color = GFC_RED;
    }
    else
    {
        gfc_line_sprintf(buffer,"<none>");
        color = GFC_WHITE;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),color));
    
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,500+index,
        "view",
        "actors/button.actor",
        "View",
        vector2d(0.63,0.56),
        vector2d(100,24),
        GFC_WHITE));

    return rowList;
    
}

void facility_list_menu_update_resources(Window *win)
{
    int i,c;
    StationFacility *facility;
    Element *list,*e;
    FacilityListMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    list = gf2d_window_get_element_by_name(win,"facilities");
    gf2d_element_list_free_items(list);
    if (!list)return;
    c = player_get_facility_count();
    for (i = 0; i < c; i++)
    {
        facility = player_get_facility_nth(i);
        if (!facility)continue;
        e = facility_list_menu_build_row(win, facility,i);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
    }
    if (c > gf2d_element_list_get_row_count(list))
    {
        data->scrollCount = c - gf2d_element_list_get_row_count(list);
    }
    else data->scrollCount = 0;
    data->updated = player_get_day();
}

Window *facility_list_menu(Window *parent)
{
    Window *win;
    FacilityListMenuData *data;
    win = gf2d_window_load("menus/facility_list.menu");
    if (!win)
    {
        slog("failed to load facility list window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(FacilityListMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = facility_list_menu_update;
    win->free_data = facility_list_menu_free;
    win->draw = facility_list_menu_draw;
    facility_list_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
