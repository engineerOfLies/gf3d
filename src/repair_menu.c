#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_button.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"
#include "gf2d_windows_common.h"

#include "config_def.h"
#include "entity.h"
#include "resources.h"
#include "player.h"
#include "station.h"
#include "station_def.h"
#include "station_facility.h"
#include "repair_menu.h"

#define REPAIR_CREW_MAX 20
#define REPAIR_CREW_MIN 5

typedef struct
{
    Uint32 staffAssigned;
    int daysToComplete;
    StationSection *section;
    StationFacility *facility;
}RepairMenuData;

void repair_menu_setup(Window *win,RepairMenuData *data);

int repair_menu_free(Window *win)
{
    RepairMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int repair_menu_update(Window *win,List *updateList)
{
    int i,count;
    PlayerData *player;
    Element *e;
    RepairMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (RepairMenuData*)win->data;
    player = player_get_data();
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"staff_assign")==0)
        {
            if (data->staffAssigned < REPAIR_CREW_MAX)
            {
                if (player->staff <= 0)
                {
                    message_new("Cannot assign any more staff.  Please hire more.");
                    return 1;
                }
                player->staff--;
                data->staffAssigned++;
                repair_menu_setup(win,data);
            }
            return 1;
        }
        if (strcmp(e->name,"staff_remove")==0)
        {
            if (data->staffAssigned > 0)
            {
                player->staff++;
                data->staffAssigned--;
                repair_menu_setup(win,data);
            }
            return 1;
        }
        if (strcmp(e->name,"cancel")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return 1;
}

int repair_menu_draw(Window *win)
{
 //   RepairMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void repair_menu_setup(Window *win,RepairMenuData *data)
{
    const char *display_name;
    TextLine buffer;
    if ((!win)||(!data))return;
    
    if (data->section)
    {
        display_name = station_def_get_display_name(data->section->name);
        if (display_name)
        {
            gfc_line_sprintf(buffer,"%s %i",display_name,data->section->id);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"section"),buffer);
            gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"section"), 0);
        }
        if (data->staffAssigned < REPAIR_CREW_MIN)
        {
            data->daysToComplete = -1;
        }
        else
        {
            data->daysToComplete = (data->section->hullMax - data->section->hull)/(float)(data->staffAssigned * 2);
        }
    }
    else
    {
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"section"), 1);
    }
    
    if (data->facility)
    {
        display_name = station_facility_get_display_name(data->facility->name);
        if (display_name)
        {
            gfc_line_sprintf(buffer,"%s %i",display_name,data->facility->id);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"facility"),buffer);
            gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"facility"), 0);
        }
        if (data->staffAssigned < REPAIR_CREW_MIN)
        {
            data->daysToComplete = -1;
        }
        else
        {
            data->daysToComplete = (data->facility->damage * 100)/(float)data->staffAssigned;
        }
    }
    else
    {
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"facility"), 1);
    }

    if (data->daysToComplete < 0)
    {
        gfc_line_sprintf(buffer,"Time To Complete: Cannot Complete");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"time"),buffer);
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"time"),GFC_RED);
    }
    else
    {
        gfc_line_sprintf(buffer,"Time To Complete: %i days",data->daysToComplete);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"time"),buffer);
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"time"),GFC_WHITE);
    }
    
    gfc_line_sprintf(buffer,"Staff: %i / %i",data->staffAssigned,REPAIR_CREW_MAX);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
    if (data->staffAssigned < REPAIR_CREW_MIN)
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_RED);
    }
    else
    {
        gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_WHITE);
    }
    
}

Window *repair_menu(Window *parent, StationSection *section, StationFacility *facility)
{
    Window *win;
    RepairMenuData *data;
    win = gf2d_window_load("menus/repair.menu");
    if (!win)
    {
        slog("failed to load repair menu");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(RepairMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = repair_menu_update;
    win->free_data = repair_menu_free;
    win->draw = repair_menu_draw;
    data->section = section;
    data->facility = facility;
    repair_menu_setup(win,data);
    message_buffer_bubble();
    return win;
}
