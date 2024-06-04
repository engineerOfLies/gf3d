#include <stdio.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_pak.h"

#include "gf2d_actor.h"


typedef struct
{
    Uint32 maxActors;
    Actor * actorList;
}ActorManager;

static ActorManager actor_manager = {0};

void gf2d_actor_clear_all();

void gf2d_actor_close()
{
    gf2d_actor_clear_all();
    if (actor_manager.actorList != NULL)
    {
        free(actor_manager.actorList);
    }
    actor_manager.actorList = NULL;
    actor_manager.maxActors = 0;
}

void gf2d_actor_init(Uint32 max)
{
    if (!max)
    {
        slog("cannot intialize actor manager for Zero actors!");
        return;
    }
    actor_manager.maxActors = max;
    actor_manager.actorList = (Actor *)gfc_allocate_array(sizeof(Actor),max);
    atexit(gf2d_actor_close);
}

void gf2d_actor_delete(Actor *actor)
{
    if (!actor)return;
    gfc_action_list_free(actor->al);
    if (actor->sprite)gf2d_sprite_free(actor->sprite);
    memset(actor,0,sizeof(Actor));
}

void gf2d_actor_free(Actor *actor)
{
    if (!actor) return;
    actor->_refCount--;
}

void gf2d_actor_clear_all()
{
    int i;
    for (i = 0;i < actor_manager.maxActors;i++)
    {
        gf2d_actor_delete(&actor_manager.actorList[i]);// clean up the data
    }
}

Actor *gf2d_actor_new()
{
    int i;
    /*search for an unused actor address*/
    for (i = 0;i < actor_manager.maxActors;i++)
    {
        if ((actor_manager.actorList[i]._refCount == 0)&&(actor_manager.actorList[i].al == NULL))
        {
            actor_manager.actorList[i]._refCount = 1;//set ref count
            actor_manager.actorList[i].al = gfc_action_list_new();
            return &actor_manager.actorList[i];//return address of this array element        }
        }
    }
    /*find an unused actor address and clean up the old data*/
    for (i = 0;i < actor_manager.maxActors;i++)
    {
        if (actor_manager.actorList[i]._refCount == 0)
        {
            gf2d_actor_delete(&actor_manager.actorList[i]);// clean up the old data
            actor_manager.actorList[i]._refCount = 1;//set ref count
            actor_manager.actorList[i].al = gfc_action_list_new();
            return &actor_manager.actorList[i];//return address of this array element
        }
    }
    slog("error: out of actor addresses");
    return NULL;
}

Actor *gf2d_actor_get_by_filename(const char * filename)
{
    int i;
    if (!filename)
    {
        return NULL;
    }
    for (i = 0;i < actor_manager.maxActors;i++)
    {
        if (gfc_line_cmp(actor_manager.actorList[i].filename,filename)==0)
        {
            return &actor_manager.actorList[i];
        }
    }
    return NULL;// not found
}

SJson *gf2d_actor_to_json(Actor *actor)
{
    SJson *save;
    if (!actor)return NULL;
    save = sj_object_new();
    if (!save)return NULL;
    sj_object_insert(save,"sprite",sj_new_str(actor->spriteFile));
    sj_object_insert(save,"frameWidth",sj_new_int(actor->frameWidth));
    sj_object_insert(save,"frameHeight",sj_new_int(actor->frameHeight));
    sj_object_insert(save,"framesPerLine",sj_new_int(actor->framesPerLine));
    sj_object_insert(save,"scale",sj_vector2d_new(actor->scale));
    sj_object_insert(save,"center",sj_vector2d_new(actor->center));
    sj_object_insert(save,"color",sj_vector4d_new(gfc_color_to_vector4(actor->color)));
    sj_object_insert(save,"drawOffset",sj_vector2d_new(actor->drawOffset));
    sj_object_insert(save,"actionList",gfc_action_list_to_json(actor->al));
    return save;
}

void gf2d_actor_save(Actor *actor,const char *filename)
{
    SJson *save,*actorjs;
    if ((!actor)||(!filename))return;
    actorjs = sj_object_new();
    if (!actorjs)return;
    save = gf2d_actor_to_json(actor);
    if (!save)return;
    sj_object_insert(actorjs,"actor",save);
    sj_save(actorjs,filename);
    sj_free(actorjs);
}

Actor *gf2d_actor_load_json(SJson *json)
{
    GFC_Vector4D color = {255,255,255,255};
    GFC_Vector2D scaleTo;
    Actor *actor;
    SJson *actorJS = NULL;
    if (!json)
    {
        return NULL;
    }
    actor = gf2d_actor_new();
    if (!actor)return NULL;
    actorJS = sj_object_get_value(json,"actor");
    if (!actorJS)
    {
        slog("missing actor object in sprite actor file");
        return NULL;
    }
    if (sj_get_string_value(sj_object_get_value(actorJS,"sprite")))
    {
        actor->sprite = gf2d_sprite_parse(actorJS);
        if (actor->sprite)
        {
            gfc_line_cpy(actor->spriteFile,actor->sprite->filename);
        }
    }

    sj_value_as_vector2d(sj_object_get_value(actorJS,"scale"),&actor->scale);
    if ((actor->sprite) && (sj_value_as_vector2d(sj_object_get_value(actorJS,"scaleTo"),&scaleTo)))
    {
        actor->scale.x = scaleTo.x / actor->sprite->frameWidth;
        actor->scale.y = scaleTo.y / actor->sprite->frameHeight;
    }
    sj_value_as_vector2d(sj_object_get_value(actorJS,"center"),&actor->center);
    sj_value_as_vector2d(sj_object_get_value(actorJS,"drawOffset"),&actor->drawOffset);
    sj_value_as_vector4d(sj_object_get_value(actorJS,"color"),&color);
    actor->color = gfc_color_from_vector4(color);
    
    actor->size.x = actor->frameWidth * actor->scale.x;
    actor->size.y = actor->frameHeight * actor->scale.y;

    actor->al = gfc_action_list_parse(sj_object_get_value(actorJS,"actionList"));
    return actor;
}

