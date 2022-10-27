#include <stdlib.h>
#include <stdio.h>
#include "simple_logger.h"

#include "gf2d_element_actor.h"

void gf2d_element_actor_draw(Element *element,Vector2D offset)
{
    ActorElement *actor;
    Vector2D position;
    if (!element)return;
    actor = (ActorElement*)element->data;
    if (!actor)return;
    vector2d_add(position,offset,element->bounds);
    if (actor->image)
    {
        gf2d_sprite_draw_full(
            actor->image,
            position,
            actor->scale,
            vector2d(0,0),
            0,
            actor->flip,
            element->color,
            actor->frame);
    }
    else if (actor->actor)
    {
        if (actor->center.x)
        {
            position.x += element->bounds.w / 2;
        }
        if (actor->center.y)
        {
            position.y += element->bounds.h / 2;
        }
        gf2d_actor_draw(
            actor->actor,
            actor->frame,
            position,
            &actor->scale,
            NULL,
            NULL,
            &element->color,
            &actor->flip);
    }
}

List * gf2d_element_actor_update(Element *element,Vector2D offset)
{
    ActorElement *actor;
    if (!element)return NULL;
    actor = (ActorElement*)element->data;
    if (!actor)return NULL;
    gf2d_action_next_frame(actor->action,&actor->frame);
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


ActorElement *gf2d_element_actor_new_full(const char *actorFile, const char *action,Vector2D scale,const char *image,Vector2D center,Vector2D flip)
{
    ActorElement *ae;
    ae = gf2d_element_actor_new();
    if (!ae)
    {
        return NULL;
    }
    vector2d_copy(ae->scale,scale);
    vector2d_copy(ae->center,center);
    vector2d_copy(ae->flip,flip);
    if ((actorFile)&&(strlen(actorFile)))
    {
        ae->actor = gf2d_actor_load(actorFile);
        if ((action) && (strlen(action) > 0))
        {
            ae->action = gf2d_actor_set_action(ae->actor, action ,&ae->frame);
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
    if (ae->actor->size.x)ae->scale.x = e->bounds.w/ ae->actor->size.x ;
    if (ae->actor->size.y)ae->scale.y = e->bounds.h /ae->actor->size.y;
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
    if (actorFile)
    {
        ae->actor = gf2d_actor_load(actorFile);
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

void gf2d_element_actor_set_action(Element *e, const char *action)
{
    ActorElement *ae;
    if ((!e)||(e->type != ET_Actor))return;

    if (e->type != ET_Actor)return;
    ae = (ActorElement *)e->data;
    if (!ae->actor)return;
    ae->action = gf2d_actor_set_action(ae->actor, action,&ae->frame);
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
    Vector2D flip = {0};
    Vector2D center = {0};
    SJson *value;
    const char *buffer = NULL;
    const char *action = NULL;
    const char *image = NULL;
    Vector2D scale;
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

    sj_value_as_vector2d(sj_object_get_value(json,"flip"),&flip);
    sj_value_as_vector2d(sj_object_get_value(json,"center"),&center);
    

    scale.x = scale.y = 1;
    sj_value_as_vector2d(sj_object_get_value(json,"scale"),&scale);
    gf2d_element_make_actor(e,gf2d_element_actor_new_full((char *)buffer,(char *)action,scale,image,center,flip));
}
/*eol@eof*/
