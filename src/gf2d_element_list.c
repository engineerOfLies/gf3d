#include <stdlib.h>
#include <stdio.h>
#include "simple_logger.h"
#include "gf2d_element_list.h"

int gf2d_element_list_get_items_per_line(Element *element)
{
    GF2D_ListElement *list;
    if (!element)return 0;
    list = (GF2D_ListElement*)element->data;
    return list->itemsPerLine;
}

void gf2d_element_list_set_scroll_offset(Element *element,int offset)
{
    GF2D_ListElement *list;
    if (!element)return;
    list = (GF2D_ListElement*)element->data;
    list->scrollOffset = offset;
}

int gf2d_element_list_get_row_count(Element *element)
{
    GF2D_ListElement *list;
    if (!element)return 0;
    list = (GF2D_ListElement*)element->data;
    if (!list->itemSize.y)return 0;
    return (int)element->drawBounds.h / list->itemSize.y;
}

GFC_Vector2D gf2d_element_list_get_item_size(Element *element)
{
    GF2D_ListElement *list;
    if (!element)return gfc_vector2d(-1,-1);
    list = (GF2D_ListElement*)element->data;
    return list->itemSize;
}


int gf2d_element_list_get_items_per_column(Element *element)
{
    GF2D_ListElement *list;
    if (!element)return 0;
    list = (GF2D_ListElement*)element->data;
    return list->itemsPerColumn;
}

GFC_Vector2D gf2d_element_get_item_position(Element *element,int i,GFC_Vector2D lastPosition)
{
    GF2D_ListElement* list;
    Element *item;
    GFC_Vector2D itemSize;
    GFC_Vector2D position = {0};
    if (i == 0)return lastPosition;
    if (!element)return lastPosition;
    list = (GF2D_ListElement*)element->data;
    if (!list)return lastPosition;
    item = (Element *)gfc_list_get_nth(list->list,i - 1);
    if (!item)return position;
    if (list->packed)
    {
        itemSize.x = item->drawBounds.w;
        itemSize.y = item->drawBounds.h;
    }
    else
    {
        gfc_vector2d_copy(itemSize,list->itemSize);
    }
    if ((list->listStyle == LS_Horizontal) && (list->wraps))
    {
        list->itemsPerLine = element->drawBounds.w / itemSize.x;
        if (((i % list->itemsPerLine) == 0)&&(i != 0))
        {
            // this is a new line
            position.x = element->drawBounds.x;
            position.y = lastPosition.y + itemSize.y;
        }
        else
        {
            position.x = lastPosition.x + itemSize.x;
            position.y = lastPosition.y;
        }
        return position;
    }
    if ((list->listStyle == LS_Vertical) && (list->wraps))
    {
        list->itemsPerColumn = element->drawBounds.h / itemSize.y;
        if (((i % list->itemsPerColumn) == 0)&&(i != 0))
        {
            position.x = lastPosition.x + itemSize.x;
            position.y = element->drawBounds.y;
        }
        else
        {
            position.x = lastPosition.x;
            position.y = lastPosition.y + itemSize.y;
        }
        return position;
    }
    if (list->listStyle == LS_Horizontal)
    {
        position.x = lastPosition.x + itemSize.x;
        position.y = lastPosition.y;
        return position;
    }
    if (list->listStyle == LS_Vertical)
    {
        position.x = lastPosition.x;
        position.y = lastPosition.y + itemSize.y;
        return position;
    }
    return position;
}

void gf2d_element_list_draw(Element *element,GFC_Vector2D offset)
{
    int skip;
    GF2D_ListElement *list;
    GFC_Vector2D position = {0};
    GFC_Vector2D drawPosition = {0};
    int count,i;
    Element *e;
    if (!element)return;
    list = (GF2D_ListElement*)element->data;
    if (!list)return;
    position.x = element->drawBounds.x;
    position.y = element->drawBounds.y;
    if (list->scrolls)
    {
        if ((list->listStyle == LS_Horizontal) && (list->wraps))
        {
            position.y -= list->scrollOffset * list->itemSize.y;
        }
        else if ((list->listStyle == LS_Vertical) && (list->wraps))
        {
            position.x -= list->scrollOffset * list->itemSize.x;
        }
        else if (list->listStyle == LS_Horizontal)
        {
            position.x -= list->scrollOffset * list->itemSize.x;
        }
        else if (list->listStyle == LS_Vertical)
        {
            position.y -= list->scrollOffset * list->itemSize.y;
        }
    }
    count = gfc_list_get_count(list->list);
    for (i = 0; i < count; i++)
    {
        e = (Element *)gfc_list_get_nth(list->list,i);
        if (!e)continue;
        skip = 0;
        position = gf2d_element_get_item_position(element,i,position);
        if (list->cropped)
        {
            if ((position.x < element->drawBounds.x)||(position.y < element->drawBounds.y))
            {
                skip = 1;
            }
            if ((position.x + e->drawBounds.w > element->drawBounds.x + element->drawBounds.w)||(position.y + e->drawBounds.h > element->drawBounds.y + element->drawBounds.h))
            {
                skip = 1;//skip outside of range
            }
        }
        gfc_vector2d_add(drawPosition,position,offset);
        if (skip)continue;
        gf2d_element_draw(e, drawPosition);
    }
    gf2d_element_draw(list->scrollbar, offset);
}

