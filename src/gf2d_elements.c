#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"
#include "gfc_shape.h"

#include "gf2d_draw.h"
#include "gf2d_elements.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_button.h"
#include "gf2d_element_entry.h"
#include "gf2d_element_list.h"
#include "gf2d_element_label.h"

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
    TextLine name,
    Rect bounds,
    Color color,
    int state,
    Color backgroundColor,
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

Vector2D gf2d_element_get_draw_position(Element *e)
{
    if (!e)return vector2d(0,0);
    return e->lastDrawPosition;
}

void gf2d_element_set_hidden(Element *element, int hidden)
{
    if (!element)return;
    element->hidden = hidden;
}

void gf2d_element_draw(Element *e, Vector2D offset)
{
    Rect rect;
    if ((!e)||(e->hidden))
    {
        return;
    }
    gfc_rect_set(rect,offset.x + e->bounds.x,offset.y + e->bounds.y,e->bounds.w,e->bounds.h);
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

List * gf2d_element_update(Element *e, Vector2D offset)
{
    if ((!e)||(e->hidden))
    {
        return NULL;
    }
    if (e->draw)return e->update(e,offset);
    return NULL;
}

void gf2d_element_calibrate(Element *e,Element *parent, Window *win)
{
    Rect res;
    int negx = 0,negy = 0;
    if (!e)return;
    if (parent != NULL)
    {
        gfc_rect_copy(res,parent->bounds);
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
    if (e->bounds.x < 0)
    {
        negx = 1;
        e->bounds.x *= -1;
    }
    if (e->bounds.y < 0)
    {
        negy = 1;
        e->bounds.y *= -1;
    }
    if ((e->bounds.x > 0)&&(e->bounds.x < 1.0))
    {
        e->bounds.x *= res.w;
    }
    if ((e->bounds.y > 0)&&(e->bounds.y < 1.0))
    {
        e->bounds.y *= res.h;
    }
    if ((e->bounds.w > 0)&&(e->bounds.w <= 1.0))
    {
        e->bounds.w *= res.w;
    }
    if ((e->bounds.h > 0)&&(e->bounds.h <= 1.0))
    {
        e->bounds.h *= res.h;
    }
    
    if (negx)
    {
        e->bounds.x = res.h - e->bounds.x;
    }
    if (negy)
    {
        e->bounds.y = res.w - e->bounds.y;
    }
}

Element *gf2d_element_load_from_config(SJson *json,Element *parent,Window *win)
{
    Element *e = NULL;
    SJson *value;
    const char *type;
    Vector4D vector ={0,0,0,0};
    if (!sj_is_object(json))return NULL;
    e = gf2d_element_new();
    if (!e)return NULL;
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
        sj_value_as_vector4d(value,&vector);
        gfc_rect_set(e->bounds,vector.x,vector.y,vector.z,vector.w);
    }
    else if (parent)
    {
        gfc_rect_copy(e->bounds,parent->bounds);
    }
    gf2d_element_calibrate(e,parent, win);
    
    value = sj_object_get_value(json,"type");
    if (value)
    {
        type = sj_get_string_value(value);
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
    }
    else
    {
        slog("element definition missing type!");
    }
    return e;
}

void gf2d_element_set_color(Element *element,Color color)
{
    if (!element)return;
    element->color = color;
}

int gf2d_element_set_focus(Element *element,int focus)
{
    if (!element->canHasFocus)return 0;
    element->hasFocus = focus;
    return 1;
}

Rect gf2d_element_get_absolute_bounds(Element *element,Vector2D offset)
{
    Rect r = {0};
    if (!element)return r;
    r.x = element->bounds.x + offset.x;
    r.y = element->bounds.y + offset.y;
    r.w = element->bounds.w;
    r.h = element->bounds.h;
    return r;
}

Element *gf2d_element_get_by_id(Element *e,int id)
{
    if (!e)return NULL;
    if (e->index == id)return e;
    switch(e->type)
    {
        case ET_List:
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
