#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_color.h"
#include "gfc_text.h"
#include "gfc_config.h"
#include "gfc_pak.h"

#include "gf2d_figure.h"

typedef struct
{
    Uint32 maxFigures;
    Figure *figureGFC_List;
}FigureManager;

static FigureManager figure_manager = {0};
void gf2d_figure_instance_delete_links(FigureInstance *instance);

void gf2d_figure_system_close()
{
    int i;
    if (figure_manager.figureGFC_List != NULL)
    {
        for (i = 0;i < figure_manager.maxFigures;i++)
        {
            gf2d_figure_free(&figure_manager.figureGFC_List[i]);
        }
        free(figure_manager.figureGFC_List);
    }
    memset(&figure_manager,0,sizeof(FigureManager));
    slog("figure system closed");
}

void gf2d_figure_init(Uint32 maxFigures)
{
    if (!maxFigures)
    {
        slog("cannot initialize figure system for zero entities");
        return;
    }
    memset(&figure_manager,0,sizeof(FigureManager));
    
    figure_manager.figureGFC_List = (Figure*)gfc_allocate_array(sizeof(Figure),maxFigures);
    if (!figure_manager.figureGFC_List)
    {
        slog("failed to allocate figure list");
        gf2d_figure_system_close();
        return;
    }
    figure_manager.maxFigures = maxFigures;
    atexit(gf2d_figure_system_close);
    slog("figure system initialized");
}

void gf2d_figure_free(Figure *figure)
{
    int i,c;
    FigureLink *link;
    if (!figure)return;
    gf2d_armature_free(figure->armature);
    c = gfc_list_get_count(figure->links);
    for (i = 0;i < c; i++)
    {
        link = gfc_list_get_nth(figure->links,i);
        if (!link)continue;
        free(link);
        link = NULL;
    }
    gfc_list_delete(figure->links);
    memset(figure,0,sizeof(Figure));
}

Figure *gf2d_figure_new()
{
    int i;
    for (i = 0; i < figure_manager.maxFigures;i++)
    {
        if (figure_manager.figureGFC_List[i]._inuse)continue;
        memset(&figure_manager.figureGFC_List[i],0,sizeof(Figure));
        figure_manager.figureGFC_List[i]._inuse = 1;
        figure_manager.figureGFC_List[i].links = gfc_list_new();
        gfc_line_cpy(figure_manager.figureGFC_List[i].name,"<new figure>");
        return &figure_manager.figureGFC_List[i];
    }
    return NULL;
}

GFC_Action *gf2d_figure_get_action(Figure *figure, const char *name,float *frame)
{
    if ((!figure)||(!figure->armature))return NULL;
    return gf2d_armature_get_action(figure->armature, name,frame);
}

GFC_Action *gf2d_figure_get_action_by_index(Figure *figure,Uint32 index)
{
    if ((!figure)||(!figure->armature))return NULL;
    return gf2d_armature_get_action_by_index(figure->armature,index);
}

void gf2d_figure_link_free(FigureLink **link)
{
    if ((!link)||(!(*link)))return;
    free((*link));
    *link = NULL;
}

FigureLink *gf2d_figure_link_new()
{
    FigureLink *link;
    link = gfc_allocate_array(sizeof(FigureLink),1);
    if (!link)return NULL;
    link->scale = gfc_vector2d(1,1);
    return link;
}

void gf2d_figure_link_set_actor(FigureLink *link,const char *actor,const char *action)
{
    if (!link)return;
    gfc_line_cpy(link->actorname,actor);
    gfc_line_cpy(link->action,action);
}

FigureLink *gf2d_figure_link_get_by_bonename(Figure *figure,const char *bonename)
{
    int i,c;
    FigureLink *link;
    if (!figure)return NULL;
    c = gfc_list_get_count(figure->links);
    for (i = 0; i < c; i++)
    {
        link = gfc_list_get_nth(figure->links,i);
        if (!link)continue;
        if (strcmp(bonename,link->bonename)==0)return link;
    }
    return NULL;
}

