#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_windows_common.h"
#include "gf2d_message_buffer.h"

#include "gf3d_camera.h"

#include "entity.h"
#include "camera_entity.h"
#include "station.h"
#include "world.h"
#include "resources.h"
#include "player.h"
#include "station.h"
#include "station_menu.h"
#include "personnel_menu.h"
#include "resources_menu.h"
#include "planet_menu.h"
#include "mission_list_menu.h"
#include "market_menu.h"
#include "main_menu.h"
#include "hud_window.h"

extern int freeBuildMode;

typedef struct
{
    TextLine filename;
    int paused;
    Entity  *player;
    int     selection;
    Window  *messages;
    World   *w;
}HUDWindowData;

const char *options[] = 
{
    "Save Game",
    "Quick Save",
    "Audio Settings",
    "Exit to Main Menu",
    NULL
};

void onFileSaveCancel(void *Data)
{
    Window *win;
    if (!Data)return;
    win = Data;
    win->child = NULL;
}

void onFileSaveOk(void *Data)
{
    TextLine filepath;
    Window *win;
    HUDWindowData *data;
    win = Data;
    if ((!win)||(!win->data))return;
    data = win->data;
    win->child = NULL;
    if (strlen(data->filename) <= 0)return;
    gfc_line_sprintf(filepath,"saves/%s.save",data->filename);
    message_printf("saving game to %s",filepath);
    player_save(filepath);
}

void hud_reset_camera(Window *win)
{
    HUDWindowData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    camera_entity_set_look_target(vector3d(0,0,0));
    camera_entity_set_position(data->w->cameraPosition);
    camera_entity_set_auto_pan(1);
    camera_entity_set_look_mode(CTT_Position);
    camera_entity_enable_free_look(0);
}

void hud_options_select(void *Data)
{
    char *c;
    Window *win;
    PlayerData *player;
    HUDWindowData *data;
    win = Data;
    if ((!win)||(!win->data))return;
    win->child = NULL;
    data = win->data;
    if (data->selection < 0)return;
    switch(data->selection)
    {
        case 0:
            player = player_get_data();
            if (!player)return;
            c = strchr(player->filename,'/');
            if (c != NULL)
            {
                c++;
            }
            else c = player->filename;
            gfc_line_cpy(data->filename,c);
            c = strchr(data->filename,'.');
            if (c != NULL)
            {
                *c = '\0';//terminate it here
            }
            win->child = window_text_entry("Enter filename to Save", data->filename, win, GFCLINELEN, onFileSaveOk,onFileSaveCancel);
            break;
        case 1:
            player_save("saves/quick.save");
            message_new("quick save...");
            break;
        case 3:
            //exit to main menu
            gf2d_window_free(win);
            main_menu();
            break;
    }
}

void hud_open_options_menu(Window *win)
{
    int i;
    List *list;
    HUDWindowData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    if (win->child != NULL)
    {
        gf2d_window_free(win->child);
        return;
    }
    list = gfc_list_new();
    for (i = 0; options[i] != NULL; i++)list = gfc_list_append(list,(void *)options[i]);
    
    win->child = item_list_menu(win,vector2d(1050,58),200,"Options",list,hud_options_select,(void *)win,&data->selection);
    
    gfc_list_delete(list);
}

int hud_free(Window *win)
{
    HUDWindowData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_window_free(data->messages);
    entity_free(data->player);
    free(data);
    return 0;
}

