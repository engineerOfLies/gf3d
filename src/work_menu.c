#include "simple_logger.h"

#include "gfc_types.h"
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
#include "facility_menu.h"
#include "station_facility.h"
#include "work_menu.h"

#define WORK_CREW_MAX 20
#define WORK_CREW_MIN 5

typedef struct
{
    TextLine action;
    int inProgress;
    int workPossible;
    Uint32 staffAssigned;
    int daysToComplete;
    float cost;
    StationSection *section;
    StationFacility *facility;
}WorkMenuData;

void work_menu_setup(Window *win,WorkMenuData *data);

int work_menu_free(Window *win)
{
    WorkMenuData *data;
    if (!gf2d_window_check(win,"work_menu"))return 0;
    data = win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

void work_mission(Window *win)
{
    TextLine buffer;
    Uint32 day;
    WorkMenuData *data;
    if (!gf2d_window_check(win,"work_menu"))return;
    data = win->data;
    
    resources_list_withdraw(player_get_resources(),"credits",data->cost);
    
    day = player_get_day();
    if (data->facility)
    {
        if (strcmp(data->action,"repair")==0)
        {
            gfc_line_sprintf(buffer,"%i:%s",data->facility->id,data->facility->name);
            data->facility->mission = mission_begin(
                "Facility Repair",
                "repair",
                "facility",
                buffer,
                day,
                day + data->daysToComplete,
                data->staffAssigned);
        }
        else if (strcmp(data->action,"remove")==0)
        {
            gfc_line_sprintf(buffer,"%i",data->facility->id);
            data->facility->mission = mission_begin(
                "Facility Removal",
                "facility_sale",
                data->facility->name,
                buffer,
                day,
                day + data->daysToComplete,
                data->staffAssigned);
        }
        data->facility->working = 1;
        facility_menu_refresh_view(gf2d_window_get_by_name("station_facility_menu"));
    }
    else if (data->section)
    {
        if (strcmp(data->action,"repair")==0)
        {
            gfc_line_sprintf(buffer,"%i",data->section->id);
            data->section->mission = mission_begin(
                "Section Repair",
                "repair",
                "section",
                buffer,
                day,
                day + data->daysToComplete,
                data->staffAssigned);
        }
        else if (strcmp(data->action,"remove")==0)
        {
            gfc_line_sprintf(buffer,"%i",data->section->id);
            data->section->mission = mission_begin(
                "Section Remove",
                "section_sale",
                "section",
                buffer,
                day,
                day + data->daysToComplete,
                data->staffAssigned);
        }
        data->section->working = 1;
    }
    data->inProgress = 1;
    work_menu_setup(win,data);
    data->staffAssigned = 0;
    message_new("Work Mission Begin!");
}

int work_menu_update(Window *win,List *updateList)
{
    int i,count;
    PlayerData *player;
    Element *e;
    WorkMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (WorkMenuData*)win->data;
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
                message_new("Work already in progress, we cannot assign more staff");
                return 1;
            }
            if (data->staffAssigned < WORK_CREW_MAX)
            {
                if (player->staff <= 0)
                {
                    message_new("Cannot assign any more staff.  Please hire more.");
                    return 1;
                }
                player->staff--;
                data->staffAssigned++;
                work_menu_setup(win,data);
            }
            return 1;
        }
        if (strcmp(e->name,"staff_remove")==0)
        {
            if (data->inProgress)
            {
                message_new("Work already in progress, we cannot change the staff");
                return 1;
            }
            if (data->staffAssigned > 0)
            {
                player->staff++;
                data->staffAssigned--;
                work_menu_setup(win,data);
            }
            return 1;
        }
        if (strcmp(e->name,"work")==0)
        {
            if (data->inProgress)
            {
                message_new("Already in progress");
                return 1;
            }
            if (data->workPossible)
            {
                work_mission(win);
            }
            else message_new("cannot initiate work order");
            return 1;
        }
        if (strcmp(e->name,"cancel")==0)
        {
            player_return_staff(data->staffAssigned);
            gf2d_window_free(win);
            return 1;
        }
    }
    return 1;
}

