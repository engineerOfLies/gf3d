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
    int inProgress;
    int repairPossible;
    Uint32 staffAssigned;
    int daysToComplete;
    float cost;
    StationSection *section;
    StationFacility *facility;
}RepairMenuData;

void repair_menu_setup(Window *win,RepairMenuData *data);

int repair_menu_free(Window *win)
{
    RepairMenuData *data;
    if (!gf2d_window_check(win,"repair_menu"))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

void repair_mission(Window *win)
{
    TextLine buffer;
    Uint32 day;
    RepairMenuData *data;
    if (!gf2d_window_check(win,"repair_menu"))return;
    data = win->data;
    
    resources_list_withdraw(player_get_resources(),"credits",data->cost);
    
    day = player_get_day();
    if (data->facility)
    {
        gfc_line_sprintf(buffer,"%i",data->facility->id);
        data->facility->mission = mission_begin(
            "repair",
            "facility",
            buffer,
            day,
            day + data->daysToComplete,
            data->staffAssigned);
        data->facility->repairing = 1;
    }
    else if (data->section)
    {
        gfc_line_sprintf(buffer,"%i",data->section->id);
        data->section->mission = mission_begin(
            "repair",
            "section",
            buffer,
            day,
            day + data->daysToComplete,
            data->staffAssigned);
        data->section->repairing = 1;
    }
    data->inProgress = 1;
    repair_menu_setup(win,data);
    message_new("Repair Mission Begin!");
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
            if (data->inProgress)
            {
                message_new("Repair already in progress, we cannot assign more staff");
                return 1;
            }
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
            if (data->inProgress)
            {
                message_new("Repair already in progress, we cannot change the staff");
                return 1;
            }
            if (data->staffAssigned > 0)
            {
                player->staff++;
                data->staffAssigned--;
                repair_menu_setup(win,data);
            }
            return 1;
        }
        if (strcmp(e->name,"repair")==0)
        {
            if (data->inProgress)
            {
                message_new("Repair already in progress");
                return 1;
            }
            if (data->repairPossible)
            {
                repair_mission(win);
            }
            else message_new("cannot initiate repair");
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
    List *costs;
    List *supply;
    Mission *mission;
    Uint32 day = player_get_day();
    Bool inProgress = 0;
    const char *display_name;
    TextLine buffer;
    if ((!win)||(!data))return;
    
    data->inProgress = 0;
    data->repairPossible = 1;
    data->cost = -1;
    if (data->section)
    {
        display_name = station_def_get_display_name(data->section->name);
        if (display_name)
        {
            gfc_line_sprintf(buffer,"%s %i",display_name,data->section->id);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"section"),buffer);
            gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"section"), 0);
        }
        if (data->section->repairing)
        {
            data->inProgress = 1;
            data->repairPossible = 0;
            inProgress = 1;
            mission = data->section->mission;
        }
        else
        {
            if (data->staffAssigned < REPAIR_CREW_MIN)
            {
                data->daysToComplete = -1;
            }
            else
            {
                data->daysToComplete = (data->section->hullMax - data->section->hull)/(float)(data->staffAssigned * 2);
            }
            costs = station_get_resource_cost(data->section->name);
            if (costs)
            {
                data->cost = resources_list_get_amount(costs,"credits");
                resources_list_free(costs);
                data->cost *= data->section->hull/(float)data->section->hullMax;
            }
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
        if (data->facility->repairing)
        {
            data->inProgress = 1;
            data->repairPossible = 0;
            inProgress = 1;
            mission = data->facility->mission;
        }
        else
        {
            if (data->staffAssigned < REPAIR_CREW_MIN)
            {
                data->daysToComplete = -1;
            }
            else
            {
                data->daysToComplete = (data->facility->damage * 100)/(float)data->staffAssigned;
            }

            costs = station_facility_get_resource_cost(data->facility->name,"cost");
            if (costs)
            {
                data->cost = resources_list_get_amount(costs,"credits");
                resources_list_free(costs);
                data->cost *= data->facility->damage;
            }
        }
    }
    else
    {
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"facility"), 1);
    }

    if (!inProgress)
    {
        if (data->daysToComplete < 0)
        {
            data->repairPossible = 0;
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

        if (data->cost > 0)
        {
            supply = player_get_resources();
            gfc_line_sprintf(buffer,"Cost: %.2fcr",data->cost);
            if (resources_list_get_amount(supply,"credits") > data->cost)
            {
                gf2d_element_set_color(gf2d_window_get_element_by_name(win,"cost"),GFC_WHITE);
            }
            else gf2d_element_set_color(gf2d_window_get_element_by_name(win,"cost"),GFC_RED);
        }
        else
        {
            data->repairPossible = 0;
            gfc_line_sprintf(buffer,"Cost: Cannot Complete");
            gf2d_element_set_color(gf2d_window_get_element_by_name(win,"cost"),GFC_RED);
        }
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cost"),buffer);

        gfc_line_sprintf(buffer,"Staff: %i / %i",data->staffAssigned,REPAIR_CREW_MAX);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
        if (data->staffAssigned < REPAIR_CREW_MIN)
        {
            data->repairPossible = 0;
            gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_RED);
        }
        else
        {
            gf2d_element_set_color(gf2d_window_get_element_by_name(win,"staff"),GFC_WHITE);
        }
    }
    else
    {
        if (mission)
        {
            data->repairPossible = 0;
            gfc_line_sprintf(buffer,"Staff: %i",mission->staff);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
            gfc_line_sprintf(buffer,"In Progress");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cost"),buffer);
            gfc_line_sprintf(buffer,"Time To Complete: %i days",mission->dayFinished - day);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"time"),buffer);
        }
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
