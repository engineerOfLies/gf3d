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

#include "gf3d_entity.h"
#include "camera_entity.h"
#include "station.h"
#include "world.h"
#include "resources.h"
#include "station.h"
#include "station_menu.h"
#include "personnel_menu.h"
#include "resources_menu.h"
#include "planet_menu.h"
#include "ship_list_view.h"
#include "mission_list_menu.h"
#include "market_menu.h"
#include "main_menu.h"
#include "player.h"
#include "hud_window.h"
#include "combat_window.h"

extern int freeBuildMode;

typedef struct
{
    CombatZone *combatZone;
    TextLine    filename;
    int         paused;
    int         round;
}CombatWindowData;


int combat_window_free(Window *win)
{
    CombatWindowData *data;
    if ((!win)||(!win->data))return 0;
    gf2d_window_close_child(win->parent,win);
    data = win->data;
    free(data);
    return 0;
}

int combat_window_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    CombatWindowData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (CombatWindowData*)win->data;
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"options")==0)
        {
            hud_open_options_menu(win->parent);
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


int combat_window_draw(Window *win)
{
    TextLine buffer;
    Vector2D res, position;
    CombatWindowData *data;
    PlayerData *player;
    StationData *station;
    int spacing = 22;

    if ((!win)||(!win->data))return 0;
    player = player_get_data();
    if (!player)return 0;
    res = gf3d_vgraphics_get_resolution();
    data = win->data;
    world_draw(player_get_world());
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
    else 
    {
        gfc_line_sprintf(buffer,"Round: %i",data->round);
    }
    gf2d_font_draw_line_tag(buffer,FT_H6,gfc_color8(255,255,255,255), position);
    position.y += spacing;
    
    gfc_line_sprintf(buffer,"Station HULL: %i / %i",(int)station->hull,(int)station->hullMax);
    gf2d_font_draw_line_tag(buffer,FT_H6,gfc_color8(255,255,255,255), position);

    return 0;
}

void combat_window_setup_camera(Window *win)
{
    CombatWindowData *data;    
    if ((!win)||(!win->data))return;
    data = win->data;
    if (!data->combatZone)return;
    camera_entity_set_look_target(data->combatZone->center);
    camera_entity_set_position(data->combatZone->camera);
    camera_entity_set_auto_pan(1);
    camera_entity_set_look_mode(CTT_Position);
    camera_entity_enable_free_look(0);
}

Window *combat_window(Window *parent,const char *zone)
{
    Window *win;
    CombatWindowData *data;
    if (!zone)return NULL;
    win = gf2d_window_load("menus/combat_window.menu");
    if (!win)
    {
        slog("failed to load hud window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(CombatWindowData),1);
    if (!data)return NULL;
    win->parent = parent;
    win->update = combat_window_update;
    win->free_data = combat_window_free;
    win->draw = combat_window_draw;
    win->data = data;
    data->combatZone = world_get_combat_zone_by_name(zone);
    if (!data->combatZone)
    {
        slog("failed to get a combat zone named %s",zone);
        gf2d_window_free(win);
        return NULL;
    }
    combat_window_setup_camera(win);
    message_buffer_bubble();
    return win;
}
