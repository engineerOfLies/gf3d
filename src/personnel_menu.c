#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
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
#include "player.h"
#include "station_extension_menu.h"
#include "station_buy_menu.h"
#include "facility_menu.h"
#include "personnel_menu.h"

typedef struct
{
    int updated;
}PersonnelMenuData;

void personnel_menu_update_resources(Window *win);

int personnel_menu_free(Window *win)
{
    PersonnelMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int personnel_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    StationData *station;
    PlayerData *player;
    PersonnelMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (PersonnelMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        personnel_menu_update_resources(win);
    }
    player = player_get_data();
    station = player_get_station_data();
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"tax_raise")==0)
        {
            player->taxRate += 0.01;
            personnel_menu_update_resources(win);
            return 1;
        }
        if (strcmp(e->name,"tax_lower")==0)
        {
            player->taxRate -= 0.01;
            if (player->taxRate < 0)player->taxRate = 0;
            personnel_menu_update_resources(win);
            return 1;
        }
        if (strcmp(e->name,"hire_more")==0)
        {
            if ((player->staff + station->staffAssigned) < player->population)
            {
                player->staff++;
            }
            else
            {
                message_new("There is no one left to hire on the station.");
            }
            personnel_menu_update_resources(win);
            return 1;
        }
        if (strcmp(e->name,"hire_less")==0)
        {
            if (player->staff > 0)player->staff--;
            personnel_menu_update_resources(win);
            return 1;
        }
        if (strcmp(e->name,"wage_raise")==0)
        {
            player->wages += 0.5;
            personnel_menu_update_resources(win);
            return 1;
        }
        if (strcmp(e->name,"wage_lower")==0)
        {
            player->wages -= 0.5;
            if (player->wages < 1)player->wages = 1;
            personnel_menu_update_resources(win);
            return 1;
        }
    }
    if (gfc_input_command_released("cancel"))
    {
        gf2d_window_free(win);
        return 1;
    }
    return 0;
}


int personnel_menu_draw(Window *win)
{
//    PersonnelMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

void personnel_menu_update_resources(Window *win)
{
    TextLine buffer;
    PlayerData *player;
    StationData *station;
    PersonnelMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    player = player_get_data();
    station = player_get_station_data();
    if ((!player)||(!station))return;

    gfc_line_sprintf(buffer,"Population : %i",player->population);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"population"),buffer);

    gfc_line_sprintf(buffer,"Housing : %i",station->housing);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"housing"),buffer);

    gfc_line_sprintf(buffer,"Taxes: %.2f %%",player->taxRate);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"taxes"),buffer);

    gfc_line_sprintf(buffer,"Wages: %.2fCr",player->wages);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"wages"),buffer);

    gfc_line_sprintf(buffer,"Total Staff : %i",player->staff + station->staffAssigned);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff_total"),buffer);

    gfc_line_sprintf(buffer,"Free Staff : %i",player->staff);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"staff"),buffer);
    gfc_line_sprintf(buffer,"Assigned : %i",station->staffAssigned);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"assigned"),buffer);
    
    gfc_line_sprintf(buffer,"Approval Rating: %.1f%%",100 * player->reputation.satisfaction);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"satisfaction"),buffer);

    gfc_line_sprintf(buffer,"Basic Needs Met: %.1f%%",100 * player->reputation.basicNeeds);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"basic_needs"),buffer);

    gfc_line_sprintf(buffer,"Commerce: %.1f%%",100 * player->reputation.commerce);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"commerce"),buffer);

    gfc_line_sprintf(buffer,"Employment: %.1f%%",100 * player->reputation.opportunities);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"opportunity"),buffer);

    gfc_line_sprintf(buffer,"Public Safety: %.1f%%",100 * player->reputation.safety);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"safety"),buffer);

    gfc_line_sprintf(buffer,"Entertainment: %.1f%%",100 * player->reputation.entertainment);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"entertainment"),buffer);

    gfc_line_sprintf(buffer,"Crime Rate : %.1f%%",MAX(1,station->crimeRate));
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"crime_rate"),buffer);

    data->updated = player_get_day();
}

Window *personnel_menu(Window *parent)
{
    Window *win;
    PersonnelMenuData *data;
    win = gf2d_window_load("menus/personnel.menu");
    if (!win)
    {
        slog("failed to load personnel window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(PersonnelMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = personnel_menu_update;
    win->free_data = personnel_menu_free;
    win->draw = personnel_menu_draw;
    personnel_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