int gf2d_element_list_get_item_count(Element *element)
{
    GF2D_ListElement *list;
    if (!element)return 0;
    list = (GF2D_ListElement*)element->data;
    if (!list)return 0;
    return gfc_list_get_count(list->list);
}

Element *gf2d_list_get_next(Element *element,Element *from)
{
    GF2D_ListElement *list;
    int i,c;
    Element *e,*sub;
    if (!element)return NULL;
    list = (GF2D_ListElement*)element->data;
    if (!list)return NULL;
    c = gfc_list_get_count(list->list);
    if (element == from)
    {
        e = gfc_list_get_nth(list->list,0);
        if (e == NULL)
        {
            return from;//its me, but I have no children
        }
        return e;
    }
    for (i = 0; i < c; i++)
    {
        e = gfc_list_get_nth(list->list,i);
        if (!e)continue;
        sub = e->get_next(e,from);
        if (sub == NULL)continue;//nothing found that matches
        if (sub == from)
        {
            e = gfc_list_get_nth(list->list,i+1);
            if (e)
            {
                return e;
            }
            return from;
        }
        return sub;
    }
    return NULL;
}

void gf2d_element_list_recalibrate(Element *e)
{
    int c,i;
    Element *item;
    GF2D_ListElement *list;
    if (!e)return;
    list = (GF2D_ListElement*)e->data;
    if (!list)return;
    c = gfc_list_count(list->list);
    for (i = 0; i < c; i++)
    {
        item = gfc_list_nth(list->list,i);
        if (!item)continue;
        gf2d_element_recalibrate(item);
    }
}


Element *list_get_by_name(Element *element,const char *name)
{
    GF2D_ListElement *list;
    int count,i;
    Element *e,*r;
    if (!element)return NULL;
    list = (GF2D_ListElement*)element->data;
    if (!list)return NULL;
    count = gfc_list_get_count(list->list);
    for (i = 0; i < count; i++)
    {
        e = (Element *)gfc_list_get_nth(list->list,i);
        if (!e)continue;
        r = gf2d_get_element_by_name(e,name);
        if (r)return r;
    }
    return NULL;
}

GFC_List *gf2d_element_list_update(Element *element,GFC_Vector2D offset)
{
    GF2D_ListElement *list;
    GFC_Vector2D position,drawPosition;
    int count,i;
    Element *e;
    GFC_List *ret = NULL;
    GFC_List *updated;
    if (!element)return NULL;
    list = (GF2D_ListElement*)element->data;
    if (!list)return NULL;
    count = gfc_list_get_count(list->list);
    position.x = 0;
    position.y = 0;
    for (i = 0; i < count; i++)
    {
        e = (Element *)gfc_list_get_nth(list->list,i);
        if (!e)continue;
        position = gf2d_element_get_item_position(element,i,position);
        gfc_vector2d_add(drawPosition,position,offset);
        updated = gf2d_element_update(e, drawPosition);
        if (updated != NULL)
        {
            if (ret == NULL)
            {
                ret = gfc_list_new();
            }
            gfc_list_concat_free(ret,updated);
        }
    }
    gf2d_element_update(list->scrollbar, position);
    return ret;
}

void gf2d_element_list_free_items(Element *element)
{
    int count,i;
    Element *e;
    GF2D_ListElement *list;
    if (!element)return;
    list = (GF2D_ListElement*)element->data;
    if ((list != NULL)&&(list->list != NULL))
    {
        /*for each item, free it*/
        count = gfc_list_get_count(list->list);
        for (i = 0; i < count; i++)
        {
            e = (Element *)gfc_list_get_nth(list->list,i);
            if (!e)continue;
            gf2d_element_free(e);
        }
        gfc_list_delete(list->list);
        list->list = gfc_list_new();
    }
}

void gf2d_element_list_free(Element *element)
{
    int count,i;
    Element *e;
    GF2D_ListElement *list;
    if (!element)return;
    list = (GF2D_ListElement*)element->data;
    if (list != NULL)
    {
        /*for each item, free it*/
        count = gfc_list_get_count(list->list);
        for (i = 0; i < count; i++)
        {
            e = (Element *)gfc_list_get_nth(list->list,i);
            if (!e)continue;
            gf2d_element_free(e);
        }
        gfc_list_delete(list->list);
        free(list);
    }
}

GF2D_ListElement *gf2d_element_list_new()
{
    GF2D_ListElement *list;
    list = (GF2D_ListElement *)malloc(sizeof(GF2D_ListElement));
    if (!list)
    {
        slog("failed to allocate memory for list");
        return NULL;
    }
    memset(list,0,sizeof(GF2D_ListElement));
    list->list = gfc_list_new();
    return list;
}


