#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_button.h"
#include "gf2d_element_scrollbar.h"

void gf2d_element_scrollbar_update_range(Element *element);


void gf2d_element_scrollbar_draw(Element *element,GFC_Vector2D offset)
{
    GFC_Rect rect;
    GFC_Vector2D position;
    ScrollbarElement *data;
    if (!element)return;
    data = (ScrollbarElement*)element->data;

    gfc_vector2d_add(position,offset,element->drawBounds);
    //draw background of the sliders
    gfc_rect_set(rect,
        offset.x + element->drawBounds.x + 20,
        offset.y + element->drawBounds.y + 20,
        element->drawBounds.w -25,
        element->drawBounds.h -40);
    gf2d_draw_rect_filled(rect,element->backgroundColor);
    
    gf2d_element_draw(data->scrollUp,position);
    gf2d_element_draw(data->scrollSlider,position);
    gf2d_element_draw(data->scrollDown,position);
}

Element *gf2d_element_scrollbar_get_next(Element *element, Element *from)
{
    ScrollbarElement *data;
    if ((!element)||(!element->data))return NULL;
    data = (ScrollbarElement*)element->data;
    if (element == from)
    {
        if (data->scrollUp)return data->scrollUp;
        if (data->scrollDown)return data->scrollDown;
        if (data->scrollSlider)return data->scrollSlider;
        return from;
    }
    if (data->scrollUp == from)
    {
        if (data->scrollDown)return data->scrollDown;
        if (data->scrollSlider)return data->scrollSlider;
        return from;
    }
    if (data->scrollDown == from)
    {
        if (data->scrollSlider)return data->scrollSlider;
        return from;
    }
    if (from == data->scrollSlider)return from;//search item was my last child, return me
    return NULL;
}

void gf2d_element_scrollbar_scroll_up(Element *element)
{
    ScrollbarElement *data;
    if ((!element)||(!element->data))return;
    data = (ScrollbarElement*)element->data;
    if (data->position > 0)
    {
        gf2d_element_list_set_scroll_offset(data->list,--data->position);
    }
}

void gf2d_element_scrollbar_scroll_down(Element *element)
{
    ScrollbarElement *data;
    if ((!element)||(!element->data))return;
    data = (ScrollbarElement*)element->data;
    if (data->position < data->scrollCount)
    {
        gf2d_element_list_set_scroll_offset(data->list,++data->position);
    }
}

GFC_List *gf2d_element_scrollbar_update(Element *element,GFC_Vector2D offset)
{
    GFC_List *list;
    GFC_Rect bounds;
    ScrollbarElement *data;
    if (!element)return NULL;
    data = (ScrollbarElement*)element->data;
    if (!data)return NULL;
    if (data->list)//if we have a target list, make sure the mouse in relative to it
    {
        bounds = gf2d_element_get_absolute_bounds(data->list,offset);
        bounds.x = data->list->lastDrawPosition.x;
        bounds.y = data->list->lastDrawPosition.y;
        gf2d_element_scrollbar_update_range(element);
    }
    else
    {   
        bounds = gf2d_element_get_absolute_bounds(element,offset);
        bounds.x = element->lastDrawPosition.x;
        bounds.y = element->lastDrawPosition.y;
    }
    
    list = gf2d_element_update(data->scrollUp, offset);
    if (list)
    {
        gfc_list_delete(list);
        gf2d_element_scrollbar_scroll_up(element);
        return NULL;        
    }
    list = gf2d_element_update(data->scrollDown, offset);
    if (list)
    {
        gfc_list_delete(list);
        gf2d_element_scrollbar_scroll_down(element);
        return NULL;        
    }
    if (gfc_input_command_pressed("panup"))
    {
        gf2d_element_scrollbar_scroll_up(element);
        return NULL;
    }
    if (gfc_input_command_pressed("pandown"))
    {
        gf2d_element_scrollbar_scroll_down(element);
        return NULL;
    }
    if (gf2d_mouse_hidden() <= 0)
    {
        if(gf2d_mouse_in_rect(bounds))
        {
            if (gfc_input_mouse_wheel_up())
            {
                gf2d_element_scrollbar_scroll_up(element);
                return NULL;
            }
            if (gfc_input_mouse_wheel_down())
            {
                gf2d_element_scrollbar_scroll_down(element);
                return NULL;
            }
        }
    }
    return NULL;
}

void gf2d_element_scrollbar_update_range(Element *element)
{
    int total;
    ScrollbarElement *data;
    if ((!element)||(!element->data))return;
    data = (ScrollbarElement*)element->data;
    if (!data->list)return;
    total = gf2d_element_list_get_item_count(data->list);
    data->scrollCount = total - gf2d_element_list_get_row_count(data->list);//the amount we have to scroll

}

