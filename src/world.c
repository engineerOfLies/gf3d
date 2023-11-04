#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "entity.h"
#include "world.h"

/*
typedef struct
{

    Model *worldModel;
    List *spawnList;        //entities to spawn
    List *entityList;       //entities that exist in the world
}World;
*/
World* current_world = NULL;

World *world_load(char *filename)
{
    SJson *json,*wjson;
    World *w = NULL;
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

    sj_value_as_vector3d(sj_object_get_value(wjson,"scale"),&w->scale);
    sj_value_as_vector3d(sj_object_get_value(wjson,"position"),&w->position);
    sj_value_as_vector3d(sj_object_get_value(wjson,"rotation"),&w->rotation);
    sj_free(json);
    w->color = gfc_color(1,1,1,1);
    w->size = gf3d_get_model_size_from_obj("models/antioch/antioch.obj");
    w->worldBoundingBox.min = get_World_Bounding_Box_Min(w->size, w->position);
    w->worldBoundingBox.max = get_World_Bounding_Box_Max(w->size, w->position);
    current_world = w;
    return w;
}

void world_draw(World *world)
{
    if (!world)return;
    if (!world->model)return;// no model to draw, do nothing
    gf3d_model_draw(world->model,world->modelMat,gfc_color_to_vector4f(world->color),vector4d(2,2,2,2));
    gf3d_model_draw_highlight(world->model,world->modelMat,vector4d(1,.5,.1,1));
}

void world_delete(World *world)
{
    if (!world)return;
    gf3d_model_free(world->model);
    free(world);
}

void world_run_updates(World *self)
{

    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_scale(self->modelMat,self->scale);
    gfc_matrix_rotate_by_vector(self->modelMat,self->modelMat,self->rotation);
    gfc_matrix_translate(self->modelMat,self->position);

}

void world_add_entity(World *world,Entity *entity);

Vector3D get_World_Bounding_Box_Max(Vector3D size, Vector3D position)
{
    Vector3D max;
    max.x = position.x + size.x / 2;
    max.y = position.y + size.y / 2;
    max.z = position.z + size.z / 2;
    return max;
}

Vector3D get_World_Bounding_Box_Min(Vector3D size, Vector3D position)
{
    Vector3D min;
    min.x = position.x - size.x / 2;
    min.y = position.y - size.y / 2;
    min.z = position.z - size.z / 2;
    return min;
}

World* get_world()
{
    return current_world;
}


/*eol@eof*/