FigureLink *gf2d_figure_add_link_to_bone(Figure *figure, const char *bone,const char *actor,const char *action)
{
    FigureLink *link;
    if (!bone)return NULL;
    link = gf2d_figure_link_new();
    if (!link)return NULL;
    gfc_line_cpy(link->bonename,bone);
    if (actor)
    {
        gf2d_figure_link_set_actor(link,actor,action);
    }
    if (figure)
    {
        gfc_list_append(figure->links,link);
        if (figure->armature)
        {
            link->bone = gf2d_armature_get_bone_by_name(figure->armature, bone);
            if (!link->bone)
            {
                slog("failed to link new bone to figure");
            }
        }
    }
    return link;
}

FigureLink *gf2d_figure_link_from_json(SJson *load)
{
    FigureLink *link;
    if (!load)return NULL;
    link = gf2d_figure_link_new();
    if (!link)return NULL;

    gfc_line_cpy(link->bonename,sj_get_string_value(sj_object_get_value(load,"bonename")));
    gfc_line_cpy(link->actorname,sj_get_string_value(sj_object_get_value(load,"actorname")));
    gfc_line_cpy(link->action,sj_get_string_value(sj_object_get_value(load,"action")));
    sj_value_as_vector2d(sj_object_get_value(load,"scale"),&link->scale);
    sj_value_as_vector2d(sj_object_get_value(load,"offset"),&link->offset);
    sj_get_float_value(sj_object_get_value(load,"rotation"),&link->rotation);
    return link;
}

void gf2d_figure_link_armature(Figure *figure,Armature2D *armature)
{
    int i,c;
    FigureLink *link;
    if (!figure)return;
    if (armature)
    {
        figure->armature = armature;
    }
    if (!figure->armature)
    {
        slog("cannot link figure to armature: none provided");
        return;
    }
    c = gfc_list_get_count(figure->links);
    for (i = 0; i < c; i++)
    {
        link = gfc_list_get_nth(figure->links,i);
        if (!link)continue;
        link->bone = gf2d_armature_get_bone_by_name(figure->armature,link->bonename);
        if (!link->bone)
        {
            slog("failed to link bone '%s' to armature",link->bonename);
        }
    }
}

Figure *gf2d_figure_from_json(SJson *load,const char *filename)
{
    int i,c;
    SJson *array,*link;
    const char *tempStr;
    Figure *figure;
    if (!load)return NULL;
    figure = gf2d_figure_new();
    if (!figure)
    {
        sj_free(load);
        return NULL;
    }
    gfc_line_cpy(figure->filename,filename);
    tempStr = sj_get_string_value(sj_object_get_value(load,"name"));
    if (tempStr)
    {
        gfc_line_cpy(figure->name,tempStr);
    }
    tempStr = sj_get_string_value(sj_object_get_value(load,"armature"));
    if (tempStr)
    {
        figure->armature = gf2d_armature_load(tempStr);
    }
    array = sj_object_get_value(load,"links");
    c = sj_array_get_count(array);
    for (i =0; i < c;i++)
    {
        link = sj_array_get_nth(array,i);
        if (!link)continue;
        gfc_list_append(figure->links,gf2d_figure_link_from_json(link));
    }
    gf2d_figure_link_armature(figure,NULL);
    return figure;
}

Figure *gf2d_figure_load(const char *filepath)
{
    SJson *load;
    Figure *figure;
    if (!filepath)return NULL;
    load = gfc_pak_load_json(filepath);
    if (!load)return NULL;
    figure = gf2d_figure_from_json(load,filepath);
    sj_free(load);
    return figure;
}

SJson *gf2d_figure_link_to_json(FigureLink *link)
{
    SJson *save;
    if (!link)return NULL;
    save = sj_object_new();
    if (!save)return NULL;
    sj_object_insert(save,"bonename",sj_new_str(link->bonename));
    sj_object_insert(save,"actorname",sj_new_str(link->actorname));
    sj_object_insert(save,"offset",sj_vector2d_new(link->offset));
    sj_object_insert(save,"rotation",sj_new_float(link->rotation));    
    sj_object_insert(save,"scale",sj_vector2d_new(link->scale));
    sj_object_insert(save,"action",sj_new_str(link->action));
    return save;
}

