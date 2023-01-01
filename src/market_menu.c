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
    TextLine entryText;
    int updated;
    char *resource;
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

void market_purchase_ok(Window *win)
{
    TextLine buffer;
    int amount = -1;
    float cost = 0;
    float credits = 0;
    MarketMenuData *data;
    if ((!win)||(!win->data))return;
    data = win->data;
    win->child = NULL;
    amount = abs(atoi(data->entryText));
    if (amount <= 0)
    {
        message_printf("Not ordering any %s",data->resource);
        return;
    }
    cost = resources_list_get_amount(data->salePrice,data->resource);
    if (!cost)
    {
        message_printf("No price information for %s",data->resource);
        return;
    }
    cost *= amount;
    message_printf("ordering %i tons of %s for %.2f credits",amount,data->resource,cost);
    credits = resources_list_get_amount(data->playerSupply,"credits");
    if (credits < cost)
    {
        message_printf("however, you do not have enough credits for the order");
        return;
    }
    gfc_line_sprintf(buffer,"%i",amount);
    resources_list_withdraw(data->playerSupply,"credits",cost);
    mission_begin(
        "commodity_order",
        data->resource,
        buffer,
        player_get_day(),
        player_get_day()+3,//TODO: base this on WHO we are buying from
        0);
    message_printf("It will arrive in %i days",3);
}

void market_purchase_cancel(Window *win)
{
    if (!win)return;
    win->child = NULL;
}

int market_menu_update(Window *win,List *updateList)
{
    int i,count;
    int choice;
    TextLine buffer;
    Resource *resource,*stock;
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
        if ((e->index >= 500)&&(e->index < 600))
        {
            //stock_up
            choice = e->index - 500;
            resource = gfc_list_get_nth(data->playerSupply,choice);
            if (!resource)return 1;
            resources_list_give(data->stockpile,resource->name,100);
            market_menu_update_resources(win);
            return 1;
        }
        if ((e->index >= 600)&&(e->index < 700))
        {
            //stock_up
            choice = e->index - 600;
            resource = gfc_list_get_nth(data->playerSupply,choice);
            if (!resource)return 1;
            stock = resources_list_get(data->stockpile,resource->name);
            if (stock)
            {
                stock->amount -= 100;
                if (stock->amount < 0)stock->amount = 0;
            }
            market_menu_update_resources(win);
            return 1;
        }
        if ((e->index >= 700)&&(e->index < 800))
        {
            //stock_up
            choice = e->index - 700;
            resource = gfc_list_get_nth(data->playerSupply,choice);
            if (!resource)return 1;
            stock = resources_list_get(data->allowSale,resource->name);
            if (stock)
            {
                if (stock->amount > 0)stock->amount = 0;
                else stock->amount = 1;
            }
            else
            {
                resources_list_give(data->allowSale,resource->name,1);
            }
            market_menu_update_resources(win);
            return 1;
        }
        if ((e->index >= 800)&&(e->index < 900))
        {
            //stock_up
            choice = e->index - 800;
            if (win->child)return 1;
            resource = gfc_list_get_nth(data->playerSupply,choice);
            if (!resource)return 1;
            data->resource = resource->name;
            gfc_line_clear(data->entryText);
            gfc_line_sprintf(buffer,"Enter Amount of %s to Purchase",data->resource);
            win->child = window_text_entry(buffer, data->entryText, win, 10, (gfc_work_func*)market_purchase_ok,(gfc_work_func*)market_purchase_cancel);
            return 1;
        }
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
    
    if (resources_list_get_amount(data->allowSale,resource)> 0)
    {
        gfc_line_sprintf(buffer,"Yes");
    }
    else
    {
        gfc_line_sprintf(buffer,"no");
    }
    gf2d_element_list_add_item(rowList,gf2d_button_new_simple(win,700+index,
        "allow",
        "actors/button.actor",
        buffer,
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
    data->allowSale = player_get_allow_sale();
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