GF2D_ListElement *gf2d_element_list_new_full(GFC_Rect bounds,GFC_Vector2D itemSize,GFC_ListStyle ls,int wraps,int scrolls,int packed,int cropped)
{
    GF2D_ListElement *list;
    list = gf2d_element_list_new();
    if (!list)
    {
        return NULL;
    }
    if (itemSize.x <= 1)itemSize.x *= bounds.w;
    if (itemSize.y <= 1)itemSize.y *= bounds.h;
    gfc_vector2d_copy(list->itemSize,itemSize);
    list->listStyle = ls;
    list->wraps = wraps;
    list->scrolls = scrolls;
    list->packed = packed;
    list->cropped = cropped;
    return list;
}

void gf2d_element_make_list(Element *e,GF2D_ListElement *list)
{
    if ((!e)||(!list))return;// no op
    e->data = list;
    e->type = ET_GFC_List;
    e->draw = gf2d_element_list_draw;
    e->update = gf2d_element_list_update;
    e->free_data = gf2d_element_list_free;
    e->get_by_name = list_get_by_name;
    e->get_next = gf2d_list_get_next;
    e->recalibrate = gf2d_element_list_recalibrate;
}

void gf2d_element_list_remove_item(Element *e,Element *item)
{
    GF2D_ListElement *list;
    if ((!e)||(!item))return;// no op
    list = (GF2D_ListElement *)e->data;
    gfc_list_delete_data(list->list,(void*)item);
}

void gf2d_element_list_add_item(Element *e,Element *item)
{
    GF2D_ListElement *list;
    if ((!e)||(!item))return;// no op
    list = (GF2D_ListElement *)e->data;
    item->parent = e;
    if (item->bounds.w == 1)item->bounds.w = list->itemSize.x;
    if (item->bounds.h == 1)item->bounds.h = list->itemSize.y;
    gf2d_element_recalibrate(item);
    gfc_list_append(list->list,(void*)item);
}

Element *gf2d_element_list_get_item_by_id(Element *e,int id)
{
    GF2D_ListElement *list;
    Element *item, *q;
    int count,i;
    if (!e)return NULL;
    if (e->type != ET_GFC_List)return NULL;
    list = (GF2D_ListElement *)e->data;
    if (!list)return NULL;
    count = gfc_list_get_count(list->list);
    for (i = 0; i < count; i++)
    {
        item = (Element *)gfc_list_get_nth(list->list,i);
        if (!item)continue;
        q = gf2d_element_get_by_id(item,id);
        if (q)return q;
    }
    return NULL;
}

Element *gf2d_element_list_new_complete(
    Element *parent,
    Window *win,
    int      index,
    GFC_TextLine name,
    GFC_Rect bounds,
    GFC_Color color,
    GFC_Color backgroundGFC_Color,
    int backgroundDraw,
    GFC_Vector2D itemSize,
    GFC_ListStyle ls,
    int wraps,
    int scrolls,
    int packed,
    int cropped)
{
    Element *list;
    GF2D_ListElement *le;
    le = gf2d_element_list_new_full(
        gfc_rect(0,0,1,1),
        itemSize,
        ls,
        wraps,scrolls,packed,cropped);
    if (!le)return NULL;
    list = gf2d_element_new_full(
        parent,index,name,
        bounds,
        color,0,
        backgroundGFC_Color,backgroundDraw,win);
    gf2d_element_make_list(list,le);
    return list;
}


void gf2d_element_load_list_from_config(Element *e,SJson *json,Window *win)
{
    SJson *value = NULL;
    SJson *item = NULL;
    GF2D_ListElement *list = NULL;
    GFC_Vector2D gfc_vector = {0};
    GFC_ListStyle ls = 0;
    int i,count;
    const char *style = NULL;
    short int wraps = 0,scrolls = 0;
    short int packed = 0,cropped = 0;
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
        
    style = sj_get_string_value(sj_object_get_value(json,"style"));
    if (style)
    {
        if (strcmp(style,"horizontal") == 0)
        {
            ls = LS_Horizontal;
        }
        if (strcmp(style,"vertical") == 0)
        {
            ls = LS_Vertical;
        }
    }
    
    sj_get_bool_value(sj_object_get_value(json,"cropped"),&cropped);
    sj_get_bool_value(sj_object_get_value(json,"wraps"),&wraps);
    sj_get_bool_value(sj_object_get_value(json,"packed"),&packed);
    sj_get_bool_value(sj_object_get_value(json,"scrolls"),&scrolls);
    sj_value_as_vector2d(sj_object_get_value(json,"item_size"),&gfc_vector);
    
    list = gf2d_element_list_new_full(e->bounds,gfc_vector,ls,wraps,scrolls,packed,cropped);
    gf2d_element_make_list(e,list);
    
    value = sj_object_get_value(json,"elements");
    count = sj_array_get_count(value);

    for (i = 0; i < count; i++)
    {
        item = sj_array_get_nth(value,i);
        if (!item)continue;
        gf2d_element_list_add_item(e,gf2d_element_load_from_config(item,e,win));
    }
    value = sj_object_get_value(json,"scrollbar");
    if (value)
    {
        list->scrollbar = gf2d_element_load_from_config(value,e,win);
    }

}
/*eol@eof*/