void gf2d_element_scrollbar_set_list_target(Element *scrollbar,Element *list)
{
    ScrollbarElement *data;
    if ((!scrollbar)||(!scrollbar->data))return;
    data = (ScrollbarElement*)scrollbar->data;
    data->list = list;
    if (data->list != NULL)
    {
        gf2d_element_scrollbar_update_range(scrollbar);
    }
}


void gf2d_element_scrollbar_free(Element *element)
{
    ScrollbarElement *data;
    if (!element)return;
    data = (ScrollbarElement*)element->data;
    if (!data)return;
    gf2d_element_free(data->scrollUp);
    gf2d_element_free(data->scrollDown);
    gf2d_element_free(data->scrollSlider);
    //NOT the list, we don't own that
    free(data);
}

ScrollbarElement *gf2d_element_scrollbar_new()
{
    ScrollbarElement *scrollbar;
    scrollbar = gfc_allocate_array(sizeof(ScrollbarElement),1);
    return scrollbar;
}

ScrollbarElement *gf2d_element_scrollbar_new_full()
{
    ScrollbarElement *scrollbar;
    scrollbar = gf2d_element_scrollbar_new();
    if (!scrollbar)return NULL;
    
    
    
    return scrollbar;
}

Element *gf2d_element_scrollbar_get_by_name(Element *e,const char *name)
{
    ScrollbarElement *bar;
    Element *r;
    if ((!e)||(!e->data))return NULL;
    bar = (ScrollbarElement*)e->data;
    r = gf2d_get_element_by_name(bar->scrollUp,name);
    if (r)return r;
    r = gf2d_get_element_by_name(bar->scrollDown,name);
    if (r)return r;
    return gf2d_get_element_by_name(bar->scrollSlider,name);
}

void gf2d_element_scrollbar_recalibrate(Element *e)
{
    ScrollbarElement *bar;
    if ((!e)||(!e->data))return;
    bar = (ScrollbarElement*)e->data;
    gf2d_element_recalibrate(bar->scrollUp);
    gf2d_element_recalibrate(bar->scrollDown);
}

void gf2d_element_make_scrollbar(Element *e,ScrollbarElement *scrollbar)
{
    if (!e)return;
    e->data = scrollbar;
    e->type = ET_Scrollbar;
    e->draw = gf2d_element_scrollbar_draw;
    e->update = gf2d_element_scrollbar_update;
    e->free_data = gf2d_element_scrollbar_free;
    e->get_by_name = gf2d_element_scrollbar_get_by_name;
    e->get_next = gf2d_element_scrollbar_get_next;
    e->recalibrate = gf2d_element_scrollbar_recalibrate;
}

void gf2d_element_scrollbar_load_from_config(Element *e,SJson *json,Window *win)
{
    const char *str;
    ScrollbarElement *bar;
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
    
    gf2d_element_make_scrollbar(e,gf2d_element_scrollbar_new());
    bar = e->data;
    if (!bar)return;
    str = sj_object_get_value_as_string(json,"list_target");
    if (str)
    {
        gf2d_element_scrollbar_set_list_target(e,gf2d_window_get_element_by_name(e->win,str));
    }
    else
    {
        if (e->parent->type == ET_GFC_List)
        {
            gf2d_element_scrollbar_set_list_target(e,e->parent);
        }
    }
    if (bar->list)
    {
        e->bounds.x = bar->list->bounds.w - 40; 
        e->bounds.w = 32; 
        e->bounds.h = bar->list->bounds.h; 
    }
    gf2d_element_recalibrate(e);
    str = sj_object_get_value_as_string(json,"style");
    if (str)
    {
        if (strcmp(str,"horizontal") == 0)
        {
            bar->scrollStyle = LS_Horizontal;
        }
        if (strcmp(str,"vertical") == 0)
        {
            bar->scrollStyle = LS_Vertical;
        }
    }
    //TODO: check if the config overrides default arrow buttons
    bar->scrollUp = gf2d_button_new_simple(
        e->win,
        0,
        "scrollup",
        "actors/arrow_button_up.json",
        " ",
        gfc_vector2d(0.5,0.5),
        gfc_vector2d(32,20),
        GFC_COLOR_WHITE);
    bar->scrollDown = gf2d_button_new_simple(
        e->win,
        0,
        "scrolldown",
        "actors/arrow_button_up.json",
        " ",
        gfc_vector2d(0.5,-0.5),
        gfc_vector2d(32,20),
        GFC_COLOR_WHITE);
    bar->scrollDown->bounds.y = e->bounds.h - bar->scrollDown->bounds.h;


    str = sj_object_get_value_as_string(json,"list_target");
    if (str)
    {
        gf2d_element_scrollbar_set_list_target(e,gf2d_window_get_element_by_name(e->win,str));
    }
    else
    {
        if (e->parent->type == ET_GFC_List)
        {
            gf2d_element_scrollbar_set_list_target(e,e->parent);
        }
    }
}


/*eol@eof*/
