#ifndef __WORLD_H__
#define __WORLD_H__


#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"

#include "gf3d_vgraphics.h"
#include "gf3d_particle.h"
#include "gf3d_model.h"

#include "entity.h"

typedef struct
{
    List    *model_list;
    Matrix4 skyMat;
    Particle theSun;
    Model *sky;
    Mix_Music *backgroundMusic;
}World;

World *world_load(char *filename);

void world_draw(World *world);

void world_delete(World *world);

void world_run_updates(World *world);

void world_add_entity(World *world,Entity *entity);

#endif
