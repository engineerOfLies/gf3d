#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "gf3d_lights.h"

#include "world.h"

World *world_load(char *filename)
{
    Vector4D globalColor = {0};
    Vector3D globalDir  = {0};
    SJson *json,*wjson;
    World *w = NULL;
    Vector3D skyScale = {1,1,1};
    const char *modelName = NULL;
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
    modelName = sj_get_string_value(sj_object_get_value(wjson,"model"));
    if (!modelName)
    {
        slog("world data (%s) has no model",filename);
        sj_free(json);
        return w;
    }
    w->model = gf3d_model_load(modelName);
    
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
    sj_value_as_vector3d(sj_object_get_value(wjson,"scale"),&w->scale);
    sj_value_as_vector3d(sj_object_get_value(wjson,"position"),&w->position);
    sj_value_as_vector3d(sj_object_get_value(wjson,"rotation"),&w->rotation);
    sj_value_as_vector3d(sj_object_get_value(wjson,"lightDir"),&globalDir);
    sj_value_as_vector4d(sj_object_get_value(wjson,"lightColor"),&globalColor);
    sj_free(json);
    w->color = gfc_color(1,1,1,1);
    gfc_matrix_identity(w->skyMat);
    gfc_matrix_scale(w->skyMat,skyScale);
    gf3d_lights_set_global_light(globalColor,vector4d(globalDir.x,globalDir.y,globalDir.z,1));
    return w;
}

void world_draw(World *world)
{
    if (!world)return;
    if (!world->model)return;// no model to draw, do nothing
    gf3d_model_draw_sky(world->sky,world->skyMat,gfc_color(1,1,1,1));
    gf3d_model_draw(world->model,world->modelMat,gfc_color_to_vector4f(world->color),vector4d(1,1,1,0.5));
}

void world_delete(World *world)
{
    if (!world)return;
    gf3d_model_free(world->model);
    free(world);
}

void world_run_updates(World *self)
{
    self->rotation.z += 0.0001;
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_scale(self->modelMat,self->scale);
    gfc_matrix_rotate_by_vector(self->modelMat,self->modelMat,self->rotation);
    gfc_matrix_translate(self->modelMat,self->position);

}

void world_add_entity(World *world,Entity *entity);


/*eol@eof*/