int hud_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    HUDWindowData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (HUDWindowData*)win->data;
    if ((!freeBuildMode)&&(!data->paused))world_run_updates(data->w);
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"options")==0)
        {
            hud_open_options_menu(win);
            return 1;
        }
        if (strcmp(e->name,"personnel")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"personnel_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if (win->child)
            {
                gf2d_window_free(win->child);
            }
            win->child = personnel_menu(win);
            return 1;
        }
        if (strcmp(e->name,"station")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"station_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if (win->child)
            {
                gf2d_window_free(win->child);
            }
            win->child = station_menu_window(win,player_get_station_data());
            return 1;
        }
        if (strcmp(e->name,"planet")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"planet_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if (win->child)
            {
                gf2d_window_free(win->child);
            }
            win->child = planet_menu(win);
            return 1;
        }
        if (strcmp(e->name,"market")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"market_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if (win->child)
            {
                gf2d_window_free(win->child);
            }
            win->child = market_menu(win);
            return 1;
        }
        if (strcmp(e->name,"missions")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"mission_list_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if (win->child)
            {
                gf2d_window_free(win->child);
            }
            win->child = mission_list_menu(win);
            return 1;
        }
        
        if (strcmp(e->name,"freelook")==0)
        {
            if (win->child)return 1;// skip if a child window is open
            if (camera_entity_free_look_enabled())
            {//if free look was already enabled, reset the position
                hud_reset_camera(win);
            }
            else camera_entity_toggle_free_look();

            return 1;
        }
        if (strcmp(e->name,"pause")==0)
        {
            data->paused = !data->paused;
            return 1;
        }
        if (strcmp(e->name,"quick_save")==0)
        {
            player_save("saves/quick.save");
            message_new("quick save...");
            return 1;
        }

    }
    return 1;
}


int hud_draw(Window *win)
{
    TextLine buffer;
    List *resources;
    Color color;
    float cash,lastCash;
    Vector2D res, position;
    HUDWindowData *data;
    PlayerData *player;
    StationData *station;
    int spacing = 22;

    if ((!win)||(!win->data))return 0;
    player = player_get_data();
    if (!player)return 0;
    res = gf3d_vgraphics_get_resolution();
    data = win->data;
    world_draw(data->w);
    planet_draw(player_get_planet());
    
    
    position.x = res.x * 0.74;
    position.y = res.y - 100;
    gf2d_draw_window_border_generic(gfc_rect(position.x,position.y,res.x *0.255,100),gfc_color8(255,255,255,255));
    
    
    station = player_get_station_data();
    if (!station)return 0;
    position.y += 5;
    position.x += 20;
    
    if (data->paused)
    {
        gfc_line_sprintf(buffer,"-<PAUSED>-");
    }
    else get_datetime(buffer);
    gf2d_font_draw_line_tag(buffer,FT_H6,gfc_color8(255,255,255,255), position);
    position.y += spacing;
    
    gfc_line_sprintf(buffer,"Station HULL: %i / %i",(int)station->hull,(int)station->hullMax);
    gf2d_font_draw_line_tag(buffer,FT_H6,gfc_color8(255,255,255,255), position);

    position.y += spacing;
    gfc_line_sprintf(buffer,"Energy Surplus: %i",(int)(station->energyOutput - station->energyDraw));
    gf2d_font_draw_line_tag(buffer,FT_H6,gfc_color8(255,255,255,255), position);

    resources = player_get_resources();
    if (!resources)return 0;
    position.y += spacing;
    cash = resources_list_get_amount(resources,"credits");
    lastCash = resources_list_get_amount(player->yesterday,"credits");
    color = GFC_WHITE;
    if (cash < lastCash)color = GFC_RED;
    if (cash > lastCash)color = GFC_GREEN;
    gfc_line_sprintf(buffer,"Credits: %iCr",(int)resources_list_get_amount(resources,"credits"));
    gf2d_font_draw_line_tag(buffer,FT_H6,color, position);
    return 0;
}

Window *hud_window(const char *savefile)
{
    Window *win;
    HUDWindowData *data;
    win = gf2d_window_load("menus/hud_window.menu");
    if (!win)
    {
        slog("failed to load hud window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(HUDWindowData),1);
    if (!data)return NULL;
    win->update = hud_update;
    win->free_data = hud_free;
    win->draw = hud_draw;
    win->data = data;
    data->player = player_new(savefile);
    if (!data->player)
    {
        gf2d_window_free(win);
        main_menu();
        return NULL;
    }
    data->messages = window_message_buffer(5, 1000, gfc_color8(0,255,100,255));
    data->w = player_get_world();
    hud_reset_camera(win);
    return win;
}
