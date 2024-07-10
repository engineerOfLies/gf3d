#include <stdlib.h>
#include <stdio.h>
#include "simple_logger.h"
#include "gfc_text.h"
#include "gf2d_font.h"
#include "gf2d_element_label.h"

void gf2d_element_label_draw(Element *element,GFC_Vector2D offset)
{
    LabelElement *label;
    GFC_Vector2D position;
    GFC_Vector2D size = {0};
    GFC_Rect r;
    if (!element)return;
    label = (LabelElement*)element->data;
    if (!label)return;
    if (strlen(label->text) <= 0)return;
    if (label->wraps)
    {
        r = gf2d_font_get_text_wrap_bounds_tag(
            label->text,
            label->style,
            element->drawBounds.w,
            element->drawBounds.h);
        size.x = r.w;
        size.y = r.h;
    }
    else
    {
        size = gf2d_font_get_bounds_tag(label->text,label->style);
    }
    if (size.x < 0)
    {
        return;
    }
    // adjust position to top left
    gfc_vector2d_add(position,offset,element->drawBounds);
    switch(label->justify)
    {
        case LJ_Left:
            break;
        case LJ_Center:
            position.x += (element->drawBounds.w - size.x)/2 ;
            break;
        case LJ_Right:
            position.x += (element->drawBounds.w - size.x);
            break;
    }
    switch(label->alignment)
    {
        case LA_Top:
            break;
        case LA_Middle:
            position.y += (element->drawBounds.h - size.y)/2 ;
            break;
        case LA_Bottom:
            position.y += (element->drawBounds.h - size.y);
            break;
    }
    if (label->wraps)
    {
        gf2d_font_draw_text_wrap_tag(label->text,label->style,element->color, gfc_rect(position.x, position.y, element->drawBounds.w, element->drawBounds.h));
    }
    else 
    {
        gf2d_font_draw_line_tag(label->text,label->style,element->color, position);
    }
}

GFC_List *gf2d_element_label_update(Element *element,GFC_Vector2D offset)
{
    return NULL;
}

void gf2d_element_label_free(Element *element)
{
    LabelElement *label;
    if (!element)return;
    label = (LabelElement*)element->data;
    if (label != NULL)
    {
        free(label);
    }
}
//returns itself if it was found.  Caller should then check if from == this return
Element *gf2d_label_get_next(Element *element, Element *from)
{
    if (element == from)
    {
        return from;
    }
    return NULL;
}

LabelElement *gf2d_element_label_new()
{
    LabelElement *label;
    label = (LabelElement *)malloc(sizeof(LabelElement));
    if (!label)
    {
        slog("failed to allocate memory for label");
        return NULL;
    }
    memset(label,0,sizeof(LabelElement));
    return label;
}


LabelElement *gf2d_element_label_new_full(const char *text,GFC_Color color,int style,int justify,int align,int wraps)
{
    LabelElement *label;
    label = gf2d_element_label_new();
    if (!label)
    {
        return NULL;
    }
    if (text)
    {
        gfc_block_cpy(label->text,text);
    }
    label->bgcolor = color;
    label->style = style;
    label->justify = justify;
    label->alignment = align;
    label->wraps = wraps;
    return label;
}

Element *gf2d_label_new_simple_size(Window *win,int index,const char *text,int style,GFC_Vector2D size, GFC_Color color)
{
    Element *le;
    LabelElement *label;

    if (!text)return NULL;
    
    label = gf2d_element_label_new_full(text,color,style,LJ_Left,LA_Middle,0);
    le = gf2d_element_new_full(
        NULL,
        0,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        color,
        0,
        gfc_color(1,1,1,1),0,win);
    gf2d_element_make_label(le,label);
    return le;
}

void gf2d_element_label_set_wrap(Element *e,Uint8 wraps)
{
    LabelElement *label;
    if (!e)return;
    if (e->type != ET_Label)return;
    label = (LabelElement *)e->data;
    if (!label)return;
    label->wraps = wraps;
}

