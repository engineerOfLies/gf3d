#ifndef __WORLD_H__
#define __WORLD_H__


#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_model.h"

#include "entity.h"

typedef struct
{
    Matrix4 modelMat;
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;
    Model *model;
    Color color;
    List *spawnList;        //entities to spawn
    List *entityList;       //entities that exist in the world
}World;

World *world_load(char *filename);

void world_draw(World *world);

void world_delete(World *world);

void world_run_updates(World *world);

void world_add_entity(World *world,Entity *entity);

#endif
