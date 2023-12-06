#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "world.h"
#include "player.h"
#include "weapon.h"
#include "agumon.h"
#include "resource.h"
#include "enemy.h"


static World *the_world = NULL;
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
    SJson *json,*wjson,*ejson;
    World *w = NULL;
    const char *modelName = NULL;
    const char *entName = NULL;

    Vector3D entPosition;

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

    ejson = sj_object_get_value(json,"player");
    if (!ejson)
    {
        slog("failed to find world object in %s world condig",filename);
        free(w);
        sj_free(json);
        return NULL;
    }

    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    if (!entName)
    {
        slog("entity data in (%s) has no name",filename);
        sj_free(json);
        return w;
    }

    w->entityList = gfc_list_new();



    if(gfc_stricmp(entName, "Player") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        gfc_list_append(w->entityList,player_new(entPosition));

    }

    ejson = sj_object_get_value(json,"wep1");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName:|%s|", entName);

    if(gfc_stricmp(entName, "AK") == 0)
    {
        gfc_list_append(w->entityList,weapon_new());
    }

    ejson = sj_object_get_value(json,"agumon");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName:|%s|", entName);

    if(gfc_stricmp(entName, "Agumon") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        gfc_list_append(w->entityList,agumon_new(entPosition));
    }


    ejson = sj_object_get_value(json,"log");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName:|%s|", entName);

    if(gfc_stricmp(entName, "log") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "log";
        gfc_list_append(w->entityList,resource_new(entPosition, modelName, entName));
    }


    ejson = sj_object_get_value(json,"cement");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName:|%s|", entName);

    if(gfc_stricmp(entName, "concrete") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "concrete";
        gfc_list_append(w->entityList,resource_new(entPosition, modelName, entName));
    }


    ejson = sj_object_get_value(json,"metal");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName: %s", entName);

    if(gfc_stricmp(entName, "metal") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "metal";
        gfc_list_append(w->entityList,resource_new(entPosition, modelName, entName));
    }

    ejson = sj_object_get_value(json,"fuel");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName: %s", entName);

    if(gfc_stricmp(entName, "fuel") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "fuel";
        gfc_list_append(w->entityList,resource_new(entPosition, modelName, entName));
    }

    ejson = sj_object_get_value(json,"water");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName: %s", entName);

    if(gfc_stricmp(entName, "water") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "water";
        gfc_list_append(w->entityList,resource_new(entPosition, modelName, entName));
    }


    ejson = sj_object_get_value(json,"walker");
    entName = sj_get_string_value(sj_object_get_value(ejson,"entityName"));
    slog("entName: %s", entName);

    if(gfc_stricmp(entName, "walker") == 0)
    {
        sj_value_as_vector3d(sj_object_get_value(ejson,"position"),&entPosition);
        modelName = sj_get_string_value(sj_object_get_value(ejson,"model"));
        entName = "walker";
        gfc_list_append(w->entityList,enemy_new(entPosition, modelName, entName));
    }




    sj_free(json);
    w->color = gfc_color(1,1,1,1);

    the_world = w;
    return w;
}

void world_draw(World *world)
{
    if (!world)return;
    if (!world->model)return;// no model to draw, do nothing
    gf3d_model_draw(world->model,world->modelMat,gfc_color_to_vector4f(world->color),vector4d(2,2,2,2));
    //gf3d_model_draw_highlight(world->worldModel,world->modelMat,vector4d(1,.5,.1,1));
}

void world_delete(World *world)
{
    if (!world)return;
    gf3d_model_free(world->model);
    free(world);
    the_world = NULL;
}

void world_run_updates(World *self)
{
    //self->rotation.z += 0.0001;
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_scale(self->modelMat,self->scale);
    gfc_matrix_rotate_by_vector(self->modelMat,self->modelMat,self->rotation);
    gfc_matrix_translate(self->modelMat,self->position);

}


float world_get_collision_height(Vector3D down){
//     Vector3D contact = {0};
//     Edge3D e;
    if(!the_world){
        return 0;
    }
//     e.a = from;
//     e.b = from;
//     e.b.z -= 80000;
//     gf3d_obj_load_edge_test(the_world->model->mesh->obj, e, &contact);
    return 0;
}

void world_add_entity(World *world,Entity *entity);




/*eol@eof*/
