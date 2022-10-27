#include "simple_logger.h"

#include "gfc_callbacks.h"

#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_button.h"
#include "gf2d_font.h"
#include "gf2d_item_list_menu.h"

typedef struct
{
    Callback callback;
    int *result;
}ItemListMenuData;

int item_list_menu_free(Window *win)
{
    ItemListMenuData* data;
    if ((!win)||(!win->data))return 0;
    data = (ItemListMenuData*)win->data;
    gf2d_window_close_child(win->parent,win);
    free(data);
    return 0;
}

int item_list_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    ItemListMenuData* data;
    if ((!win)||(!win->data))return 0;
    if (!updateList)return 0;
    data = (ItemListMenuData*)win->data;
        
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (e->index >= 1000)
        {
            if (data->result)
            {
                *data->result = e->index - 1000;
            }
            gfc_callback_call(&data->callback);
            gf2d_window_free(win);
            return 1;
        }
    }
    return gf2d_window_mouse_in(win);
}

void item_list_menu_add_option(Window *win, const char *option,int index)
{
    Element *list;
    Element *be,*le;
    
    LabelElement *label;

    if ((!win)||(!option))return;
    list = gf2d_window_get_element_by_name(win,"options");
    if (!list)
    {
        slog("window missing options element");
        return;
    }
    
    label = gf2d_element_label_new_full(option,gfc_color8(200,255,255,255),FT_Small,LJ_Left,LA_Middle,0);

    be = gf2d_element_new_full(
        list,
        1000+index,
        (char *)option,
        gfc_rect(0,0,336,24),
        gfc_color(0.9,0.9,0.6,1),
        0,
        gfc_color(.5,.5,.5,1),0,win);
    le = gf2d_element_new_full(
        be,
        2000+index,
        (char *)option,
        gfc_rect(0,0,336,24),
        gfc_color(0.9,0.9,0.6,1),
        0,
        gfc_color(1,1,1,1),0,win);
    
    gf2d_element_make_label(le,label);
    gf2d_element_make_button(be,gf2d_element_button_new_full(le,NULL,gfc_color(1,1,1,1),gfc_color(0.9,0.9,0.9,1),0));
    gf2d_element_list_add_item(list,be);
}

void item_list_menu_add_all_options(Window *win,List *options)
{
    const char *item;
    int i,c;
    if ((!win)||(!options))return;
    c = gfc_list_get_count(options);
    gf2d_window_set_dimensions(
        win,
        gfc_rect(win->dimensions.x,win->dimensions.y,win->dimensions.w,win->dimensions.h + c * 36));
    for (i = 0;i < c;i++)
    {
        item = gfc_list_get_nth(options,i);
        if (!item)continue;
        item_list_menu_add_option(win, item,i);
    }
}


Window *item_list_menu(Window *parent,Vector2D position,char *question,List *options,void(*onSelect)(void *),void *callbackData,int *result)
{
    Window *win;
    ItemListMenuData* data;
    win = gf2d_window_load("menus/item_list_menu.json");
    if (!win)
    {
        slog("failed to load editor menu");
        return NULL;
    }
    win->update = item_list_menu_update;
    win->free_data = item_list_menu_free;
    data = gfc_allocate_array(sizeof(ItemListMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->data = data;
    win->parent = parent;
    data->result = result;
    data->callback.data = callbackData;
    data->callback.callback = onSelect;
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),question);
    gf2d_window_set_position(win,position);
    item_list_menu_add_all_options(win,options);
    return win;
}

/*eol@eof*/
