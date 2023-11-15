#include "simple_logger.h"
#include "gfc_types.h"

#include "resource.h"
#include "world.h"

void resource_think(Entity *self);
void resource_update(Entity *self);
void resource_free(Entity *self);

Entity *resource_new(Vector3D position, const char *modelToLoad, const char *name){

    Entity *log = NULL;
    //Entity *player = NULL;

    log = entity_new();
    //player = entity_get_player();

    if(!log){
        slog("Could not create resource");
        return NULL;
    }

    log->selectedColor = gfc_color(0,0,0,1);
    log->color = gfc_color(1,1,1,1);
    log->model = gf3d_model_load(modelToLoad);
    log->scale = vector3d(0.0039,0.0039,0.0039);
    //log->scale = vector3d(1,1,1);
    log->think = resource_think;
    log->update = resource_update;
    log->free = resource_free;
    log->isResource = 1;
    vector3d_copy(log->position, position);
    //log->rotation.y = GFC_HALF_PI;
    log->entityName = name;
    return log;
}

void resource_think(Entity *self){

    Entity *player;
    player = entity_get_player();

    if(!self)return;
    if(!player)return;
}

void resource_update(Entity *self){

    float height;
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }


    height = world_get_collision_height(self->position);
    if(self->position.z > height){
        self->velocity.z -= 0.0000098;
    }
    vector3d_add(self->position,self->position,self->velocity);

    if(self->position.z < height){
        self->position.z = height;
        if(self->velocity.z < 0)self->velocity.z = 0;
    }
}

void resource_free(Entity *self){
    if(!self)return;
}
