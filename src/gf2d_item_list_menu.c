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
    GFC_Callback        callback;
    int                *result;
    int                 itemCount;
    Uint8               allowClose;
    Uint8               persist;
    FontTypes           font;
    GFC_Color           fontBase;
    GFC_Color           backgroundBase;
    GFC_Color           Selected;
    ButtonCustomAction  highStyle;
    int                 selectedOption;
}ItemListMenuData;

int item_list_menu_free(Window *win)
{
    ItemListMenuData* data;
    if ((!win)||(!win->data))return 0;
    data = (ItemListMenuData*)win->data;
    free(data);
    return 0;
}

int item_list_menu_update(Window *win,GFC_List *updateList)
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
            if (!data->persist)gf2d_window_free(win);
            return 1;
        }
        else if (strcmp(e->name,"item_down")==0)
        {
            data->selectedOption++;
            if (data->selectedOption >= data->itemCount)data->selectedOption = 0;
            gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,1000 + data->selectedOption));
            return 1;
        }
        else if (strcmp(e->name,"item_up")==0)
        {
            data->selectedOption--;
            if (data->selectedOption < 0 )data->selectedOption = data->itemCount - 1;
            gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,1000 + data->selectedOption));
            return 1;
        }
        else if ((data->allowClose)&&(strcmp(e->name,"close")==0))
        {
            if (data->result)
            {
                *data->result = -1;
            }
            gfc_callback_call(&data->callback);
            gf2d_window_free(win);
            return 1;
        }
    }
    return 0;
}

void item_list_menu_add_option(Window *win, const char *option,int index)
{
    Element *list;
    Element *be,*le;
    GFC_Vector2D size;
    
    ButtonElement *button;
    LabelElement *label;
    
    ItemListMenuData* data;
    if ((!win)||(!option))return;
    data = win->data;
    list = gf2d_window_get_element_by_name(win,"options");
    if (!list)
    {
        slog("window missing options element");
        return;
    }
    size = gf2d_element_list_get_item_size(list);
    label = gf2d_element_label_new_full(option,data->fontBase,data->font,LJ_Left,LA_Middle,0);

    be = gf2d_element_new_full(
        list,
        1000+index,
        (char *)option,
        gfc_rect(0,0,size.x,size.y),
        data->fontBase,
        0,
        data->backgroundBase,0,win);
    le = gf2d_element_new_full(
        be,
        2000+index,
        (char *)option,
        gfc_rect(0,0,size.x,size.y),
        data->fontBase,
        0,
        data->backgroundBase,0,win);
    
    gf2d_element_make_label(le,label);
    
    button = gf2d_element_button_new_full(le,NULL,data->Selected,GFC_COLOR_DARKGREY,0);
    button->customActions = data->highStyle;
    gf2d_element_make_button(be,button);
    be->canHasFocus = 1;
    gf2d_element_list_add_item(list,be);
}

void item_list_menu_add_all_options(Window *win,GFC_List *options)
{
    const char *item;
    int i,c;
    if ((!win)||(!options))return;
    c = gfc_list_get_count(options);
    if (win->dimensions.h < ((c + 1) * 24))//only expand the list
    {
        gf2d_window_set_dimensions(
            win,
            gfc_rect(win->dimensions.x,win->dimensions.y,win->dimensions.w,(c + 1) * 24));
    }
    for (i = 0;i < c;i++)
    {
        item = gfc_list_get_nth(options,i);
        if (!item)continue;
        item_list_menu_add_option(win, item,i);
    }
}


Window *item_list_menu(
    Window *parent,
    GFC_Vector2D position,
    Uint8     persist,
    Uint8     allowClose,
    const char     *question,
    FontTypes questionFont,
    GFC_Color questionColor,
    GFC_List *options,
    FontTypes font,
    GFC_Color fontBase,
    GFC_Color backgroundBase,
    GFC_Color Selected,
    ButtonCustomAction highStyle,
    void(*onSelect)(void *),
    void *callbackData,
    int *result)
{
    Window *win;
    Element *title;
    ItemListMenuData* data;
    win = gf2d_window_load("menus/item_list.menu");
    if (!win)
    {
        slog("failed to load item_list.menu");
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
    data->itemCount = gfc_list_count(options);
    data->font = font;
    data->fontBase = fontBase;
    data->backgroundBase = backgroundBase;
    data->Selected = Selected;
    data->highStyle = highStyle;
    data->persist = persist;
    data->allowClose = allowClose;
    //set title
    title = gf2d_window_get_element_by_name(win,"title");
    gf2d_element_label_set_text(title,question);
    gf2d_element_label_set_font(title,questionFont);
    title->color = questionColor;
    gf2d_window_set_position(win,position);
    item_list_menu_add_all_options(win,options);
    data->selectedOption = 0;
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,1000 + data->selectedOption));
    return win;
}

/*eol@eof*/
