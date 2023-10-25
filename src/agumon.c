
#include "simple_logger.h"

#include "world.h"
#include "agumon.h"


void agumon_update(Entity *self);

void agumon_think(Entity *self);

Entity *agumon_new(Vector3D position)
{
    Entity *ent = NULL;
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no agumon for you!");
        return NULL;
    }
    //ent->selectedColor = gfc_color(0.1,1,0.1,1);
    ent->selectedColor = gfc_color(0,0,0,1);
    ent->color = gfc_color(1,1,1,1);
    ent->model = gf3d_model_load("models/dino.model");
    ent->think = agumon_think;
    ent->update = agumon_update;
    ent->bounds.x = -8;
    ent->bounds.y = -8;
    ent->bounds.z = -8;
    ent->bounds.w = 16;
    ent->bounds.h = 16;
    ent->bounds.d = 16;
    vector3d_copy(ent->position,position);
    return ent;
}

void agumon_update(Entity *self)
{
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




    //self->rotation.z += 0.01;
}

void agumon_think(Entity *self)
{
    if (!self)return;
    switch(self->state)
    {
        case ES_idle:
            //look for player
            break;
        case ES_hunt:
            // set move towards player
            break;
        case ES_dead:
            // remove myself from the system
            break;
        case ES_attack:
            // run through attack animation / deal damage
            break;
    }
}

/*eol@eof*/
