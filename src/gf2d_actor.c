#include <stdio.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"

#include "gf2d_actor.h"


typedef struct
{
    Uint32 maxActors;
    Actor * actorList;
}ActorManager;

static ActorManager actor_manager = {0};

const char *actionTypes[] =
{
    "none",
    "loop",
    "pass"
};

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
    slog("actor system closed");
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
    slog("actor system initialized");
    atexit(gf2d_actor_close);
}

void gf2d_action_list_delete(List *list)
{
    Action *action;
    if (!list)return;
    int i,c;
    c = gfc_list_get_count(list);
    for (i = 0; i < c;i++)
    {
        action = gfc_list_get_nth(list,i);
        if (!action)continue;
        free(action);
    }
    gfc_list_delete(list);
}

void gf2d_actor_delete(Actor *actor)
{
    if (!actor)return;
    gf2d_action_list_delete(actor->al);
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
            actor_manager.actorList[i].al = gfc_list_new();
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
            actor_manager.actorList[i].al = gfc_list_new();
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

const char *gf2d_actor_type_to_text(ActionType type)
{
    if (type >= AT_MAX)return 0;
    return actionTypes[type];
}

ActionType gf2d_actor_type_from_text(const char *text)
{
    if (!text)return AT_NONE;
    if (strcmp(text,"pass")==0)return AT_PASS;
    if (strcmp(text,"loop")==0)return AT_LOOP;
    return AT_NONE;
}


Action *gf2d_action_new()
{
    return gfc_allocate_array(sizeof(Action),1);
}

Action *gf2d_action_json_parse(
    SJson *actionSJ
)
{
    Action *action;
    int tempInt;
    float tempFloat;
    const char *tempStr;
    if (!actionSJ)
    {
        return NULL;
    }
    action = gfc_allocate_array(sizeof(Action),1);
    if (!action)return NULL;
    tempStr = sj_get_string_value(sj_object_get_value(actionSJ,"action"));
    if (tempStr)
    {
        gfc_line_cpy(action->name,tempStr);
    }
    tempStr = sj_get_string_value(sj_object_get_value(actionSJ,"type"));
    if (strcmp(tempStr,"loop")==0)
    {
        action->type = AT_LOOP;
    }
    else if (strcmp(tempStr,"pass")==0)
    {
        action->type = AT_PASS;
    }
    else if (strcmp(tempStr,"none")==0)
    {
        action->type = AT_NONE;
    }
    sj_get_integer_value(sj_object_get_value(actionSJ,"startFrame"),&tempInt);
    action->startFrame = tempInt;
    sj_get_integer_value(sj_object_get_value(actionSJ,"endFrame"),&tempInt);
    action->endFrame = tempInt;
    sj_get_float_value(sj_object_get_value(actionSJ,"frameRate"),&tempFloat);
    action->frameRate = tempFloat;
    return action;
}

SJson *gf2d_action_to_json(Action *action)
{
    SJson *save;
    if (!action)return NULL;
    save = sj_object_new();
    if (!save)return NULL;
    sj_object_insert(save,"action",sj_new_str(action->name));
    sj_object_insert(save,"type",sj_new_str(actionTypes[action->type]));
    sj_object_insert(save,"startFrame",sj_new_int(action->startFrame));
    sj_object_insert(save,"endFrame",sj_new_int(action->endFrame));
    sj_object_insert(save,"frameRate",sj_new_float(action->frameRate));
    return save;
}

SJson *gf2d_action_list_to_json(List *actions)
{
    int i,c;
    SJson *actionJS;
    Action *action;
    if (!actions)return NULL;
    actionJS = sj_array_new();
    if (!actionJS)return NULL;
    c = gfc_list_get_count(actions);
    for (i = 0;i < c;i++)
    {
        action = gfc_list_get_nth(actions,i);
        if (!action)continue;
        sj_array_append(actionJS,gf2d_action_to_json(action));
    }
    return actionJS;
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
    sj_object_insert(save,"actionList",gf2d_action_list_to_json(actor->al));
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

List *gf2d_action_list_parse(List *al,SJson *actionList)
{
    SJson *item;
    int i,c;
    if (!actionList)return al;
    if (!al)al = gfc_list_new();
    if (!al)return NULL;
    c = sj_array_get_count(actionList);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(actionList,i);
        if (!item)continue;
        al = gfc_list_append(al,gf2d_action_json_parse(item));
    }
    return al;
}

Actor *gf2d_actor_load_json(
    SJson *json,
    const char *filename
)
{
    Vector4D color = {255,255,255,255};
    Actor *actor;
    SJson *actorJS = NULL;
    const char *tempStr;
    if ((!json)||(!filename))
    {
        return NULL;
    }
    actor = gf2d_actor_new();
    if (!actor)return NULL;
    actorJS = sj_object_get_value(json,"actor");
    if (!actorJS)
    {
        slog("missing actor object in actor file");
        return NULL;
    }

    
    gfc_line_cpy(actor->filename,filename);
    sj_get_integer_value(sj_object_get_value(actorJS,"frameWidth"),&actor->frameWidth);
    sj_get_integer_value(sj_object_get_value(actorJS,"frameHeight"),&actor->frameHeight);
    sj_get_integer_value(sj_object_get_value(actorJS,"framesPerLine"),&actor->framesPerLine);
    tempStr = sj_get_string_value(sj_object_get_value(actorJS,"sprite"));
    if (tempStr)
    {
        actor->sprite = gf2d_sprite_load(
            tempStr,
            actor->frameWidth,
            actor->frameHeight,
            actor->framesPerLine);
        gfc_line_cpy(actor->spriteFile,tempStr);
    }
    
    sj_value_as_vector2d(sj_object_get_value(actorJS,"scale"),&actor->scale);
    sj_value_as_vector2d(sj_object_get_value(actorJS,"center"),&actor->center);
    sj_value_as_vector2d(sj_object_get_value(actorJS,"drawOffset"),&actor->drawOffset);
    sj_value_as_vector4d(sj_object_get_value(actorJS,"color"),&color);
    actor->color = gfc_color_from_vector4(color);
    
    actor->size.x = actor->frameWidth * actor->scale.x;
    actor->size.y = actor->frameHeight * actor->scale.y;

    actor->al = gf2d_action_list_parse(actor->al,sj_object_get_value(actorJS,"actionList"));
    return actor;
}

Action *gf2d_action_list_get_action(List *al, const char *name)
{
    Action *action;
    int i,c;
    if (!al)
    {
        slog("no action list provided");
        return NULL;
    }
    if (!name)
    {
        slog("no filename provided");
        return NULL;
    }
    c = gfc_list_get_count(al);
    for (i = 0; i < c;i++)
    {
        action = gfc_list_get_nth(al,i);
        if (!action)continue;
        if (gfc_line_cmp(action->name,name) == 0)
        {
            return action;
        }
    }
    return NULL;// not found
}

Uint32 gf2d_action_next_frame_after(Action *action,float frame)
{
    Uint32 ret = 0;
    if (!action)return 0;
    if (frame < action->startFrame)ret= action->startFrame + 1;
    else
    {
        ret = ceil(frame);
        if (ret > action->endFrame)
        {
            if (action->type == AT_LOOP)ret = action->startFrame;
            ret = action->endFrame;
        }
    }
    return ret;
}

ActionReturnType gf2d_action_next_frame(Action *action,float *frame)
{
    if ((!action)||(!frame))
    {
        return ART_ERROR;
    }
    if (*frame < action->startFrame)
    {
        *frame = action->startFrame;
        return ART_START;
    }
    *frame = *frame + action->frameRate;
    if (*frame >= action->endFrame)
    {
        switch (action->type)
        {
            case AT_MAX:
            case AT_NONE:
                break;
            case AT_LOOP:
                *frame = action->startFrame;
                break;
            case AT_PASS:
                *frame = action->endFrame;
                return ART_END;
        }
    }
    return ART_NORMAL;
}

Actor *gf2d_actor_load_image(const char *file)
{
    Actor *actor;
    Action *action;
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
    actor->size = vector2d(sprite->frameWidth,sprite->frameHeight);
    actor->scale = vector2d(1,1);
    actor->center = vector2d(sprite->frameWidth*0.5,sprite->frameHeight *0.5);
    actor->color = gfc_color8(255,255,255,255);
//    actor->drawOffset;  left zero
    actor->al = gfc_list_new();
    action = gf2d_action_new();
    gfc_line_cpy(action->name,"default");
    actor->al = gfc_list_append(actor->al,action);
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
    json = sj_load(file);
    if (json)
    {
        actor = gf2d_actor_load_json(
            json,
            file);
        sj_free(json);
        return actor;
    }
    // if it failed to load as json, then lets try it as a flat image
    return gf2d_actor_load_image(file);
}

void gf2d_action_list_frame_inserted(List *list,Uint32 index)
{
    Action *action;
    int i,c;
    if (!list)return;
    c = gfc_list_get_count(list);
    for (i = 0;i < c; i++)
    {
        action = gfc_list_get_nth(list,i);
        if (!action)continue;
        if (action->startFrame > index)action->startFrame++;
        if (action->endFrame > index)action->endFrame++;
    }
}

void gf2d_action_list_frame_deleted(List *list,Uint32 index)
{
    Action *action;
    int i,c;
    if (!list)return;
    c = gfc_list_get_count(list);
    for (i = 0;i < c; i++)
    {
        action = gfc_list_get_nth(list,i);
        if (!action)continue;
        if (action->startFrame > index)action->startFrame--;
        if (action->endFrame > index)action->endFrame--;
    }
}


Action *gf2d_actor_set_action(Actor *actor, const char *name,float *frame)
{
    Action *action;
    if (!actor)return NULL;
    action = gf2d_actor_get_action_by_name(actor,name);
    if (!action)action = gf2d_actor_get_action_by_index(actor,0);
    if (!action)return NULL;
    if (frame)*frame = action->startFrame;
    return action;
}

Action *gf2d_action_list_get_action_by_frame(List *list,Uint32 frame)
{
    Action *action;
    int i,c;
    if (!list)return NULL;
    c = gfc_list_get_count(list);
    for (i = 0; i < c; i++)
    {
        action = gfc_list_get_nth(list,i);
        if (!action)continue;
        if ((frame >= action->startFrame)&&(frame <= action->endFrame))return action;
    }
    return NULL;
}

Action *gf2d_action_list_get_action_by_name(List *list,const char *name)
{
    Action *action;
    int i,c;
    if ((!list)||(!name))return NULL;
    c = gfc_list_get_count(list);
    for (i = 0; i < c; i++)
    {
        action = gfc_list_get_nth(list,i);
        if (!action)continue;
        if (strcmp(action->name,name)==0)return action;
    }
    return NULL;
}

Action *gf2d_actor_get_action_by_name(Actor *actor,const char *name)
{
    if (!actor)return NULL;
    return gf2d_action_list_get_action_by_name(actor->al,name);
}

Uint32 gf2d_actor_get_action_count(Actor *actor)
{
    if (!actor)return 0;
    return gfc_list_get_count(actor->al);
}

Action *gf2d_actor_get_action_by_index(Actor *actor,Uint32 index)
{
    if (!actor)return NULL;
    return gfc_list_get_nth(actor->al,index);
}

Action *gf2d_actor_get_next_action(Actor *actor,Action *action)
{
    int i;
    if (!actor)return NULL;
    if (!action)return gfc_list_get_nth(actor->al,0);
    i = gfc_list_get_item_index(actor->al,action);
    if ((i == -1)||(i >= (gfc_list_get_count(actor->al) - 1)))
    {
        return gfc_list_get_nth(actor->al,0);
    }
    return gfc_list_get_nth(actor->al,i + 1);
}

Uint32 gf2d_action_get_framecount(Action *action)
{
    if (!action)return 0;
    return action->endFrame - action->startFrame;
}

Uint32 gf2d_actor_get_framecount(Actor *actor)
{
    Action *action;
    int i,c;
    Uint32 count = 0;
    if (!actor)return 0;
    c = gfc_list_get_count(actor->al);
    for (i = 0; i < c; i++)
    {
        action = gfc_list_get_nth(actor->al,i);
        if (!action)continue;
        if (action->endFrame > count)count = action->endFrame;
    }
    return count;
}

void gf2d_actor_draw(
    Actor *actor,
    float frame,
    Vector2D position,
    Vector2D * scale,
    Vector2D * center,
    float    * rotation,
    Color    * color,
    Vector2D * flip
)
{
    Color drawColor;
    Vector2D drawCenter;
    Vector2D drawScale;
    Vector2D drawPosition;
    Vector3D drawRotation = {0};
    if (!actor)return;
    vector2d_copy(drawScale,actor->scale);
    if (rotation)
    {
        drawRotation.z =*rotation;
    }
    if (center)
    {
        vector2d_copy(drawCenter,(*center));
        vector2d_copy(drawRotation,(*center));
    }
    else
    {
        vector2d_copy(drawCenter,actor->center);
    }
    if (scale)
    {
        drawScale.x *= scale->x;
        drawScale.y *= scale->y;
    }
    if (color)
    {
        gfc_color_multiply(&drawColor,*color,actor->color);
        drawColor = gfc_color_to_int8(drawColor);
    }
    else
    {
        drawColor = gfc_color_to_int8(actor->color);
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
    
    vector2d_add(drawPosition,position,actor->drawOffset);
    gf2d_sprite_draw(
        actor->sprite,
        drawPosition,
        drawScale,
        drawRotation,
        drawColor,
        (int)frame);
/*    gf2d_sprite_draw(
        actor->sprite,
        drawPosition,
        &drawScale,
        &drawCenter,
        rotation,
        flip,
        &drawColor,
        (int)frame);
*/
}

int gf2d_action_get_animation_frames(Action *action)
{
    if (!action)
    {
        return -1;
    }
    if (action->frameRate == 0)
    {
        return -1;//infinite!!! we never stop!
    }
    return (int)((action->endFrame - action->startFrame)/action->frameRate);

}

int gf2d_action_get_percent_complete(Action *action,float frame)
{
    float total,passed;
    if (!action)
    {
        return -1;
    }
    if (action->frameRate == 0)
    {
        return -1;//infinite!!! we never stop!
    }
    total = (action->endFrame - action->startFrame)/action->frameRate;
    passed = (frame - action->startFrame)/action->frameRate;
    if (!total)return 0;
    return (int)((passed*100)/total);
}

int gf2d_action_get_frames_remaining(Action *action,float frame)
{
    float total,passed;
    if (!action)
    {
        return -1;
    }
    if (action->frameRate == 0)
    {
        return -1;//infinite!!! we never stop!
    }
    total = (action->endFrame - action->startFrame)/action->frameRate;
    passed = (frame - action->startFrame)/action->frameRate;
    return (int)(total - passed);
}

int gf2d_action_get_action_frame(Action *action,float frame)
{
    if (!action)
    {
        return -1;
    }
    if (action->frameRate == 0)
    {
        return -1;//infinite!!! we never stop!
    }
    return (int)((frame - action->startFrame)/action->frameRate);
}


/*eol@eof*/
