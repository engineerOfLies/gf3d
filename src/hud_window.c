#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"

#include "entity.h"
#include "camera_entity.h"
#include "station.h"
#include "fighter.h"
#include "player.h"
#include "hud_window.h"

typedef struct
{
    Entity  *player;
    List    *station_list;
    int     selection;
}HUDWindowData;

int hud_free(Window *win)
{
    if (!win)return 0;
    if (!win->data)return 0;

    return 0;
}

void hud_station_selected(void *Data)
{
    Window *win;
    HUDWindowData *data;
    win = Data;
    if ((!win)||(!win->data))return;
    data = (HUDWindowData*)win->data;
    win->child = NULL;
    if (data->selection < 0)return;// nothing selected
}

int hud_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    HUDWindowData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (HUDWindowData*)win->data;
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if ((strcmp(e->name,"station")==0)||(!win->child))
        {
            data->selection = -1;
            data->station_list = gfc_list_new();
            data->station_list = gfc_list_append(data->station_list,"Status");
            data->station_list = gfc_list_append(data->station_list,"Structure");
            data->station_list = gfc_list_append(data->station_list,"Self Destruct");
            win->child = item_list_menu(win,vector2d(10,64),"Station",data->station_list,hud_station_selected,win,&data->selection);
            gfc_list_delete(data->station_list);
        }
        if (strcmp(e->name,"freelook")==0)
        {
            camera_entity_toggle_free_look();
            return 1;
        }
    }
    return 1;
}


int hud_draw(Window *win)
{
    if (!win)return 0;
    return 0;
}

Window *hud_window()
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
    data->player = player_new("config/playerData.cfg");
    camera_entity_enable_free_look(1);
    
    return win;
}
