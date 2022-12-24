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
#include "hud_window.h"
#include "station_extension_menu.h"
#include "station_buy_menu.h"
#include "facility_menu.h"
#include "planet.h"
#include "planet_menu.h"

typedef struct
{
    int updated;
    Vector3D viewPosition; //camera position
    Vector2D worldPosition;//site coordinates longitude, latitude
    World *world;
    
}PlanetMenuData;

void planet_menu_set_camera_at_site(Window *win,Vector2D site);

void resource_menu_update_resources(Window *win);

int planet_menu_free(Window *win)
{
    PlanetMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    hud_reset_camera(win->parent);
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int planet_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    PlanetMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (PlanetMenuData*)win->data;

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"north")==0)
        {
            planet_menu_set_camera_at_site(win,vector2d(data->worldPosition.x,data->worldPosition.y + 1));
            return 1;
        }
        if (strcmp(e->name,"south")==0)
        {
            planet_menu_set_camera_at_site(win,vector2d(data->worldPosition.x,data->worldPosition.y - 1));
            return 1;
        }
        if (strcmp(e->name,"east")==0)
        {
            planet_menu_set_camera_at_site(win,vector2d(data->worldPosition.x + 1,data->worldPosition.y));
            return 1;
        }
        if (strcmp(e->name,"west")==0)
        {
            planet_menu_set_camera_at_site(win,vector2d(data->worldPosition.x - 1,data->worldPosition.y));
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

int planet_menu_draw(Window *win)
{
    TextLine buffer;
    PlanetMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    
    gfc_line_sprintf(buffer,"Position: %i, %i",(int)data->worldPosition.x,(int)data->worldPosition.y);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"position"),buffer);

    
    return 0;
}


void planet_menu_set_camera_at_site(Window *win,Vector2D site)
{
    Vector3D position = {0};
    ModelMat *mat;
    PlanetMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    mat = world_get_model_mat(data->world,0);
    if (!mat)return;
    
    if (site.x >= 36)site.x -= 36;// wrapped around the planet
    if (site.x < 0)site.x += 36;
    if (site.y > 8)site.y = 8;// can't hit the poles
    if (site.y < -8)site.y = -8;

    vector2d_copy(data->worldPosition,site);
        
    position = planet_position_to_position(mat->scale.x + 1000, site);
    
    vector3d_add(data->viewPosition,position,mat->position);
    
    camera_entity_set_position(data->viewPosition);
    camera_entity_set_look_target(mat->position);
    camera_entity_set_look_mode(CTT_Position);
    camera_entity_set_auto_pan(0);
    camera_entity_enable_free_look(0);
}

Window *planet_menu(Window *parent)
{
    Window *win;
    PlanetMenuData *data;
    win = gf2d_window_load("menus/planet.menu");
    if (!win)
    {
        slog("failed to load planet window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(PlanetMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = planet_menu_update;
    win->free_data = planet_menu_free;
    win->draw = planet_menu_draw;
    data->world = player_get_world();
    planet_menu_set_camera_at_site(win,vector2d(0,0));
    message_buffer_bubble();
    return win;
}
