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
#include "station_def.h"
#include "station.h"
#include "player.h"
#include "station_extension_menu.h"
#include "station_buy_menu.h"
#include "station_menu.h"

typedef struct
{
    Matrix4 stationMat;
    Vector3D oldPosition;
    StationData *station;
    StationSection *selection;
    int choice;
}StationMenuData;

void station_menu_select_segment(Window *win,StationMenuData *data,int segment);

int station_menu_free(Window *win)
{
    StationMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    if (data->station)
    {
        data->station->sectionHighlight = -1;
    }

    gf3d_camera_set_position(data->oldPosition);
    free(data);
    return 0;
}

void station_menu_child_select(void *Data)
{
    Window *win;
    StationMenuData *data;
    StationSection *section;
    win = Data;
    if (!win)return;
    data = win->data;
    if ((!data)||(!data->selection))return;
    win->child = NULL;
    if (data->choice == -1)return;
    section = station_section_get_child_by_slot(data->selection,data->choice);
    if (!section)return;
    station_menu_select_segment(win,data,section->id);
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
        if (strcmp(e->name,"parent")==0)
        {
            if ((data->selection)&&(data->selection->parent))
            {
                station_menu_select_segment(win,data,data->selection->parent->id);
            }
            return 1;
        }
        if (strcmp(e->name,"prev")==0)
        {
            if ((data->selection)&&(data->selection->id > 0))
            {
                station_menu_select_segment(win,data,data->selection->id - 1);
            }
            return 1;
        }
        if (strcmp(e->name,"next")==0)
        {
            if (data->selection)
            {
                station_menu_select_segment(win,data,data->selection->id + 1);
            }
            return 1;
        }
        if (strcmp(e->name,"children")==0)
        {
            if (win->child)//if already open now close it
            {
                if (!gf2d_window_named(win->child,"station_extension_menu"))return 1;
                gf2d_window_free(win->child);
                win->child = NULL;
                return 1;
            }
            if ((data->selection)&&(data->selection->children))
            {
                win->child = station_extension_menu(
                    win,
                    vector2d(e->lastDrawPosition.x + e->bounds.w,e->lastDrawPosition.y),
                    data->selection,
                    station_menu_child_select,
                    win,
                    &data->choice);

            }
            return 1;
        }
        if (strcmp(e->name,"build")==0)
        {
            if (win->child)
            {
                if (!gf2d_window_named(win->child,"station_buy_menu"))return 1;
                gf2d_window_free(win->child);
                win->child = NULL;
                return 1;
            }
            if ((data->selection)&&(data->selection->expansionSlots))
            {
                win->child = station_buy_menu(win,data->selection,station_def_get_section_list());
            }
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


int station_menu_draw(Window *win)
{
//    StationMenuData *data;
    if ((!win)||(!win->data))return 0;
  //  data = win->data;
    return 0;
}

void station_menu_select_segment(Window *win,StationMenuData *data,int segment)
{
    Matrix4 mat;
    const char *display_name;
    StationData *station;
    StationSection *section;
    Vector3D camera = {-672.546875,-584.498535,151.30999};
    Vector3D offset = {0};
    
    if ((!win)||(!data)||(!data->station))return;
    
    station = data->station;
    section = station_get_section_by_id(data->station,segment);
    
    if ((!section)||(!station))
    {
        return;
    }
    data->selection = section;
    station->sectionHighlight = segment;
    
    gfc_matrix4_from_vectors(
            mat,
            station->mat->position,
            station->mat->rotation,
            station->mat->scale);
    gfc_matrix_multiply(mat,section->mat.mat,mat);    
    gfc_matrix4_to_vectors(mat,&offset,NULL,NULL);

    vector3d_add(camera,camera,offset);
    gf3d_camera_look_at(offset,&camera);
    
    display_name = station_def_get_display_name(section->name);
    if (display_name)
    {
        gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"section_name"),display_name);
    }
}

Window *station_menu_window(Window *parent,StationData *station)
{
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
    data->station = station;
    data->oldPosition = gf3d_camera_get_position();
    station_menu_select_segment(win,data,0);
    message_buffer_bubble();
    return win;
}