void gf2d_figure_save(Figure *figure,const char *filepath)
{
    int i,c;
    FigureLink *link;
    SJson *save,*links;
    if ((!figure)||(!filepath))return;
    save = sj_object_new();
    if (!save)return;
    links = sj_array_new();
    if (!links)
    {
        sj_free(save);
        return;
    }
    gfc_line_cpy(figure->filename,filepath);    
    if (figure->armature)
    {
        sj_object_insert(save,"armature",sj_new_str(figure->armature->filepath));        
    }
    sj_object_insert(save,"name",sj_new_str(figure->name));

    c = gfc_list_get_count(figure->links);
    for (i = 0; i < c; i++)
    {
        link = gfc_list_get_nth(figure->links,i);
        if (!link)continue;
        sj_array_append(links,gf2d_figure_link_to_json(link));
    }
    sj_object_insert(save,"links",links);
    sj_save(save,filepath);
    sj_free(save);
}

void gf2d_figure_link_instance_draw(
    Figure *figure,
    FigureLinkInstance *linkInstance,
    Uint32   poseIndex,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    float    *rotation,
    GFC_Color    *color
)
{
    GFC_Vector2D flip = {0};
    FigureLink *link;
    BonePose *bonepose;
    GFC_Vector2D drawPosition,offset;
    float drawRotation;
    GFC_Vector2D drawScale = {1,1};
    if (!figure)return;
    if (!linkInstance)return;
    if (!linkInstance->link)return;
    link = linkInstance->link;//shorthand
    if (!linkInstance->actor)return;
    if (!link->bone)return;
    bonepose = gf2d_armature_get_bone_pose_by_name(figure->armature,poseIndex, link->bone->name);
    if (!bonepose)return;// can't draw without pose    
    
    drawRotation = link->rotation;
    if (rotation)drawRotation += *rotation;
    drawRotation += bonepose->angle;

    gfc_vector2d_add(drawPosition,bonepose->position,link->bone->rootPosition);
    
    offset = gfc_vector2d_rotate(link->offset, drawRotation);
    drawRotation *= GFC_RADTODEG;
    if (scale)
    {
        gfc_vector2d_scale_by(drawPosition,drawPosition,(*scale));
        if (drawRotation)drawRotation *= gfc_vector2d_scale_flip_rotation(*scale);
    }
    gfc_vector2d_add(drawPosition,drawPosition,position);

     gfc_vector2d_add(drawPosition,drawPosition,offset);
    if (scale)
    {
        gfc_vector2d_copy(drawScale,(*scale));
    }
    
    gf2d_actor_draw(
        linkInstance->actor,
        linkInstance->frame,
        drawPosition,
        &drawScale,
        NULL,
        &drawRotation,
        color,
        &flip);
}

