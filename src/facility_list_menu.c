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
#include "facility_list_menu.h"

typedef struct
{
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

int facility_list_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
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
        if ((e->index >= 500)&&((e->index < 1000)))
        {
            //View
            slog("updated element: %s",e->name);
            if (win->child)return 1;
            facility = player_get_facility_nth(e->index - 500);
            if (strcmp(facility->facilityType,"planetary")==0)
            {
                //planet based facility
                if (!win->child)
                {
                    win->child = facility_menu(win,facility);
                }
                return 1;
            }
            section = player_get_section_by_facility(facility);
            if (section)
            {
                if (!win->child)
                {
                    win->child = facility_menu(win,facility);
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
    int offline;
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
        GFC_COLOR_WHITE,0,
        GFC_COLOR_DARKGREY,index%2,win);
    gf2d_element_make_list(rowList,le);
    
    //name
    if ((facility->inactive)||(facility->disabled))
    {
        offline = 1;
        color = GFC_COLOR_RED;
    }
    else
    {
        offline = 0;
        color = GFC_COLOR_LIGHTCYAN;
    }
    gf2d_element_list_add_item(
        rowList,
        gf2d_button_new_label_simple(
            win,500+index,
            facility->displayName,
            FT_H6,vector2d(200,24),color));

    //damag
    if (facility->damage < 0)
    {
        gfc_line_sprintf(buffer,"<uc>");
    }
    else gfc_line_sprintf(buffer,"%i",(int)(facility->damage * 100));
    if (facility->damage > 0)color = GFC_COLOR_RED;
    else color = GFC_COLOR_WHITE;
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    //energy
    if (offline)
    {
        gfc_line_sprintf(buffer,"<offline>");
        color = GFC_COLOR_RED;
    }
    else
    {
        if (facility->energyOutput > 0)
        {
            gfc_line_sprintf(buffer,"+ %i",(int)(facility->energyOutput * facility->productivity));
            color = GFC_COLOR_CYAN;
        }
        else if (facility->energyDraw > 0)
        {
            gfc_line_sprintf(buffer,"- %i",facility->energyDraw);
            color = GFC_COLOR_RED;
        }
        else
        {
            gfc_line_sprintf(buffer,"<none>");
            color = GFC_COLOR_WHITE;
        }
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),color));
    //officer
    if (!station_facility_supports_officer(facility->name))
    {
        gfc_line_sprintf(buffer,"---");
    }
    else if (strlen(facility->officer) > 0)
    {
        gfc_line_sprintf(buffer,"%s",facility->officer);
    }
    else
    {
        gfc_line_sprintf(buffer,"<unassigned>");
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),GFC_COLOR_WHITE));
    //staff
    if (facility->staffRequired == 0)
    {
        gfc_line_sprintf(buffer,"---");
        color = GFC_COLOR_WHITE;
    }
    else if (facility->staffAssigned < facility->staffRequired)
    {
        gfc_line_sprintf(buffer,"%i / %i",facility->staffAssigned,facility->staffPositions);
        color = GFC_COLOR_RED;
    }
    else
    {
        gfc_line_sprintf(buffer,"%i / %i",facility->staffAssigned,facility->staffPositions);
        color = GFC_COLOR_WHITE;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    //productivity
    if (facility->productivity == 0)
    {
        gfc_line_sprintf(buffer,"---");
        color = GFC_COLOR_WHITE;
    }
    else 
    {
        gfc_line_sprintf(buffer,"%i%%",(int)(facility->productivity * 100.0));
        if (facility->productivity < 1)color = GFC_COLOR_RED;
        else color = GFC_COLOR_WHITE;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(60,24),color));
    //storage
    if (facility->storage == 0)
    {
        gfc_line_sprintf(buffer,"---");
    }
    else 
    {
        gfc_line_sprintf(buffer,"%iT",facility->storage);
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),GFC_COLOR_WHITE));
    //housing
    if (facility->housing == 0)
    {
        gfc_line_sprintf(buffer,"---");
    }
    else 
    {
        gfc_line_sprintf(buffer,"%i",facility->housing);
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(90,24),GFC_COLOR_WHITE));
    //revenue
    if (((!facility->operatingCost)&&(!facility->income))||(facility->operatingCost == facility->income))
    {
        gfc_line_sprintf(buffer,"---");
        color = GFC_COLOR_WHITE;
    }
    else if (facility->operatingCost > facility->income)
    {
        gfc_line_sprintf(buffer,"-%i",facility->operatingCost - facility->income);
        color = GFC_COLOR_RED;
    }
    else
    {
        gfc_line_sprintf(buffer,"+%i",facility->operatingCost - facility->income);
        color = GFC_COLOR_GREEN;
    }
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(80,24),color));

    return rowList;
    
}

void facility_list_menu_update_resources(Window *win)
{
    TextLine buffer;
    int i,c;
    Color color;
    StationData *station;
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
    
    station = player_get_station_data();
    if (station)
    {
    //populate bottom line
        gfc_line_sprintf(buffer,"%i",(int)station->energySupply);
        if (station->energySupply > 0)color = GFC_COLOR_CYAN;
        else color = GFC_COLOR_RED;
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"energy_supply"),buffer);
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"energy_supply"),color);
        
        gfc_line_sprintf(buffer,"%i /%i",(int)station->staffAssigned,station->staffPositions);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff_totals"),buffer);

        gfc_line_sprintf(buffer,"%iT",(int)station->storageCapacity);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"total_storage"),buffer);

        gfc_line_sprintf(buffer,"%i",(int)station->housing);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"total_housing"),buffer);
    }
    
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
    win->refresh = facility_list_menu_update_resources;
    facility_list_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
