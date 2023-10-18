
#include "simple_logger.h"
#include "agumon.h"


void agumon_update(Entity *self);

void agumon_think(Entity *self);

void agumon_damage(Entity *self, float damage, Entity *inflictor)
{
    if (!self)return;
    self->health -= damage;
    if (self->health <= 0)entity_free(self);
}

Entity *agumon_new(Vector3D position)
{
    Entity *ent = NULL;
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no agumon for you!");
        return NULL;
    }
    ent->selectedColor = gfc_color(0.1,1,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    ent->model = gf3d_model_load("models/dino.model");
    ent->think = agumon_think;
    ent->onDamage = agumon_damage;
    ent->update = agumon_update;
    ent->bounds.x = -10;
    ent->bounds.y = -10;
    ent->bounds.z = -10;
    ent->bounds.w = 20;
    ent->bounds.h = 20;
    ent->bounds.d = 20;
    vector3d_copy(ent->position,position);
    return ent;
}

void agumon_update(Entity *self)
{
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    vector3d_add(self->position,self->position,self->velocity);
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
