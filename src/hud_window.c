#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_item_list_menu.h"
#include "gf2d_message_buffer.h"

#include "gf3d_camera.h"

#include "entity.h"
#include "camera_entity.h"
#include "station.h"
#include "world.h"
#include "player.h"
#include "station_menu.h"
#include "hud_window.h"

typedef struct
{
    Entity  *player;
    List    *station_list;
    int     selection;
    Window  *messages;
    World   *w;
}HUDWindowData;

int hud_free(Window *win)
{
    HUDWindowData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    world_delete(data->w);
    gf2d_window_free(data->messages);
    entity_free(data->player);
    free(data);
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
    
    switch(data->selection)
    {
        case 2:
            message_new("4");
            message_new("3");
            message_new("2");
            message_new("1");
            message_new("BOOM");
            break;
    }
}

int hud_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    HUDWindowData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (HUDWindowData*)win->data;
    world_run_updates(data->w);

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"personnel")==0)
        {
            message_new("the beautiful people");
            return 1;
        }
        if (strcmp(e->name,"station")==0)
        {
            if ((win->child)&&(gf2d_window_named(win->child,"station_menu")))
            {
                gf2d_window_free(win->child);
                return 1;
            }
            if ((!win->child)||(!gf2d_window_named(win->child,"station_menu")))
            {
                if (win->child)gf2d_window_free(win->child);
                win->child = station_menu_window(win,player_get_station_data());
            }
            return 1;
        }
        if (strcmp(e->name,"freelook")==0)
        {
            camera_entity_toggle_free_look();
            return 1;
        }
        if (strcmp(e->name,"quick_save")==0)
        {
            player_save("saves/quick.save");
            return 1;
        }

    }
    return 1;
}


int hud_draw(Window *win)
{
    HUDWindowData *data;
    if ((!win)||(!win->data))return 0;
    data = win->data;
    world_draw(data->w);
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
    data->messages = window_message_buffer(5, 500, gfc_color8(0,255,100,255));
    data->player = player_new("saves/default.save");
    camera_entity_enable_free_look(1);
    data->w = world_load("config/world.json");
    gf3d_camera_look_at(vector3d(0,0,0),NULL);
    
    return win;
}
