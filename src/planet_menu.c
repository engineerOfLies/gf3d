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
#include "station.h"
#include "player.h"
#include "hud_window.h"
#include "facility_buy_menu.h"
#include "facility_menu.h"
#include "planet.h"
#include "planet_menu.h"

typedef struct
{
    int updated;
    Vector3D viewPosition; //camera position
    Vector2D worldPosition;//site coordinates longitude, latitude
    PlanetData *planet;
    List *typeList;
}PlanetMenuData;

void planet_menu_set_camera_at_site(Window *win,Vector2D site);

void resource_menu_update_resources(Window *win);

int planet_menu_free(Window *win)
{
    PlanetMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    if (win->child)
    {
        gf2d_window_free(win->child);
    }
    gfc_list_delete(data->typeList);
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
        if (strcmp(e->name,"facilities")==0)
        {
            if (win->child)return 1;
            win->child = facility_menu(
                win,
                data->planet->facilities,
                gfc_list_get_count(data->planet->facilities),
                NULL);
            return 1;
        }
        if (strcmp(e->name,"build")==0)
        {
            if (win->child)return 1;
            win->child = facility_buy_menu(win,data->planet->facilities, data->typeList,data->worldPosition);
            return 1;
        }
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
    StationFacility *facility;
    SiteData *site;
    PlanetMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    
    facility = station_facility_get_by_position(data->planet->facilities,data->worldPosition);
    
    if (facility)
    {
        gfc_line_sprintf(buffer,"Facility: %s",station_facility_get_display_name(facility->name));
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"facility"),buffer);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"facility_view"), 0);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"sell"), 0);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"build"), 1);        
    }
    else
    {
        gfc_line_sprintf(buffer,"Facility: <NONE>");
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"facility"),buffer);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"facility_view"), 1);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"sell"), 1);
        gf2d_element_set_hidden(gf2d_window_get_element_by_name(win,"build"), 0);
    }

    gfc_line_sprintf(buffer,"Position: %i, %i",(int)data->worldPosition.x,(int)data->worldPosition.y);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"position"),buffer);

    gfc_line_sprintf(buffer,"Facilities: %i",gfc_list_get_count(data->planet->facilities));
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"facility_count"),buffer);
    
    site = planet_get_site_data_by_position(data->planet,data->worldPosition);

    if (site)
    {
        if (site->surveyed)
        {
            gfc_line_sprintf(buffer,"Surveyed: Yes");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"survey"),buffer);

            gfc_line_sprintf(buffer,"Nutrient Level: %i",site->resources[SRT_Nutrients]);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"nutrients"),buffer);

            gfc_line_sprintf(buffer,"Mineral Deposits: %i",site->resources[SRT_Minerals]);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"minerals"),buffer);

            gfc_line_sprintf(buffer,"Ore Deposits: %i",site->resources[SRT_Ores]);
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"ores"),buffer);
        }
        else
        {
            gfc_line_sprintf(buffer,"Surveyed: No");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"survey"),buffer);

            gfc_line_sprintf(buffer,"Nutrient Level: unknown");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"nutrients"),buffer);

            gfc_line_sprintf(buffer,"Mineral Deposits: unknown");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"minerals"),buffer);

            gfc_line_sprintf(buffer,"Ore Deposits: unknown");
            gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"ores"),buffer);
        }
    }
    

    return 0;
}


void planet_menu_set_camera_at_site(Window *win,Vector2D site)
{
    Vector3D position = {0};
    ModelMat *mat;
    PlanetMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    mat = &data->planet->mat;
    if (!mat)return;
    
    if (site.x >= 36)site.x -= 36;// wrapped around the planet
    if (site.x < 0)site.x += 36;
    if (site.y > 8)site.y = 8;// can't hit the poles
    if (site.y < -8)site.y = -8;

    vector2d_copy(data->worldPosition,site);
        
    position = planet_position_to_position(data->planet->radius + 1000, site);
    
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
    List *typeList = NULL;
    static char *list = "planetary";
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
    typeList = gfc_list_new();
    typeList = gfc_list_append(typeList,list);
    data->typeList = station_facility_get_possible_from_list(typeList);
    gfc_list_delete(typeList);

    data->planet = player_get_planet();
    
    planet_menu_set_camera_at_site(win,vector2d(0,0));
    message_buffer_bubble();
    return win;
}
