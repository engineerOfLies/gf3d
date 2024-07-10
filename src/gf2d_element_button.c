#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_font.h"
#include "gf2d_element_button.h"
#include "gf2d_element_actor.h"
#include "gf2d_element_label.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"

void gf2d_element_button_draw(Element *element,GFC_Vector2D offset)
{
    GFC_Rect rect;
    ButtonElement *button;
    GFC_Vector2D position;
    if (!element)return;
    button = (ButtonElement*)element->data;
    if (!button)return;
    gfc_vector2d_add(position,offset,element->drawBounds);
    gfc_rect_set(rect,offset.x + element->drawBounds.x,offset.y + element->drawBounds.y,element->drawBounds.w,element->drawBounds.h);
    if (button->customActions == BCA_Default)
    {
        switch(element->state)
        {
            case ES_disable:
                return;
            case ES_idle:
                gf2d_element_actor_set_action(button->actor,"idle");
                gf2d_element_set_color(button->label,element->color);
                break;
            case ES_highlight:
                gf2d_element_actor_set_action(button->actor,"high");
                gf2d_element_set_color(button->label,button->highColor);
                break;
            case ES_active:
                gf2d_element_actor_set_action(button->actor,"press");
                gf2d_element_set_color(button->label,button->pressColor);
                break;
        }
    }
    else if (button->customActions == BCA_BackgroundHighlight)
    {
        switch(element->state)
        {
            case ES_disable:
                return;
            case ES_idle:
                gf2d_draw_rect_filled(rect,element->backgroundColor);
                break;
            case ES_highlight:
                gf2d_draw_rect_filled(rect,button->highColor);
                break;
            case ES_active:
                gf2d_draw_rect_filled(rect,button->pressColor);
                break;
        }

    }
    gf2d_element_draw(button->actor,position);
    gf2d_element_draw(button->label,position);
}

GFC_List *gf2d_element_button_update(Element *element,GFC_Vector2D offset)
{
    GFC_Rect bounds;
    GFC_List *list;
    ButtonElement *button;
    if (!element)return NULL;
    button = (ButtonElement*)element->data;
    if (!button)return NULL;
    bounds = gf2d_element_get_absolute_bounds(element,offset);
    bounds.x = element->lastDrawPosition.x;
    bounds.y = element->lastDrawPosition.y;
    gf2d_element_update(button->actor, offset);
    
    if (element->hasFocus)
    {
        element->state = ES_highlight;
        if ((gfc_input_command_pressed("enter"))||(button->repeat && gfc_input_command_held("enter")))
        {
            element->state = ES_active;
            list = gfc_list_new();
            gfc_list_append(list,element);
            if (strlen(button->sound))gf2d_windows_play_sound(button->sound);
            return list;
        }
    }
    else
    {
        element->state = ES_idle;
    }

    if (gf2d_mouse_hidden() <= 0)
    {
        if(gf2d_mouse_in_rect(bounds))
        {
            element->state = ES_highlight;
            if (gf2d_mouse_button_state(0))
            {
                element->state = ES_active;
                if (button->repeat && gf2d_mouse_button_held(0))
                {
                    list = gfc_list_new();
                    gfc_list_append(list,element);
                    if (strlen(button->sound))gf2d_windows_play_sound(button->sound);
                    return list;
                }
            }
            else if (gf2d_mouse_button_released(0))
            {
                list = gfc_list_new();
                gfc_list_append(list,element);
                if (strlen(button->sound))gf2d_windows_play_sound(button->sound);
                return list;
            }
        }
        else
        {
            element->state = ES_idle;
        }
    }
    if ((gfc_input_command_pressed(button->hotkey))||(button->repeat && gfc_input_command_held(button->hotkey)))
    {
        element->state = ES_active;
        list = gfc_list_new();
        gfc_list_append(list,element);
        if (strlen(button->sound))gf2d_windows_play_sound(button->sound);
        return list;
    }
    return NULL;
}

const char *gf2d_element_button_get_input(Element *e)
{
    ButtonElement *button;
    if (!e)return NULL;
    button = (ButtonElement*)e->data;
    if (!button)return NULL;
    return button->hotkey;
}

Element *gf2d_element_button_get_by_id(Element *e,int id)
{
    ButtonElement *button;
    Element *r;
    if (!e)return NULL;
    button = (ButtonElement*)e->data;
    r = gf2d_element_get_by_id(button->label,id);
    if (r)return r;
    return gf2d_element_get_by_id(button->actor,id);
}

Element *button_get_by_name(Element *e,const char *name)
{
    ButtonElement *button;
    Element *r;
    if (!e)return NULL;
    button = (ButtonElement*)e->data;
    r = gf2d_get_element_by_name(button->label,name);
    if (r)return r;
    return gf2d_get_element_by_name(button->actor,name);
}

Element *gf2d_button_get_next(Element *element, Element *from)
{
    ButtonElement *button;
    if (!element)return NULL;
    button = (ButtonElement*)element->data;
    if (element == from)
    {
        if (button->label)return button->label;
        if (button->actor)return button->actor;
        return from;
    }
    if (from == button->label)
    {
        return button->actor;
        return from;
    }
    if (from == button->actor)return from;//search item was my last child, return me
    return NULL;
}


void gf2d_element_button_recalibrate(Element *e)
{
    ButtonElement *button;
    if (!e)return;
    button = (ButtonElement*)e->data;
    if (!button)return;
    gf2d_element_recalibrate(button->label);
    gf2d_element_recalibrate(button->actor);
}

