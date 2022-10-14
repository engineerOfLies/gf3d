
#include "simple_logger.h"
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
    
    ent->model = gf3d_model_load("dino");
    ent->think = agumon_think;
    ent->update = agumon_update;
    vector3d_copy(ent->position,position);
    return ent;
}

void agumon_update(Entity *self)
{
    Vector3D scale = {0.01,0.01,0.01};
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    vector3d_add(self->position,self->position,self->velocity);
//    self->scale.x = 10;
    self->rotation.z += 0.01;
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
