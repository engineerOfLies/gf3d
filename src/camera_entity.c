#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_input.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "camera_entity.h"

typedef struct
{
    int                 autoPan;
    int                 freeLook;
    Entity             *lookTargetEntity;
    Vector3D            lookTargetPosition;
    CameraTargetType    targetType;
}CameraEntityData;

static Entity *camera_entity = NULL;

void camera_entity_think(Entity *self);

Entity *camera_entity_new(Vector3D position,Vector3D rotation)
{
    Entity *ent = NULL;
    CameraEntityData *data;

    gf3d_camera_set_position(position);
    gf3d_camera_set_rotation(rotation);
    
    if (camera_entity)
    {
        return camera_entity;
    }
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no camera_entity for you!");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(CameraEntityData),1);
    ent->data = data;
    
    ent->think = camera_entity_think;
    ent->hidden = 0;
    camera_entity = ent;
    return ent;
}

void camera_entity_set_position(Vector3D position)
{
    gf3d_camera_set_position(position);
}

Vector3D camera_entity_get_position()
{
    return gf3d_camera_get_position();
}

void camera_entity_set_look_target(Vector3D target)
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    vector3d_copy(data->lookTargetPosition,target);
}

Vector3D camera_entity_get_look_target()
{
    CameraEntityData *data;
    Vector3D target = {0};
    if (!camera_entity)return target;
    data = camera_entity->data;
    return data->lookTargetPosition;
}

Entity *camera_entity_get_look_target_entity()
{
    CameraEntityData *data;
    if (!camera_entity)return NULL;
    data = camera_entity->data;
    return data->lookTargetEntity;
}

void camera_entity_set_look_target_entity(Entity *target)
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    data->lookTargetEntity = target;
}

Bool camera_entity_free_look_enabled()
{
    CameraEntityData *data;
    if (!camera_entity)return 0;
    data = camera_entity->data;
    return data->freeLook;
}

void camera_entity_toggle_free_look()
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    camera_entity_enable_free_look(!data->freeLook);
}

void camera_entity_enable_free_look(Uint8 enable)
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    data->freeLook = enable;
}


void camera_entity_think(Entity *self)
{
    float moveSpeed = 1;
    Vector3D position,rotation;
    const Uint8 * keys;
    CameraEntityData *data;
    if (!self)return;
    data = self->data;

    if (data->freeLook)
    {
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        if (keys[SDL_SCANCODE_W])
        {
            gf3d_camera_walk_forward(moveSpeed);
        }
        if (keys[SDL_SCANCODE_S])
        {
            gf3d_camera_walk_forward(-moveSpeed);
        }
        if (keys[SDL_SCANCODE_D])
        {
            gf3d_camera_walk_right(moveSpeed);
        }
        if (keys[SDL_SCANCODE_A])    
        {
            gf3d_camera_walk_right(-moveSpeed);
        }
        if (keys[SDL_SCANCODE_SPACE])gf3d_camera_move_up(moveSpeed);
        if (keys[SDL_SCANCODE_Z])gf3d_camera_move_up(-moveSpeed);
        
        if (keys[SDL_SCANCODE_UP])gf3d_camera_pitch(-0.005);
        if (keys[SDL_SCANCODE_DOWN])gf3d_camera_pitch(0.005);
        if (keys[SDL_SCANCODE_RIGHT])gf3d_camera_yaw(-0.005);
        if (keys[SDL_SCANCODE_LEFT])gf3d_camera_yaw(0.005);
        
        if (keys[SDL_SCANCODE_R])
        {
            rotation = gf3d_camera_get_angles();
            slog("rotation: %f,%f,%f",rotation.x,rotation.y,rotation.z);
        }
        if (keys[SDL_SCANCODE_P])
        {
            position = gf3d_camera_get_position();
            slog("position: %f,%f,%f",position.x,position.y,position.z);
        }
        return;
    }
    if (data->autoPan)
    {
        gf3d_camera_walk_right(moveSpeed/5);
    }
    if (data->targetType == CTT_Position)
    {
        gf3d_camera_look_at(data->lookTargetPosition,NULL);
    }
    else if (data->targetType == CTT_Entity)
    {
        if (data->lookTargetEntity)
        {
            gf3d_camera_look_at(data->lookTargetEntity->mat.position,NULL);
        }
        else
        {
            gf3d_camera_look_at(vector3d(0,0,0),NULL);
        }
    }
}

void camera_entity_set_look_mode(CameraTargetType mode)
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    data->targetType = mode;
}

void camera_entity_set_auto_pan(Bool enable)
{
    CameraEntityData *data;
    if (!camera_entity)return;
    data = camera_entity->data;
    data->autoPan = enable;
}


/*eol@eof*/
