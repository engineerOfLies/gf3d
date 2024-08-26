#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"
#include "gfc_shape.h"

#include "gf2d_draw.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_button.h"
#include "gf2d_element_entry.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"
#include "gf2d_element_scrollbar.h"
#include "gf2d_elements.h"

extern int __DEBUG;

Element *gf2d_element_new()
{
    Element *e;
    e = (Element *)malloc(sizeof(Element));
    if (!e)
    {
        slog("failed to allocate a new window element");
        return NULL;
    }
    memset(e,0,sizeof(Element));
    return e;
}

Element *gf2d_element_new_full(
    Element *parent,
    int      index,
    GFC_TextLine name,
    GFC_Rect bounds,
    GFC_Color color,
    int state,
    GFC_Color backgroundColor,
    int backgroundDraw,
    Window *win
)
{
    Element *e;
    e = gf2d_element_new();
    if (!e)return NULL;
    if (name)
    {
        gfc_line_cpy(e->name,name);
    }
    e->index = index;
    e->color = color;
    e->state = state;
    e->bounds = bounds;
    
    e->backgroundColor = backgroundColor;
    e->backgroundDraw = backgroundDraw;
    e->win = win;
    return e;
}

void gf2d_element_free(Element *e)
{
    if (!e)return;
    if (e->free_data)
    {
        e->free_data(e);
    }
    free(e);
}

GFC_Vector2D gf2d_element_get_draw_position(Element *e)
{
    if (!e)return gfc_vector2d(0,0);
    return e->lastDrawPosition;
}

void gf2d_element_set_hidden(Element *element, int hidden)
{
    if (!element)return;
    element->hidden = hidden;
}

void gf2d_element_draw(Element *e, GFC_Vector2D offset)
{
    GFC_Rect rect;
    if ((!e)||(e->hidden))
    {
        return;
    }
    gfc_rect_set(rect,offset.x + e->drawBounds.x,offset.y + e->drawBounds.y,e->drawBounds.w,e->drawBounds.h);
    e->lastDrawPosition.x = rect.x;
    e->lastDrawPosition.y = rect.y;
    if (e->backgroundDraw)
    {
        gf2d_draw_rect_filled(rect,e->backgroundColor);
    }
    if (e->draw)e->draw(e,offset);
    if (__DEBUG)
    {
        gf2d_draw_rect(rect,gfc_color8(100,255,100,255));
    }
}

GFC_List * gf2d_element_update(Element *e, GFC_Vector2D offset)
{
    if ((!e)||(e->hidden))
    {
        return NULL;
    }
    if (e->update)return e->update(e,offset);
    return NULL;
}

void gf2d_element_calibrate(Element *e,Element *parent, Window *win)
{
    GFC_Rect res = {0};
    int negx = 0,negy = 0;
    if (!e)return;
    if (parent != NULL)
    {
        gfc_rect_copy(res,parent->drawBounds);
    }
    else if (win != NULL)
    {
        gfc_rect_copy(res,win->dimensions);
    }
    else
    {
        slog("error: need a parent element or a window");
        return;
    }
    gfc_rect_copy(e->drawBounds,e->bounds);
    if (e->bounds.x < 0)
    {
        negx = 1;
        e->drawBounds.x *= -1;
    }
    if (e->bounds.y < 0)
    {
        negy = 1;
        e->drawBounds.y *= -1;
    }
    if ((e->drawBounds.x > 0)&&(e->drawBounds.x < 1.0))
    {
        e->drawBounds.x *= res.w;
    }
    if ((e->drawBounds.y > 0)&&(e->drawBounds.y < 1.0))
    {
        e->drawBounds.y *= res.h;
    }
    if ((e->bounds.w > 0)&&(e->bounds.w <= 1.0))
    {
        e->drawBounds.w *= res.w;
    }
    if ((e->bounds.h > 0)&&(e->bounds.h <= 1.0))
    {
        e->drawBounds.h *= res.h;
    }
    
    if (negx)
    {
        e->drawBounds.x = res.w - e->drawBounds.x;
    }
    if (negy)
    {
        e->drawBounds.y = res.h - e->drawBounds.y;
    }
}

