#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_list.h"
#include "gf2d_element_button.h"
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
#include "market_menu.h"

typedef struct
{
    int updated;
    List *playerSupply;
    List *stockpile;
    List *salePrice;
    List *allowSale;
}MarketMenuData;

void market_menu_update_resources(Window *win);

int market_menu_free(Window *win)
{
    MarketMenuData *data;
    if (!win)return 0;
    gf2d_window_close_child(win->parent,win);
    if (!win->data)return 0;
    data = win->data;
    free(data);
    return 0;
}

int market_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    MarketMenuData *data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (MarketMenuData*)win->data;

    if (data->updated != player_get_day())
    {
        market_menu_update_resources(win);
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"done")==0)
        {
            gf2d_window_free(win);
            return 1;
        }
    }
    return 0;
}


int market_menu_draw(Window *win)
{
//    MarketMenuData *data;
    if ((!win)||(!win->data))return 0;
//    data = win->data;
    return 0;
}

Element *maket_menu_build_row(Window *win, const char *resource,int index, int amount)
{
    TextLine buffer;
    int stockpile;
    float price;
    Element *rowList;
    ListElement *le;
    MarketMenuData *data;
    if ((!win)||(!win->data)||(!resource))return NULL;
    data = win->data;
    
    le = gf2d_element_list_new_full(gfc_rect(0,0,1,1),vector2d(120,24),LS_Horizontal,0,0,1,0);
    rowList = gf2d_element_new_full(
        NULL,0,(char *)resource,
        gfc_rect(0,0,1,1),
        gfc_color(1,1,1,1),0,
        gfc_color(0,0,0,1),0,win);
    gf2d_element_make_list(rowList,le);
    
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,resource,FT_H6,vector2d(200,24),gfc_color(1,1,1,1)));
    gfc_line_sprintf(buffer,"%i",amount);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),gfc_color(1,1,1,1)));
    
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,500+index,
        "stock_up",
        "actors/button.actor",
        "+",
        vector2d(0.189,0.5),
        vector2d(30,24),
        gfc_color8(255,255,255,255)));
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,600+index,
        "stock_down",
        "actors/button.actor",
        "-",
        vector2d(0.189,0.5),
        vector2d(30,24),
        gfc_color8(255,255,255,255)));
    
    stockpile = (int)resources_list_get_amount(data->stockpile,resource);    
    gfc_line_sprintf(buffer,"%i",stockpile);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(90,24),gfc_color(1,1,1,1)));
        
    price = resources_list_get_amount(data->salePrice,resource);    
    gfc_line_sprintf(buffer,"%.2f",price);
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0,buffer,FT_H6,vector2d(120,24),gfc_color(1,1,1,1)));
    
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,700+index,
        "allow",
        "actors/button.actor",
        "set price",
        vector2d(0.63,0.56),
        vector2d(100,24),
        gfc_color8(255,255,255,255)));
    gf2d_element_list_add_item(rowList,gf2d_label_new_simple_size(win,0," ",FT_H6,vector2d(20,24),gfc_color(1,1,1,1)));
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,800+index,
        "allow",
        "actors/button.actor",
        "purchase",
        vector2d(0.63,0.56),
        vector2d(100,24),
        gfc_color8(255,255,255,255)));

    return rowList;
    
}

void market_menu_update_resources(Window *win)
{
    int i,c;
    Resource *resource;
    Element *list,*e;
    MarketMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    data->playerSupply = player_get_resources();
    data->stockpile = player_get_stockpile();
    data->salePrice = player_get_sale_price();
    list = gf2d_window_get_element_by_name(win,"commodities");
    gf2d_element_list_free_items(list);
    if (!list)return;
    c = gfc_list_get_count(data->playerSupply);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(data->playerSupply,i);
        if (!resource)continue;
        if (!resource_is_commodity(resource->name))continue;
        e = maket_menu_build_row(win, resource->name,i, (int)resource->amount);
        if (!e)continue;
        gf2d_element_list_add_item(list,e);
    }
    data->updated = player_get_day();
}

Window *market_menu(Window *parent)
{
    Window *win;
    MarketMenuData *data;
    win = gf2d_window_load("menus/market.menu");
    if (!win)
    {
        slog("failed to load market window");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(MarketMenuData),1);
    if (!data)
    {
        gf2d_window_free(win);
        return NULL;
    }
    win->parent = parent;
    win->data = data;
    win->update = market_menu_update;
    win->free_data = market_menu_free;
    win->draw = market_menu_draw;
    market_menu_update_resources(win);
    message_buffer_bubble();
    return win;
}