Actor *gf2d_actor_load_image(const char *file)
{
    Actor *actor;
    GFC_Action *action;
    Sprite *sprite;
    if (!file)return NULL;
    sprite = gf2d_sprite_load_image(file);
    if (!sprite)return NULL;
    actor = gf2d_actor_new();
    if (!actor)
    {
        gf2d_sprite_free(sprite);
        return NULL;
    }
    gfc_line_cpy(actor->filename,file);
    actor->sprite = sprite;
    gfc_line_cpy(actor->spriteFile,file);
    actor->frameWidth = sprite->frameWidth;
    actor->frameHeight = sprite->frameHeight;
    actor->framesPerLine = 1;
    actor->size = gfc_vector2d(sprite->frameWidth,sprite->frameHeight);
    actor->scale = gfc_vector2d(1,1);
    actor->center = gfc_vector2d(sprite->frameWidth*0.5,sprite->frameHeight *0.5);
    actor->color = gfc_color8(255,255,255,255);
    actor->al = gfc_action_list_new();
    action = gfc_action_new();
    gfc_line_cpy(action->name,"default");
    gfc_action_list_append(actor->al,action);
    return actor;
}

Actor *gf2d_actor_load(const char *file)
{
    SJson *json;
    Actor *actor;
    if ((!file)||(strlen(file) == 0))
    {
//        slog("no file provided for actor");
        return false;
    }
    actor = gf2d_actor_get_by_filename(file);
    if (actor)
    {
        actor->_refCount++;
        return actor;//found it already in memory
    }
    json = gfc_pak_load_json(file);
    if (json)
    {
        actor = gf2d_actor_load_json(
            json);
        sj_free(json);
        if (!actor)return NULL;
        gfc_line_cpy(actor->filename,file);
        return actor;
    }
    // if it failed to load as json, then lets try it as a flat image
    return gf2d_actor_load_image(file);
}

GFC_Action *gf2d_actor_get_action(Actor *actor, const char *name,float *frame)
{
    GFC_Action *action;
    if (!actor)return NULL;
    action = gf2d_actor_get_action_by_name(actor,name);
    if (!action)action = gf2d_actor_get_action_by_index(actor,0);
    if (!action)return NULL;
    if (frame)*frame = action->startFrame;
    return action;
}

GFC_Action *gf2d_actor_get_action_by_name(Actor *actor,const char *name)
{
    if (!actor)return NULL;
    return gfc_action_list_get_action_by_name(actor->al,name);
}

Uint32 gf2d_actor_get_action_count(Actor *actor)
{
    if ((!actor)||(!actor->al))return 0;
    return gfc_list_get_count(actor->al->actions);
}

GFC_Action *gf2d_actor_get_action_by_index(Actor *actor,Uint32 index)
{
    if (!actor)return NULL;
    return gfc_action_list_get_action_by_index(actor->al,index);
}

GFC_Action *gf2d_actor_get_next_action(Actor *actor,GFC_Action *action)
{
    if (!actor)return NULL;
    return gfc_action_list_get_next_action(actor->al,action);
}

Uint32 gf2d_actor_get_framecount(Actor *actor)
{
    if (!actor)return 0;
    return gfc_action_list_get_framecount(actor->al);
}

void gf2d_actor_draw(
    Actor *actor,
    float frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Color    * color,
    GFC_Vector2D * flip
)
{
    GFC_Color drawGFC_Color;
    GFC_Vector2D drawCenter;
    GFC_Vector2D drawScale = {1,1};
    GFC_Vector2D drawPosition;
    GFC_Vector4D drawClip = {0,0,0,0};
    if (!actor)return;
    gfc_vector2d_copy(drawScale,actor->scale);
    if (center)
    {
        gfc_vector2d_copy(drawCenter,(*center));
    }
    else
    {
        gfc_vector2d_copy(drawCenter,actor->center);
    }
    if (scale)
    {
        drawScale.x *= scale->x;
        drawScale.y *= scale->y;
    }
    if (color)
    {
        gfc_color_multiply(&drawGFC_Color,*color,actor->color);
        drawGFC_Color = gfc_color_to_int8(drawGFC_Color);
    }
    else
    {
        drawGFC_Color = gfc_color_to_int8(actor->color);
    }
    
    if ((drawScale.x < 0))
    {
        //flip the center point
        drawCenter.x = actor->frameWidth - drawCenter.x;
    }
    if ((drawScale.y < 0))//only for scale based flipping, normal flipping works fine
    {
        //flip the center point
        drawCenter.y = actor->frameHeight - drawCenter.y;
    }
    
    gfc_vector2d_add(drawPosition,position,actor->drawOffset);
    gf2d_sprite_draw(
        actor->sprite,
        drawPosition,
        &drawScale,
        &drawCenter,
        rotation,
        flip,
        &drawGFC_Color,
        &drawClip,
        (int)frame);
}

/*eol@eof*/
