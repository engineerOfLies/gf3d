#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_draw.h"

#include "fighter.h"


void fighter_update(Entity *self);
void fighter_draw(Entity *self);
void fighter_think(Entity *self);
void fighter_free(Entity *self);

Entity *fighter_new(Vector3D position)
{
    Entity *ent = NULL;    
    ent = gf3d_entity_new();
    if (!ent)
    {
        return NULL;
    }
    gfc_line_cpy(ent->name,"FIGHTER");
//    ent->model = gf3d_model_load("models/fighter/fighter.model");
    ent->model = gf3d_model_load("models/ships/passenger_transport.model");
    if (!ent->model)
    {
        slog("FIGHTER MODEL DID NOT LOAD");
    }
    ent->mat.scale = vector3d(10,10,10);
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    ent->think = fighter_think;
    ent->draw = fighter_draw;
    ent->update = fighter_update;
    ent->free = fighter_free;
    vector3d_copy(ent->mat.position,position);
    return ent;
}

void fighter_free(Entity *self)
{
}

void fighter_update(Entity *self)
{
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
}

void fighter_draw(Entity *self)
{
}

void fighter_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