void gf2d_element_label_set_alignment(Element *e,Uint8 justify,Uint8 align)
{
    LabelElement *label;
    if (!e)return;
    if (e->type != ET_Label)return;
    label = (LabelElement *)e->data;
    if (!label)return;
    label->justify = justify;
    label->alignment = align;
}


Element *gf2d_label_new_simple(Window *win,int index,const char *text,int style,GFC_Color color)
{
    Element *le;
    LabelElement *label;

    if (!text)return NULL;
    
    label = gf2d_element_label_new_full(text,color,style,LJ_Left,LA_Middle,0);
    le = gf2d_element_new_full(
        NULL,
        0,
        (char *)text,
        gfc_rect(0,0,1,30),
        color,
        0,
        gfc_color(1,1,1,1),0,win);
    gf2d_element_make_label(le,label);
    return le;
}

void gf2d_element_make_label(Element *e,LabelElement *label)
{
    if (!e)return;
    e->data = label;
    e->type = ET_Label;
    e->draw = gf2d_element_label_draw;
    e->update = gf2d_element_label_update;
    e->free_data = gf2d_element_label_free;
    e->get_next = gf2d_label_get_next;
}

const char *gf2d_element_label_get_text(Element *e)
{
    if (!e)return NULL;
    if (e->type != ET_Label)return NULL;
    LabelElement *label;
    label = (LabelElement *)e->data;
    if (!label)return NULL;
    return label->text;
}

void gf2d_element_label_set_font(Element *e,FontTypes font)
{
    LabelElement *label;
    if (!e)return;
    if (e->type != ET_Label)return;
    label = (LabelElement *)e->data;
    if (!label)return;
    label->style = font;
}

void gf2d_element_label_set_text(Element *e,const char *text)
{
    LabelElement *label;
    if (!e)return;
    if (e->type != ET_Label)return;
    label = (LabelElement *)e->data;
    if (!label)return;
    if ((!text)||(!strlen(text)))
    {
        gfc_block_clear(label->text);
        return;
    }
    gfc_block_cpy(label->text,text);
}

void gf2d_element_load_label_from_config(Element *e,SJson *json)
{
    SJson *value;
    GFC_Vector4D gfc_vector;
    GFC_Color color;
    const char *buffer;
    int style = FT_Normal;
    int justify = LJ_Left;  
    int align = LA_Top;
    int wraps = 0;
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
    value = sj_object_get_value(json,"style");
    buffer = sj_get_string_value(value);
    if (buffer)
    {
        style = gf2d_font_type_from_text(buffer);
    }

    value = sj_object_get_value(json,"wraps");
    if (value)
    {
        sj_get_bool_value(value,(short int *)&wraps);
    }
    value = sj_object_get_value(json,"justify");
    buffer = sj_get_string_value(value);
    if (buffer)
    {
        if (strcmp(buffer,"left") == 0)
        {
            justify = LJ_Left;
        }
        else if (strcmp(buffer,"center") == 0)
        {
            justify = LJ_Center;
        }
        else if (strcmp(buffer,"right") == 0)
        {
            justify = LJ_Right;
        }
    }

    value = sj_object_get_value(json,"align");
    buffer = sj_get_string_value(value);
    if (buffer)
    {
        if (strcmp(buffer,"top") == 0)
        {
            align = LA_Top;
        }
        else if (strcmp(buffer,"middle") == 0)
        {
            align = LA_Middle;
        }
        else if (strcmp(buffer,"bottom") == 0)
        {
            align = LA_Bottom;
        }
    }
    value = sj_object_get_value(json,"color");
    gfc_vector4d_set(gfc_vector,255,255,255,255);
    sj_value_as_vector4d(value,&gfc_vector);
    color = gfc_color_from_vector4(gfc_vector);

    value = sj_object_get_value(json,"text");
    buffer = sj_get_string_value(value);
    gf2d_element_make_label(e,gf2d_element_label_new_full((char *)buffer,color,style,justify,align,wraps));
}

/*eol@eof*/