void gf2d_element_recalibrate(Element *e)
{
    if (!e)return;
    gf2d_element_calibrate(e,e->parent, e->win);
    if (e->recalibrate)e->recalibrate(e);
}

Element *gf2d_element_load_from_config(SJson *json,Element *parent,Window *win)
{
    Element *e = NULL;
    SJson *value;
    const char *type;
    GFC_Vector4D gfc_vector ={0,0,0,0};
    if (!sj_is_object(json))return NULL;
    e = gf2d_element_new();
    if (!e)return NULL;
    e->parent = parent;
    value = sj_object_get_value(json,"name");
    if (value)
    {
        gfc_line_cpy(e->name,sj_get_string_value(value));
    }
    
    e->win = win;
    value = sj_object_get_value(json,"id");
    sj_get_integer_value(value,&e->index);

    value = sj_object_get_value(json,"state");
    sj_get_integer_value(value,&e->index);

    e->color = sj_value_as_color(sj_object_get_value(json,"color"));
    e->backgroundColor = sj_value_as_color(sj_object_get_value(json,"backgroundColor"));

    value = sj_object_get_value(json,"backgroundDraw");
    if (value)
    {
        sj_get_integer_value(value,&e->backgroundDraw);
    }
    sj_get_bool_value(sj_object_get_value(json,"canHasFocus"),(short int *)&e->canHasFocus);

    value = sj_object_get_value(json,"bounds");
    if (value)
    {
        sj_value_as_vector4d(value,&gfc_vector);
        gfc_rect_set(e->bounds,gfc_vector.x,gfc_vector.y,gfc_vector.z,gfc_vector.w);
    }
    else if (parent)
    {
        gfc_rect_copy(e->bounds,parent->bounds);
    }
    gf2d_element_calibrate(e,parent, win);
    
    type = sj_object_get_value_as_string(json,"type");
    if (type)
    {
        if (strcmp(type,"list") == 0)
        {
            gf2d_element_load_list_from_config(e,json,win);
        }
        else if (strcmp(type,"label") == 0)
        {
            gf2d_element_load_label_from_config(e,json);
        }
        else if (strcmp(type,"actor") == 0)
        {
            gf2d_element_load_actor_from_config(e,json);
        }
        else if (strcmp(type,"button") == 0)
        {
            gf2d_element_load_button_from_config(e,json,win);
        }
        else if (strcmp(type,"entry") == 0)
        {
            gf2d_element_load_entry_from_config(e,json,win);
        }
        else if (strcmp(type,"scrollbar") == 0)
        {
            gf2d_element_scrollbar_load_from_config(e,json,win);
        }
    }
    else
    {
        slog("element definition missing type!");
    }
    return e;
}

void gf2d_element_set_color(Element *element,GFC_Color color)
{
    if (!element)return;
    element->color = color;
}

void gf2d_element_set_background_color(Element *element,GFC_Color color)
{
    if (!element)return;
    element->backgroundColor = color;
}

int gf2d_element_set_focus(Element *element,int focus)
{
    if (!element->canHasFocus)return 0;
    element->hasFocus = focus;
    return 1;
}

GFC_Rect gf2d_element_get_absolute_bounds(Element *element,GFC_Vector2D offset)
{
    GFC_Rect r = {0};
    if (!element)return r;
    r.x = element->drawBounds.x + offset.x;
    r.y = element->drawBounds.y + offset.y;
    r.w = element->drawBounds.w;
    r.h = element->drawBounds.h;
    return r;
}

Element *gf2d_element_get_by_id(Element *e,int id)
{
    if (!e)return NULL;
    if (e->index == id)return e;
    switch(e->type)
    {
        case ET_GFC_List:
            return gf2d_element_list_get_item_by_id(e,id);
            break;
        case ET_Button:
            return gf2d_element_button_get_by_id(e,id);
            break;
        case ET_Entry:
        default:
            return NULL;
    }
}


Element *gf2d_get_element_by_name(Element *e,const char *name)
{
    if (!e)return NULL;
    if (gfc_line_cmp(e->name,name)==0)return e;
    if (e->get_by_name)return e->get_by_name(e,name);
    return NULL;
}
/*eol@eof*/
