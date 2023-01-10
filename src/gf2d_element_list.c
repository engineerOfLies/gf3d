#include <stdlib.h>
#include <stdio.h>
#include "simple_logger.h"
#include "gf2d_element_list.h"

int gf2d_element_list_get_items_per_line(Element *element)
{
    ListElement *list;
    if (!element)return 0;
    list = (ListElement*)element->data;
    return list->itemsPerLine;
}

void gf2d_element_list_set_scroll_offset(Element *element,int offset)
{
    ListElement *list;
    if (!element)return;
    list = (ListElement*)element->data;
    list->scrollOffset = offset;
}

int gf2d_element_list_get_row_count(Element *element)
{
    ListElement *list;
    if (!element)return 0;
    list = (ListElement*)element->data;
    if (!list->itemSize.y)return 0;
    return (int)element->bounds.h / list->itemSize.y;
}

int gf2d_element_list_get_items_per_column(Element *element)
{
    ListElement *list;
    if (!element)return 0;
    list = (ListElement*)element->data;
    return list->itemsPerColumn;
}

Vector2D gf2d_element_get_item_position(Element *element,int i,Vector2D lastPosition)
{
    ListElement* list;
    Element *item;
    Vector2D itemSize;
    Vector2D position = {0};
    if (i == 0)return lastPosition;
    if (!element)return lastPosition;
    list = (ListElement*)element->data;
    if (!list)return lastPosition;
    item = (Element *)gfc_list_get_nth(list->list,i - 1);
    if (!item)return position;
    if (list->packed)
    {
        itemSize.x = item->bounds.w;
        itemSize.y = item->bounds.h;
    }
    else
    {
        vector2d_copy(itemSize,list->itemSize);
    }
    if ((list->listStyle == LS_Horizontal) && (list->wraps))
    {
        list->itemsPerLine = element->bounds.w / itemSize.x;
        if (((i % list->itemsPerLine) == 0)&&(i != 0))
        {
            // this is a new line
            position.x = element->bounds.x;
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
        list->itemsPerColumn = element->bounds.h / itemSize.y;
        if (((i % list->itemsPerColumn) == 0)&&(i != 0))
        {
            position.x = lastPosition.x + itemSize.x;
            position.y = element->bounds.y;
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

void gf2d_element_list_draw(Element *element,Vector2D offset)
{
    int skip;
    ListElement *list;
    Vector2D position = {0};
    Vector2D drawPosition = {0};
    int count,i;
    Element *e;
    if (!element)return;
    list = (ListElement*)element->data;
    if (!list)return;
    position.x = element->bounds.x;
    position.y = element->bounds.y;
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
            if ((position.x < element->bounds.x)||(position.y < element->bounds.y))
            {
                skip = 1;
            }
            if ((position.x + e->bounds.w > element->bounds.x + element->bounds.w)||(position.y + e->bounds.h > element->bounds.y + element->bounds.h))
            {
                skip = 1;//skip outside of range
            }
        }
        vector2d_add(drawPosition,position,offset);
        if (skip)continue;
        gf2d_element_draw(e, drawPosition);
    }
    gf2d_element_draw(list->scrollbar, offset);
}

int gf2d_element_list_get_item_count(Element *element)
{
    ListElement *list;
    if (!element)return 0;
    list = (ListElement*)element->data;
    if (!list)return 0;
    return gfc_list_get_count(list->list);
}

Element *gf2d_list_get_next(Element *element,Element *from)
{
    ListElement *list;
    int i,c;
    Element *e,*sub;
    if (!element)return NULL;
    list = (ListElement*)element->data;
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

Element *list_get_by_name(Element *element,const char *name)
{
    ListElement *list;
    int count,i;
    Element *e,*r;
    if (!element)return NULL;
    list = (ListElement*)element->data;
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

List *gf2d_element_list_update(Element *element,Vector2D offset)
{
    ListElement *list;
    Vector2D position,drawPosition;
    int count,i;
    Element *e;
    List *ret = NULL;
    List *updated;
    if (!element)return NULL;
    list = (ListElement*)element->data;
    if (!list)return NULL;
    count = gfc_list_get_count(list->list);
    position.x = 0;
    position.y = 0;
    for (i = 0; i < count; i++)
    {
        e = (Element *)gfc_list_get_nth(list->list,i);
        if (!e)continue;
        position = gf2d_element_get_item_position(element,i,position);
        vector2d_add(drawPosition,position,offset);
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
    ListElement *list;
    if (!element)return;
    list = (ListElement*)element->data;
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
    ListElement *list;
    if (!element)return;
    list = (ListElement*)element->data;
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

ListElement *gf2d_element_list_new()
{
    ListElement *list;
    list = (ListElement *)malloc(sizeof(ListElement));
    if (!list)
    {
        slog("failed to allocate memory for list");
        return NULL;
    }
    memset(list,0,sizeof(ListElement));
    list->list = gfc_list_new();
    return list;
}


ListElement *gf2d_element_list_new_full(Rect bounds,Vector2D itemSize,ListStyle ls,int wraps,int scrolls,int packed,int cropped)
{
    ListElement *list;
    list = gf2d_element_list_new();
    if (!list)
    {
        return NULL;
    }
    if (itemSize.x <= 1)itemSize.x *= bounds.w;
    if (itemSize.y <= 1)itemSize.y *= bounds.h;
    vector2d_copy(list->itemSize,itemSize);
    list->listStyle = ls;
    list->wraps = wraps;
    list->scrolls = scrolls;
    list->packed = packed;
    list->cropped = cropped;
    return list;
}

void gf2d_element_make_list(Element *e,ListElement *list)
{
    if ((!e)||(!list))return;// no op
    e->data = list;
    e->type = ET_List;
    e->draw = gf2d_element_list_draw;
    e->update = gf2d_element_list_update;
    e->free_data = gf2d_element_list_free;
    e->get_by_name = list_get_by_name;
    e->get_next = gf2d_list_get_next;
}

void gf2d_element_list_remove_item(Element *e,Element *item)
{
    ListElement *list;
    if ((!e)||(!item))return;// no op
    list = (ListElement *)e->data;
    gfc_list_delete_data(list->list,(void*)item);
}

void gf2d_element_list_add_item(Element *e,Element *item)
{
    ListElement *list;
    if ((!e)||(!item))return;// no op
    list = (ListElement *)e->data;
    item->parent = e;
    if (item->bounds.w == 1)item->bounds.w = list->itemSize.x;
    if (item->bounds.h == 1)item->bounds.h = list->itemSize.y;
    list->list = gfc_list_append(list->list,(void*)item);
}

Element *gf2d_element_list_get_item_by_id(Element *e,int id)
{
    ListElement *list;
    Element *item, *q;
    int count,i;
    if (!e)return NULL;
    if (e->type != ET_List)return NULL;
    list = (ListElement *)e->data;
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

void gf2d_element_load_list_from_config(Element *e,SJson *json,Window *win)
{
    SJson *value = NULL;
    SJson *item = NULL;
    ListElement *list = NULL;
    Vector2D vector = {0};
    ListStyle ls = 0;
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
    sj_value_as_vector2d(sj_object_get_value(json,"item_size"),&vector);
    
    list = gf2d_element_list_new_full(e->bounds,vector,ls,wraps,scrolls,packed,cropped);
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
