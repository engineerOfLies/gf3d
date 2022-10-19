#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"

#include "world.h"

/*
typedef struct
{

    Model *worldModel;
    List *spawnList;        //entities to spawn
    List *entityList;       //entities that exist in the world
}World;
*/

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
    if (modelName)
    {
        w->worldModel = gf3d_model_load((char *)modelName);
        gfc_matrix_identity(w->modelMat);
        gfc_matrix_scale(
            w->modelMat,
            vector3d(10,10,10)
        );
/*        gfc_matrix_translate(
            w->modelMat,
            vector3d(0,0,-10)
        );*/
    }
    else
    {
        slog("world data (%s) has no model",filename);
    }
    w->rotation.x = 0.001;
    sj_free(json);
    w->color = gfc_color(1,1,1,1);
    return w;
}

void world_draw(World *world)
{
    if (!world)return;
    if (!world->worldModel)return;// no model to draw, do nothing
    gf3d_model_draw(world->worldModel,world->modelMat,gfc_color_to_vector4f(world->color),gfc_color_to_vector4f(world->color));
    gf3d_model_draw_highlight(world->worldModel,world->modelMat,gfc_color_to_vector4f(world->color),vector4d(1,.5,.1,1));
}

void world_delete(World *world)
{
    if (!world)return;
    gf3d_model_free(world->worldModel);
    free(world);
}

void world_run_updates(World *self)
{
    gfc_matrix_rotate_by_vector(self->modelMat,self->modelMat,self->rotation);

}

void world_add_entity(World *world,Entity *entity);


/*eol@eof*/
