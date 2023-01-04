#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "gf3d_lights.h"

#include "camera_entity.h"
#include "fighter.h"
#include "gate.h"
#include "player.h"
#include "world.h"

World *world_load(char *filename)
{
    Vector4D globalColor = {0};
    Vector3D globalDir  = {0};
    Vector3D position,rotation;
    SJson *json,*wjson,*list,*item;
    World *w = NULL;
    Vector3D skyScale = {1,1,1};
    int i,c;
    ModelMat *m;
    const char *modelName = NULL;
    const char *str;
    w = gfc_allocate_array(sizeof(World),1);
    if (w == NULL)
    {
        slog("failed to allocate data for the world");
        return NULL;
    }
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json file (%s) for the world data",filename);
        free(w);
        return NULL;
    }
    wjson = sj_object_get_value(json,"world");
    if (!wjson)
    {
        slog("failed to find world object in %s world condig",filename);
        free(w);
        sj_free(json);
        return NULL;
    }
    list = sj_object_get_value(wjson,"models");
    c = sj_array_get_count(list);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        m = gf3d_model_mat_new();
        if (!m)continue;
        gf3d_model_mat_parse(m,item);
        gf3d_model_mat_set_matrix(m);
        w->model_list = gfc_list_append(w->model_list,m);
    }
    
    modelName = sj_get_string_value(sj_object_get_value(wjson,"sky"));
    if (!modelName)
    {
        slog("world data (%s) has no sky",filename);
    }
    else
    {
        w->sky = gf3d_model_load(modelName);
    }
    sj_value_as_vector3d(sj_object_get_value(wjson,"skyScale"),&skyScale);
    gfc_matrix_identity(w->skyMat);
    gfc_matrix_scale(w->skyMat,skyScale);

    modelName = sj_get_string_value(sj_object_get_value(wjson,"backgroundMusic"));
    if (modelName)
    {
        w->backgroundMusic = Mix_LoadMUS(modelName);
        if (!w->backgroundMusic)
        {
            slog("failed to open background music %s",modelName);
        }
        else
        {
            Mix_PlayMusic(w->backgroundMusic,-1);
        }
    }

    list = sj_object_get_value(wjson,"lights");
    c = sj_array_get_count(list);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_get_string_value(sj_object_get_value(item,"type"));
        if (!str)continue;
        vector3d_set(globalDir,0,0,0);
        sj_value_as_vector3d(sj_object_get_value(item,"direction"),&globalDir);
        vector4d_set(globalColor,0,0,0,0);
        sj_value_as_vector4d(sj_object_get_value(item,"color"),&globalColor);
        vector3d_set(position,0,0,0);
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
        if (strcmp(str,"global")==0)
        {
            gf3d_lights_set_global_light(globalColor,vector4d(globalDir.x,globalDir.y,globalDir.z,1));;
            w->theSun = gf3d_particle(vector3d(-10000,-10000,0), gfc_color_from_vector4f(globalColor), 10000);
        }
    }
    item = sj_object_get_value(wjson,"camera");
    if (item)
    {
        sj_value_as_vector3d(sj_object_get_value(item,"position"),&position);
        vector3d_copy(w->cameraPosition,position);
        sj_value_as_vector3d(sj_object_get_value(item,"rotation"),&rotation);
        camera_entity_new(position,rotation);
    }
    
    sj_object_get_value_as_uint32(wjson,"hourTime",&w->hourTime);
    
    sj_free(json);
    gate_new(vector3d(0,-3000,0));
    return w;
}

ModelMat *world_get_model_mat(World *world,Uint32 index)
{
    if (!world)return NULL;
    return gfc_list_get_nth(world->model_list,index);
}

void world_draw(World *world)
{
    int i,c;
    ModelMat *m;
    if (!world)return;
    gf3d_model_draw_sky(world->sky,world->skyMat,gfc_color(1,1,1,1));
    c = gfc_list_get_count(world->model_list);
    for (i = 0; i < c; i++)
    {
        m = gfc_list_get_nth(world->model_list,i);
        if (!m)continue;
        gf3d_model_draw(m->model,0,m->mat,vector4d(1,1,1,1),vector4d(1,1,1,1),vector4d(1,1,1,0.5));
    }
    gf3d_particle_draw(world->theSun);
}

void world_delete(World *world)
{
    int i,c;
    ModelMat *m;
    if (!world)return;
    c = gfc_list_get_count(world->model_list);
    for (i = 0; i < c; i++)
    {
        m = gfc_list_get_nth(world->model_list,i);
        if (!m)continue;
        gf3d_model_mat_free(m);
    }
    gfc_list_delete(world->model_list);
    free(world);
}

void world_run_updates(World *w)
{
    Uint32 now;
    if (!w)return;
    now = SDL_GetTicks();
    if (now > (w->lastHour + w->hourTime))
    {
        w->lastHour = now;
        player_hour_advance();
    }
}


/*eol@eof*/