void gf2d_figure_link_instance_draw_tweened(
    Figure *figure,
    FigureLinkInstance *linkInstance,
    Uint32   poseA,
    Uint32   poseB,
    float    fraction,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    float    *rotation,
    GFC_Color    *color
)
{
    GFC_Vector2D flip = {0};
    FigureLink *link;
    BonePose bonepose;
    GFC_Vector2D drawPosition,offset;
    float drawRotation;
    GFC_Vector2D drawScale = {1,1};
    if (!figure)return;
    if (!linkInstance)return;
    if (!linkInstance->link)return;
    link = linkInstance->link;//shorthand
    if (!linkInstance->actor)return;
    if (!link->bone)return;
    bonepose = gf2d_armature_get_tweened_pose_bone(figure->armature,poseA, poseB,link->bone->index, fraction);

    if (!bonepose.bone)return;// can't draw without pose    
    
    drawRotation = link->rotation;
    if (rotation)drawRotation += *rotation;
    drawRotation += bonepose.angle;

    gfc_vector2d_add(drawPosition,bonepose.position,link->bone->rootPosition);
    
    offset = gfc_vector2d_rotate(link->offset, drawRotation);
    drawRotation *= GFC_RADTODEG;
    if (scale)
    {
        gfc_vector2d_scale_by(drawPosition,drawPosition,(*scale));
        if (drawRotation)drawRotation *= gfc_vector2d_scale_flip_rotation(*scale);
    }
    gfc_vector2d_add(drawPosition,drawPosition,position);

     gfc_vector2d_add(drawPosition,drawPosition,offset);
    if (scale)
    {
        gfc_vector2d_copy(drawScale,(*scale));
    }
    
    gf2d_actor_draw(
        linkInstance->actor,
        linkInstance->frame,
        drawPosition,
        &drawScale,
        NULL,
        &drawRotation,
        color,
        &flip);
}

void gf2d_figure_link_instance_update_link(FigureLinkInstance *linkInstance)
{
    if (!linkInstance)return;
    if (!linkInstance->link)return;
    if (linkInstance->actor)
    {
        gf2d_actor_free(linkInstance->actor);
    }
    linkInstance->actor = gf2d_actor_load(linkInstance->link->actorname);
    if (linkInstance->actor)
    {
        linkInstance->action = gf2d_actor_get_action(linkInstance->actor, linkInstance->link->action,&linkInstance->frame);
    }
}

void gf2d_figure_link_instance_free(FigureLinkInstance *linkInstance)
{
    if (!linkInstance)return;
    gf2d_actor_free(linkInstance->actor);
    free(linkInstance);
}

FigureLinkInstance *gf2d_figure_link_instance_new(FigureLink *link)
{
    FigureLinkInstance *linkInstance;
    if (!link)return NULL;
    linkInstance = gfc_allocate_array(sizeof(FigureLinkInstance),1);
    if (!linkInstance)return NULL;
    linkInstance->actor = gf2d_actor_load(link->actorname);
    if (linkInstance->actor)
    {
        linkInstance->action = gf2d_actor_get_action(linkInstance->actor, link->action,&linkInstance->frame);
    }
    linkInstance->link = link;
    return linkInstance;
}

void gf2d_figure_instance_add_link(FigureInstance *instance, FigureLink *link)
{
    if (!instance)return;
    if (!link)return;
    if (instance->instances == NULL)
    {
        instance->instances = gfc_list_new();
    }
    gfc_list_append(instance->instances,gf2d_figure_link_instance_new(link));
}

void gf2d_figure_instance_new(FigureInstance *instance)
{
    if (!instance)return;
    if (instance->_inuse)return;
    if (instance->instances == NULL)
    {
        instance->instances = gfc_list_new();
    }
    instance->_inuse = 1;
}

void gf2d_figure_link_remove(Figure *figure,FigureLink *link)
{
    if ((!figure)||(!link))return;
    gfc_list_delete_data(figure->links,link);
    gf2d_figure_link_free(&link);
}

void gf2d_figure_instance_link(FigureInstance *instance)
{
    FigureLink *link;
    int i,c;
    if (!instance)return;
    if (instance->instances != NULL)
    {
        gf2d_figure_instance_delete_links(instance);
    }
    instance->instances = gfc_list_new();//clean slate
    if (!instance->figure)return;
    c = gfc_list_get_count(instance->figure->links);
    for (i = 0; i < c; i++)
    {
        link = gfc_list_get_nth(instance->figure->links,i);
        if (!link)continue;
        gf2d_figure_instance_add_link(instance, link);
    }
}

void gf2d_figure_instance_remove_link(FigureInstance *instance,FigureLinkInstance *link)
{
    if ((!instance)||(!link))return;
    gfc_list_delete_data(instance->instances,link);
    
    gf2d_figure_link_remove(instance->figure,link->link);
    
    gf2d_figure_link_instance_free(link);
}

