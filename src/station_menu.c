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
#include "station_menu.h"

typedef struct
{
    Matrix4 stationMat;
    Vector3D oldPosition;
    Vector3D oldTarget;
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
    camera_entity_set_look_target(data->oldTarget);
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

void station_menu_child_build(void *Data)
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
    if (section)
    {
        message_new("cannot build on that extension slot, it is already in use");
        return;
    }
    win->child = station_buy_menu(win,data->station,data->selection,data->choice,station_def_get_section_list());
}


void station_menu_yes(void *Data)
{
    int parent;
    Window *win;
    StationMenuData *data;
    List *cost;
    win = Data;
    if (!win)return;
    win->child = NULL;
    data = win->data;
    if (!data)return;
    if (!data->selection)return;
    cost = station_get_resource_cost(data->selection->name);
    if (data->selection->parent)
    {
        parent = data->selection->parent->id;
    }else parent = 0;
    resource_list_sell(player_get_resources(), cost,0.9);
    resources_list_free(cost);
    station_remove_section(data->station,data->selection);
    station_menu_select_segment(win,data,parent);
}

void station_menu_no(void *Data)
{
    Window *win;
    win = Data;
    if (!win)return;
    win->child = NULL;
}


int station_menu_update(Window *win,List *updateList)
{
    int index;
    int i,count;
    Element *e;
    StationSection *section;
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
            if (data->selection)
            {
                index = gfc_list_get_item_index(data->station->sections,data->selection);
                if (index > 0)
                {
                    section = gfc_list_get_nth(data->station->sections,index - 1);
                    if (section)
                    {
                        station_menu_select_segment(win,data,section->id);
                    }
                }
            }
            return 1;
        }
        if (strcmp(e->name,"next")==0)
        {
            if (data->selection)
            {
                index = gfc_list_get_item_index(data->station->sections,data->selection);
                if (index > -1)
                {
                    section = gfc_list_get_nth(data->station->sections,index + 1);
                    if (section)
                    {
                        station_menu_select_segment(win,data,section->id);
                    }
                }
            }
            return 1;
        }
        if (strcmp(e->name,"sell")==0)
        {
            if (win->child)return 1;
            if (data->selection)
            {
                if (data->selection->id == 0)
                {
                    message_new("cannot sell the base core of the station!");
                    return 1;
                }
                if (gfc_list_get_count(data->selection->children) > 0)
                {
                    message_new("cannot sell section, it has child extensions!");
                    return 1;
                }
                win->child = window_yes_no("Sell selected Section?", station_menu_yes,station_menu_no,win);
            }
            return 1;
        }
        if (strcmp(e->name,"children")==0)
        {
            if (win->child)//if already open now close it
            {
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
        if (strcmp(e->name,"facilities")==0)
        {
            if (win->child)//if already open now close it
            {
                return 1;
            }
            if ((data->selection)&&(data->selection->facilitySlots > 0))
            {
                win->child = facility_menu(win,data->station, data->selection);
            }
            else
            {
                message_new("Station section cannot support facilities");
            }
        }
        if (strcmp(e->name,"build")==0)
        {
            if (win->child)
            {
                return 1;
            }
            if (data->selection)
            {
                if (gfc_list_get_count(data->selection->children) >= data->selection->expansionSlots)
                {
                    message_new("no free expansion slots for this section");
                    return 1;
                }
                win->child = station_extension_menu(
                    win,
                    vector2d(e->lastDrawPosition.x + e->bounds.w,e->lastDrawPosition.y),
                    data->selection,
                    station_menu_child_build,
                    win,
                    &data->choice);
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
    camera_entity_set_look_target(offset);
    
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
    data->oldTarget = camera_entity_get_look_target();
    station_menu_select_segment(win,data,0);
    message_buffer_bubble();
    return win;
}
