#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"

#include "entity.h"
#include "station.h"
#include "fighter.h"
#include "hud_window.h"

typedef struct
{
    Entity *station;
}HUDWindowData;

int hud_free(Window *win)
{
    if (!win)return 0;
    if (!win->data)return 0;

    return 0;
}

int hud_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
//    HUDWindowData *data;
    if (!win)return 0;
    if (!updateList)return 0;
//    data = (HUDWindowData*)win->data;
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"station")==0)
        {
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
    data->station = station_new(vector3d(0,-1000,0));
    
    fighter_new(vector3d(-900,900,300));
    return win;
}
