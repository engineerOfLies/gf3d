#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_draw.h"

#include "gate.h"

typedef struct
{
    Model *tunnel;
    float frame;
    float r;
}GateData;


void gate_update(Entity *self);
void gate_draw(Entity *self);
void gate_think(Entity *self);
void gate_free(Entity *self);

Entity *gate_new(Vector3D position)
{
    Entity *ent = NULL;    
    GateData *data = NULL;
    ent = gf3d_entity_new();
    if (!ent)
    {
        return NULL;
    }
    data = gfc_allocate_array(sizeof(GateData),1);
    data->tunnel = gf3d_model_load("models/tunnel_gate/tunnel.model");
    ent->data = data;
    ent->model = gf3d_model_load("models/tunnel_gate/gate.model");
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    ent->think = gate_think;
    ent->draw = gate_draw;
    ent->update = gate_update;
    ent->free = gate_free;
    ent->mat.scale = vector3d(100,100,100);
    vector3d_copy(ent->mat.position,position);
    //ent->mat.rotation.z = GFC_PI;
    return ent;
}

void gate_free(Entity *self)
{
}

void gate_update(Entity *self)
{
    GateData *data;
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    data = self->data;
    data->r += 0.001;
    data->frame += 0.0001;
    if (data->frame >= 1)data->frame = 0;
}

void gate_draw(Entity *self)
{
    Matrix4 mat;
    GateData *data;
    data = self->data;
    gfc_matrix4_from_vectors(
        mat,
        self->mat.position,
        vector3d(self->mat.rotation.x + data->r,self->mat.rotation.y,self->mat.rotation.z),
        vector3d(self->mat.scale.x,self->mat.scale.y * data->frame,self->mat.scale.z));
    
    gf3d_model_draw(data->tunnel,0,mat,vector4d(255,255,255,128),vector4d(0,0,0,0),vector4d(0,0,0,1));
}

void gate_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
