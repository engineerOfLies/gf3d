#include "simple_logger.h"
#include "gfc_types.h"

#include "enemy.h"
#include "world.h"

void enemy_think(Entity *self);
void enemy_update(Entity *self);
void enemy_free(Entity *self);

Entity *enemy_new(Vector3D position, const char *modelToLoad, const char *name){

    Entity *enemy = NULL;
    Entity *player = NULL;

    enemy = entity_new();
    player = entity_get_player();

    if(!enemy){
        slog("could not create enemy!");
        return NULL;
    }

    enemy->selectedColor = gfc_color(0,0,0,1);
    enemy->color = gfc_color(1,1,1,1);
    enemy->model = gf3d_model_load(modelToLoad);
    enemy->scale = vector3d(0.0625,0.0625,0.0625);
    //enemy->scale = vector3d(1,1,1);
    enemy->think = enemy_think;
    enemy->update = enemy_update;
    enemy->free = enemy_free;
    enemy->isEnemy = 1;
    vector3d_copy(enemy->position, position);
    //enemy->rotation.y = GFC_HALF_PI;
    enemy->entityName = name;
    enemy->target = player;
    return enemy;
}


void enemy_think(Entity *self){

    Entity *player;
    player = entity_get_player();
    Vector3D forward = {0};
    Vector3D right = {0};
    Vector2D w;

    if(!self)return;
    if(!player)return;

    w = vector2d_from_angle(self->rotation.z);
    forward.x = ((w.x)*0.0625);
    forward.y = ((w.y)*0.0625);
    right.x = ((w.x)*0.0625);
    right.y = ((w.y)*0.0625);
}

void enemy_update(Entity *self){

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

void enemy_free(Entity *self){
    if(!self)return;
}