int work_menu_draw(Window *win)
{
 //   WorkMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void work_menu_setup(Window *win,WorkMenuData *data)
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
    data->workPossible = 1;
    data->cost = -1;
    
    if (strcmp(data->action,"repair")==0)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),"Repair Order");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"action_label"),"repair");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),"Order crew to work the damaged equipment.");
    }
    else if (strcmp(data->action,"remove")==0)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),"Remove Order");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"action_label"),"remove");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),"Order a crew to dismantle and recover the materials from the unit.");
    }
    
    if (data->section)
    {
        display_name = station_def_get_display_name(data->section->name);
        if (display_name)
        {
            gfc_line_sprintf(buffer,"%s %i",display_name,data->section->id);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"section"),buffer);
            gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"section"), 0);
        }
        if (data->section->working)
        {
            data->inProgress = 1;
            data->workPossible = 0;
            inProgress = 1;
            mission = data->section->mission;
        }
        else
        {
            if (data->staffAssigned < WORK_CREW_MIN)
            {
                data->daysToComplete = -1;
            }
            else
            {
                if (strcmp(data->action,"repair")==0)
                {
                    data->daysToComplete = (data->section->hullMax - data->section->hull)/(float)(data->staffAssigned * 2);
                }
                else if (strcmp(data->action,"remove")==0)
                {
                    data->daysToComplete = MAX(1,40*(data->section->hull/(float)data->section->hullMax )/(float)(data->staffAssigned));
                }
            }
            costs = station_get_resource_cost(data->section->name);
            if (costs)
            {
                data->cost = resources_list_get_amount(costs,"credits");
                resources_list_free(costs);
                if (strcmp(data->action,"repair")==0)
                {
                    data->cost *= 1.0 - (data->section->hull/(float)data->section->hullMax);
                }
                else if (strcmp(data->action,"remove")==0)
                {
                    data->cost *= (data->section->hull/(float)data->section->hullMax)*0.1;
                }
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
        if (data->facility->working)
        {
            data->inProgress = 1;
            data->workPossible = 0;
            inProgress = 1;
            mission = data->facility->mission;
        }
        else
        {
            if (data->staffAssigned < WORK_CREW_MIN)
            {
                data->daysToComplete = -1;
            }
            else
            {
                if (strcmp(data->action,"repair")==0)
                {
                    if (data->facility->damage > 0)
                    {
                        data->daysToComplete = (data->facility->damage * 100)/(float)data->staffAssigned;
                    }
                    else data->daysToComplete = -1;
                }
                else if (strcmp(data->action,"remove")==0)
                {
                    if (data->facility->damage >= 0)
                    {
                        data->daysToComplete = MAX((40 * (1 - data->facility->damage)/(float)data->staffAssigned),1);
                    }
                    else data->daysToComplete = -1;
                }
            }

            costs = station_facility_get_resource_cost(data->facility->name,"cost");
            if (costs)
            {
                data->cost = resources_list_get_amount(costs,"credits");
                resources_list_free(costs);
                
                if (strcmp(data->action,"repair")==0)
                {
                    data->cost *= data->facility->damage;
                }
                else if (strcmp(data->action,"remove")==0)
                {
                    data->cost *= ((1 - data->facility->damage) * 0.1);
                }
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
            data->workPossible = 0;
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
            data->workPossible = 0;
            gfc_line_sprintf(buffer,"Cost: Cannot Complete");
            gf2d_element_set_color(gf2d_window_get_element_by_name(win,"cost"),GFC_RED);
        }
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"cost"),buffer);

        gfc_line_sprintf(buffer,"Staff: %i / %i",data->staffAssigned,WORK_CREW_MAX);
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
        if (data->staffAssigned < WORK_CREW_MIN)
        {
            data->workPossible = 0;
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
            data->workPossible = 0;
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

Window *work_menu(Window *parent, StationSection *section, StationFacility *facility,const char *action)
{
    Window *win;
    WorkMenuData *data;
    win = gf2d_window_load("menus/work.menu");
    if (!win)
    {
        slog("failed to load work menu");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(WorkMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = work_menu_update;
    win->free_data = work_menu_free;
    win->draw = work_menu_draw;
    if (action)gfc_line_cpy(data->action,action);
    data->section = section;
    data->facility = facility;
    work_menu_setup(win,data);
    message_buffer_bubble();
    return win;
}
