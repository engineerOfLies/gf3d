#include <stdlib.h>
#include <stdio.h>
#include "simple_logger.h"

#include "gf2d_element_actor.h"

void gf2d_element_actor_draw(Element *element,GFC_Vector2D offset)
{
    ActorElement *actor;
    GFC_Vector2D position;
    if (!element)return;
    actor = (ActorElement*)element->data;
    if (!actor)return;
    gfc_vector2d_add(position,offset,element->drawBounds);
    if (actor->image)
    {
        gf2d_sprite_draw_full(
            actor->image,
            position,
            actor->scale,
            actor->center,
            actor->rotation,
            actor->flip,
            element->color,
            gfc_vector4d(0,0,0,0),
            actor->frame);
    }
    else if (actor->actor)
    {
        if (actor->center.x)
        {
            position.x += element->drawBounds.w / 2;
        }
        if (actor->center.y)
        {
            position.y += element->drawBounds.h / 2;
        }
        gf2d_actor_draw(
            actor->actor,
            actor->frame,
            position,
            &actor->scale,
            &actor->center,
            &actor->rotation,
            &element->color,
            &actor->flip);
    }
}

GFC_List * gf2d_element_actor_update(Element *element,GFC_Vector2D offset)
{
    ActorElement *actor;
    if (!element)return NULL;
    actor = (ActorElement*)element->data;
    if (!actor)return NULL;
    gfc_action_next_frame(actor->action,&actor->frame);
    return NULL;
}

void gf2d_element_actor_free(Element *element)
{
    ActorElement *actor;
    if (!element)return;
    actor = (ActorElement*)element->data;
    if (actor != NULL)
    {
        gf2d_actor_free(actor->actor);
        free(actor);
    }
}

ActorElement *gf2d_element_actor_new()
{
    ActorElement *actor;
    actor = (ActorElement *)malloc(sizeof(ActorElement));
    if (!actor)
    {
        slog("failed to allocate memory for actor");
        return NULL;
    }
    memset(actor,0,sizeof(ActorElement));
    return actor;
}


ActorElement *gf2d_element_actor_new_full(const char *actorFile, const char *action,GFC_Vector2D scale,const char *image,GFC_Vector2D center,GFC_Vector2D flip,float rotation)
{
    ActorElement *ae;
    ae = gf2d_element_actor_new();
    if (!ae)
    {
        return NULL;
    }
    ae->rotation = rotation;
    gfc_vector2d_copy(ae->scale,scale);
    gfc_vector2d_copy(ae->center,center);
    gfc_vector2d_copy(ae->flip,flip);
    if ((actorFile)&&(strlen(actorFile)))
    {
        ae->actor = gf2d_actor_load(actorFile);
        if ((action) && (strlen(action) > 0))
        {
            ae->action = gf2d_actor_get_action(ae->actor, action ,&ae->frame);
        }
    }
    else if (image != NULL)
    {
        ae->image = gf2d_sprite_load_image((char *)image);
    }
    return ae;
}

const char *gf2d_element_actor_get_action_name(Element *e)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return NULL;

    ae = (ActorElement *)e->data;
    if (!ae->action)return NULL;
    return ae->action->name;
}

void gf2d_element_actor_next_action(Element *e)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    ae->action = gf2d_actor_get_next_action(ae->actor,ae->action);
    if (ae->action)ae->frame = ae->action->startFrame;
}

void gf2d_element_actor_auto_scale(Element *e)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    if (ae->actor)
    {
        ae->scale.x = e->drawBounds.w / (float)ae->actor->size.x;
        ae->scale.y = e->drawBounds.h / (float)ae->actor->size.y;
    }
    else if (ae->image)
    {
        ae->scale.x = e->drawBounds.w / (float)ae->image->frameWidth ;
        ae->scale.y = e->drawBounds.h / (float)ae->image->frameHeight;
    }
}

void gf2d_element_actor_set_actor(Element *e, const char *actorFile)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    if (ae->actor)
    {
        gf2d_actor_free(ae->actor);
        ae->actor = NULL;
    }
    if (ae->image)
    {
        gf2d_sprite_free(ae->image);
        ae->image = NULL;
    }
    if (actorFile)
    {
        ae->actor = gf2d_actor_load(actorFile);
    }
    ae->action = NULL;
    ae->frame = 0;
}

void gf2d_element_actor_set_image(Element *e, const char *imageFile)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    if (ae->actor)
    {
        gf2d_actor_free(ae->actor);
        ae->actor = NULL;
    }
    if (ae->image)
    {
        gf2d_sprite_free(ae->image);
        ae->image = NULL;
    }
    if (imageFile)
    {
        ae->image = gf2d_sprite_load_image(imageFile);
    }
    ae->action = NULL;
    ae->frame = 0;
}


void gf2d_element_actor_set_frame(Element *e, Uint32 i)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    ae->frame = i;
}

void gf2d_element_actor_set_scale(Element *e, GFC_Vector2D scale)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    ae = (ActorElement *)e->data;
    ae->scale = scale;
}


void gf2d_element_actor_set_action(Element *e, const char *action)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    if (e->type != ET_Actor)return;
    ae = (ActorElement *)e->data;
    if (!ae->actor)return;
    ae->action = gf2d_actor_get_action(ae->actor, action,&ae->frame);
}


Actor *gf2d_element_actor_get_actor(Element *e)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return NULL;

    ae = (ActorElement *)e->data;
    if (!ae)return NULL;
    return ae->actor;
}

Element *gf2d_actor_get_next(Element *element, Element *from)
{
    if (element == from)return from;
    return NULL;
}

void gf2d_element_make_actor(Element *e,ActorElement *actor)
{
    if (!e)return;
    e->data = (void*)actor;
    e->type = ET_Actor;
    e->draw = gf2d_element_actor_draw;
    e->update = gf2d_element_actor_update;
    e->free_data = gf2d_element_actor_free;
    e->get_next = gf2d_actor_get_next;
}

void gf2d_element_load_actor_from_config(Element *e,SJson *json)
{
    GFC_Vector2D flip = {0};
    GFC_Vector2D center = {0};
    Bool scaleToFit = 0;
    float rotation = 0;
    SJson *value;
    const char *buffer = NULL;
    const char *action = NULL;
    const char *image = NULL;
    GFC_Vector2D scale;
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
    value = sj_object_get_value(json,"actor");
    if (value)
    {
        buffer = sj_get_string_value(value);
        value = sj_object_get_value(json,"action");
        if (value)
        {
            action = sj_get_string_value(value);
        }
    }
    else
    {
        value = sj_object_get_value(json,"image");
        image = sj_get_string_value(value);
    }

    sj_object_get_value_as_float(json,"rotation",&rotation);
    rotation *= GFC_DEGTORAD;//always accept degrees from people, but work in radians
    sj_value_as_vector2d(sj_object_get_value(json,"flip"),&flip);
    sj_value_as_vector2d(sj_object_get_value(json,"center"),&center);
    

    scale.x = scale.y = 1;
    sj_value_as_vector2d(sj_object_get_value(json,"scale"),&scale);
    sj_object_get_value_as_bool(json,"scaleToFit",&scaleToFit);
    gf2d_element_make_actor(e,gf2d_element_actor_new_full((char *)buffer,(char *)action,scale,image,center,flip,rotation));
    if (scaleToFit)
    {
        gf2d_element_actor_auto_scale(e);
    }
}
/*eol@eof*/