void gf2d_figure_instance_link_change_actor(FigureInstance *instance,const char *name,const char *actor, const char *action)
{
    FigureLinkInstance *link;
    if (!instance)return;
    link = gf2d_figure_instance_get_link(instance, name);
    if (!link)
    {
        slog("figure instance has no link %s",name);
        return;
    }
    if (link->actor)
    {
        link->actor = NULL;
        gf2d_actor_free(link->actor);
    }
    if (!actor)return;// our work here is done
    link->actor = gf2d_actor_load(actor);
    if (!link->actor)return;
    link->action = gf2d_actor_get_action(link->actor, action,&link->frame);
}

FigureLinkInstance *gf2d_figure_instance_get_link(FigureInstance *instance, const char *name)
{
    FigureLinkInstance *linkInstance;
    int i,c;
    if (!instance)return NULL;
    c = gfc_list_get_count(instance->instances);
    for (i = 0;i < c;i++)
    {
        linkInstance = gfc_list_get_nth(instance->instances,i);
        if (!linkInstance)continue;
        if (linkInstance->link == NULL)continue;
        if (strcmp(linkInstance->link->bonename,name) == 0)return linkInstance;
    }
    return NULL;
}

void gf2d_figure_instance_delete_links(FigureInstance *instance)
{
    FigureLinkInstance *linkInstance;
    int i,c;
    if (!instance)return;
    c = gfc_list_get_count(instance->instances);
    for (i = 0;i < c;i++)
    {
        linkInstance = gfc_list_get_nth(instance->instances,i);
        if (!linkInstance)continue;
        gf2d_figure_link_instance_free(linkInstance);
    }
    gfc_list_delete(instance->instances);
}

void gf2d_figure_instance_free(FigureInstance *instance)
{
    if (!instance)return;
    gf2d_figure_free(instance->figure);
    gf2d_figure_instance_delete_links(instance);
    memset(instance,0,sizeof(FigureInstance));
}

int gf2d_figure_instance_load(FigureInstance *instance,const char *figurename)
{
    if (!instance)return -1;
    instance->figure = gf2d_figure_load(figurename);
    if (!instance->figure)return -1;
    instance->instances = gfc_list_new();
    if (!instance->instances)
    {
        gf2d_figure_instance_free(instance);
        return -1;
    }
    
    gf2d_figure_instance_link(instance);
    instance->_inuse = 1;
    return 0;
}


void gf2d_figure_instance_draw(
    FigureInstance *instance,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    float *rotation,
    GFC_Color *color,
    int tween)
{
    int i,c;
    Uint32   poseA;
    float    fraction;
    GFC_TextLine bonename;
    FigureLinkInstance *linkInstance;
    FigureLinkInstance *linkInstanceOther;
    if (!instance)
    {
        slog("no instance provided");
        return;
    }
    c = gfc_list_get_count(instance->instances);
    for (i = 0; i < c; i++)
    {
        linkInstance = gfc_list_get_nth(instance->instances,i);
        if (!linkInstance)continue;
        if ((scale)&&(scale->x < 0)&&(linkInstance->link != NULL))
        {
            if (gf2d_armature_bone_name_lr_flip(bonename,linkInstance->link->bonename))
            {
                linkInstanceOther = gf2d_figure_instance_get_link(instance, bonename);
                if (linkInstanceOther)linkInstance = linkInstanceOther;
            }
        }
        if (tween)
        {
            poseA = (Uint32)instance->frame;
            fraction = instance->frame - (float)poseA;
            gf2d_figure_link_instance_draw_tweened(
                instance->figure,
                linkInstance,
                poseA,
                instance->nextFrame,
                fraction,
                position,
                scale,
                rotation,
                color);
        }
        else
        {
            gf2d_figure_link_instance_draw(
                instance->figure,
                linkInstance,
                (int)instance->frame,
                position,
                scale,
                rotation,
                color
            );
        }
    }
}

/*eol@eof*/
