#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"

#include "entity.h"
#include "camera_entity.h"
#include "station.h"
#include "player.h"
#include "station_menu.h"

typedef struct
{
    Vector3D oldPosition;
    StationData *station;
}StationMenuData;

int station_menu_free(Window *win)
{
    StationMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    gf3d_camera_set_position(data->oldPosition);
    free(data);
    return 0;
}

int station_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    StationMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (StationMenuData*)win->data;

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
    }
    if (gfc_input_command_released("cancel"))
    {
        gf2d_window_free(win);
        return 1;
    }
    return 0;
}


int station_menu_draw(Window *win)
{
    StationMenuData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    return 0;
}

Window *station_menu_window(Window *parent,StationData *station)
{
    Vector3D camera = {-672.546875,-584.498535,151.30999};
    Window *win;
    StationMenuData *data;
    win = gf2d_window_load("menus/station.menu");
    if (!win)
    {
        slog("failed to load hud window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(StationMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = station_menu_update;
    win->free_data = station_menu_free;
    win->draw = station_menu_draw;
    data->oldPosition = gf3d_camera_get_position();
    camera_look_at(vector3d(0,0,0),&camera);
    
    return win;
}
