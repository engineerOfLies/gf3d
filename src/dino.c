#include "simple_logger.h"

#include "gfc_input.h"
#include "gf3d_camera.h"

#include "dino.h"

typedef struct
{
    Uint8   freeLook;
    float   cameraPitch;
}DinoData;

void dino_think(Entity *self);
void dino_update(Entity *self);
void dino_free(Entity *self);


Entity *dino_spawn(GFC_Vector3D position)
{
    Entity *self;
    DinoData *data;
    self = entity_new();
    if (!self)return NULL;
    self->model = gf3d_model_load("models/dino.model");
    self->free = dino_free;
    self->think = dino_think;
    self->update = dino_update;
    self->position = position;
    data = gfc_allocate_array(sizeof(DinoData),1);
    if (data)
    {
        self->data = data;
    }
        
    
    return self;
}

void dino_free(Entity *self)
{
    DinoData *data;
    if (!self)return;
    if (!self->data)return;
    data = (DinoData*)self->data;
    //clean up the needful
    free(data);
    self->data = NULL;
}


void dino_think(Entity *self)
{
    int dx,dy;
    GFC_Vector2D    dir = {0,-1};
    DinoData *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    dir = gfc_vector2d_rotate(dir, self->rotation.z);
    if (gfc_input_command_pressed("locate"))
    {
        slog("position: %f,%f,%f",self->position.x,self->position.y,self->position.z);
    }
    if (gfc_input_command_pressed("freelook"))
    {
        data->freeLook = !data->freeLook;
        gf3d_camera_enable_free_look(data->freeLook);
    }
    SDL_GetRelativeMouseState(&dx,&dy);
    self->rotation.z += dx *0.01;
    data->cameraPitch += dy *0.01;
    if (gfc_input_command_down("walkforward"))
    {
        gfc_vector2d_add(self->position,self->position,dir);
    }
    if (gfc_input_command_down("walkback"))
    {
        gfc_vector2d_add(self->position,self->position,-dir);
    }
}

void dino_update(Entity *self)
{
    GFC_Vector3D lookTarget,camera,dir = {0};
    DinoData *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    if (data->freeLook)return;
    gfc_vector3d_copy(lookTarget,self->position);
    lookTarget.z += 5;
    dir.y = -5.0;
    gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
    gfc_vector3d_sub(camera,self->position,dir);
    camera.z += 10;
    gf3d_camera_look_at(lookTarget,&camera);
}


/*eol@eof*/
