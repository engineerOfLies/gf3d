
#include "simple_logger.h"
#include "station.h"

typedef struct
{
    Model *segment;
}StationData;

void station_update(Entity *self);
void station_draw(Entity *self);
void station_think(Entity *self);
void station_free(Entity *self);

Entity *station_new(Vector3D position)
{
    Entity *ent = NULL;
    StationData *data;
    
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
    data = gfc_allocate_array(sizeof(StationData),1);
    ent->data = data;
    data->segment = gf3d_model_load("models/station/station_core.model");
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    ent->model = gf3d_model_load("models/station/station_ring.model");
    ent->think = station_think;
    ent->draw = station_draw;
    ent->update = station_update;
    ent->free = station_free;
    vector3d_copy(ent->position,position);
    return ent;
}

void station_free(Entity *self)
{
    StationData *data;
    if (!self)return;
    data = (StationData *)self->data;
    if (data)
    {
        gf3d_model_free(data->segment);
        free(data);
    }
}

void station_update(Entity *self)
{
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    vector3d_add(self->position,self->position,self->velocity);
    self->rotation.y += 0.001;
}

void station_draw(Entity *self)
{
    StationData *data;
    if (!self)return;
    data = (StationData *)self->data;
    gf3d_model_draw(data->segment,self->modelMat,gfc_color_to_vector4f(self->color),vector4d(1,1,1,1));
}

void station_think(Entity *self)
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