void gf2d_element_button_free(Element *element)
{
    ButtonElement *button;
    if (!element)return;
    button = (ButtonElement*)element->data;
    if (button != NULL)
    {
        gf2d_element_free(button->label);
        gf2d_element_free(button->actor);
        free(button);
    }
}

ButtonElement *gf2d_element_button_new()
{
    ButtonElement *button;
    button = (ButtonElement *)malloc(sizeof(ButtonElement));
    if (!button)
    {
        slog("failed to allocate memory for button");
        return NULL;
    }
    memset(button,0,sizeof(ButtonElement));
    return button;
}


void gf2d_element_make_button(Element *e,ButtonElement *button)
{
    if (!e)return;
    e->data = button;
    e->type = ET_Button;
    e->draw = gf2d_element_button_draw;
    e->update = gf2d_element_button_update;
    e->free_data = gf2d_element_button_free;
    e->recalibrate = gf2d_element_button_recalibrate;
    e->get_by_name = button_get_by_name;
    e->get_next = gf2d_button_get_next;
}

ButtonElement *gf2d_element_button_new_full(Element *label,Element *actor,GFC_Color highColor,GFC_Color pressColor,int customActions)
{
    ButtonElement *button;
    button = gf2d_element_button_new();
    if (!button)return NULL;
    button->label = label;
    button->actor = actor;
    button->highColor = highColor;
    button->pressColor = pressColor;
    button->customActions = customActions;
    return button;
}

void gf2d_element_load_button_from_config(Element *e,SJson *json,Window *win)
{
    GFC_Vector4D highColor = {255,255,255,255},pressColor = {255,255,255,255};
    Element *label = NULL;
    Element *actor = NULL;
    SJson *value;
    const char *text;
    int customActions = 0;
    ButtonElement *button;
    
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
    
    value = sj_object_get_value(json,"highColor");
    if (value)
    {
        sj_value_as_vector4d(value,&highColor);
    }
    value = sj_object_get_value(json,"pressColor");
    if (value)
    {
        sj_value_as_vector4d(value,&pressColor);
    }
    
    sj_get_integer_value(sj_object_get_value(json,"customActions"),&customActions);

    value = sj_object_get_value(json,"label");
    if (value)
    {
        label = gf2d_element_load_from_config(value,e,win);
    }
    value = sj_object_get_value(json,"actor");
    if (value)
    {
        actor = gf2d_element_load_from_config(value,e,win);
    }
    gf2d_element_make_button(e,gf2d_element_button_new_full(label,actor,gfc_color_from_vector4(highColor),gfc_color_from_vector4(pressColor),customActions));
 
    button = (ButtonElement*)e->data;
    text = sj_get_string_value(sj_object_get_value(json,"sound"));
    if (text)gfc_line_cpy(button->sound,text);
    sj_object_get_value_as_bool(json,"repeat",&button->repeat);
    value = sj_object_get_value(json,"hotkey");
    if (value)
    {
        gfc_line_cpy(button->hotkey,sj_get_string_value(value));
    }
}


Element *gf2d_button_new_label_simple(Window *win,int index,const char *text,FontTypes ft, GFC_Vector2D size, GFC_Color color)
{
    Element *be,*le;
    
    LabelElement *label;

    if (!text)return NULL;
    
    label = gf2d_element_label_new_full(text,color,ft,LJ_Left,LA_Middle,0);

    be = gf2d_element_new_full(
        NULL,
        index,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        color,
        0,
        gfc_color(.5,.5,.5,1),0,win);
    le = gf2d_element_new_full(
        be,
        0,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        gfc_color(1,1,1,1),
        0,
        gfc_color(1,1,1,1),0,win);
    
    gf2d_element_make_label(le,label);
    gf2d_element_make_button(be,gf2d_element_button_new_full(le,NULL,GFC_COLOR_WHITE,GFC_COLOR_GREY,0));
    return be;
}

Element *gf2d_button_new_simple(
    Window *win,
    int index,
    const char *name,
    const char *actorFile,
    const char *text,
    GFC_Vector2D scale,
    GFC_Vector2D size,
    GFC_Color color)
{
    Element *be,*le,*ae;
    
    LabelElement *label;
    ActorElement *actor;
    if (!text)return NULL;
    
    label = gf2d_element_label_new_full(text,color,FT_Small,LJ_Center,LA_Middle,0);

    be = gf2d_element_new_full(
        NULL,
        index,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        color,
        0,
        gfc_color(.5,.5,.5,1),0,win);
    
    actor = gf2d_element_actor_new_full(actorFile, "idle" ,scale,NULL,gfc_vector2d(0,0),gfc_vector2d(0,0),0);
    ae = gf2d_element_new_full(
        be,
        0,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        gfc_color(1,1,1,1),
        0,
        gfc_color(1,1,1,1),0,win);

    le = gf2d_element_new_full(
        be,
        0,
        (char *)text,
        gfc_rect(0,0,size.x,size.y),
        gfc_color(1,1,1,1),
        0,
        gfc_color(1,1,1,1),0,win);
    
    gf2d_element_make_label(le,label);
    gf2d_element_make_actor(ae,actor);
    gf2d_element_make_button(be,gf2d_element_button_new_full(le,ae,gfc_color(1,1,1,1),gfc_color(0.9,0.9,0.9,1),0));
    return be;
}


/*eol@eof*/
